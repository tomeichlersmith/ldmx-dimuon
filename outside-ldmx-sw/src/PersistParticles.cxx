#include "PersistParticles.h"

#include "RunHeader.h"

#include "G4EventManager.hh"
#include "G4RunManager.hh"
#include "G4Gamma.hh"
#include "G4VProcess.hh"

static void AbortEvent(const std::string& reason) {
#if(DEBUG==1)
  std::cout 
    << "[ event "
    << G4EventManager::GetEventManager()->GetConstCurrentEvent()->GetEventID()
    << " ] Aborting due to " << reason << std::endl;
#endif
  G4RunManager::GetRunManager()->AbortEvent();

}

PersistParticles::PersistParticles(const std::string& out_file, std::optional<double> filter_threshold, std::optional<double> bias_factor, const std::string& target, double depth, double beam, bool photons, long seed)
  : out_{out_file.c_str(), "RECREATE"}, filter_threshold_{filter_threshold}, bias_factor_{bias_factor}, target_{target}, depth_{depth}, beam_{beam}, photons_{photons}, seed_{seed} {
    out_.cd();
    events_ = new TTree("events","dimuon_events");
    events_->Branch("incident", &incident_);
    events_->Branch("parent", &parent_);
    events_->Branch("mu_plus", &mu_plus_);
    events_->Branch("mu_minus", &mu_minus_);
    events_->Branch("extra", &extra_);
    events_->Branch("ecal", &ecal_);
    events_->Branch("weight", &weight_, "weight/D");
}

PersistParticles::~PersistParticles() {
  std::cout
    << "[ dimuon-simulate ]: Generated " << events_completed_
    << " events out of " << events_started_ << " requested."
    << std::endl;
  RunHeader rh(
      events_started_,
      filter_threshold_,
      bias_factor_,
      target_,
      depth_,
      beam_,
      photons_,
      seed_
  );
  out_.WriteObject(&rh, "run");
  events_->Write();
  out_.Close();
}

bool PersistParticles::success() {
  if (not filter_threshold_) return true;
  return (
      incident_.is_valid() and
      parent_.is_valid() and
      mu_plus_.is_valid() and
      mu_minus_.is_valid() and
      (
       mu_plus_.total_energy() > filter_threshold_.value() or
       mu_minus_.total_energy() > filter_threshold_.value()
      )
  );
}

void PersistParticles::BeginOfEventAction(const G4Event*) {
  no_more_particles_above_threshold_ = false;
  weight_ = 1.;
  extra_.clear();
  incident_.clear();
  parent_.clear();
  mu_plus_.clear();
  mu_minus_.clear();
  ecal_.clear();
  ++events_started_;
}

G4ClassificationOfNewTrack PersistParticles::ClassifyNewTrack(const G4Track* track) {
  /**
   * If the track has kinetic energy above our filtering
   * threshold or if there are no more particles above the threshold,
   * we tell Geant4 to process it as soon as possible.
   */
  if (track->GetDefinition()==G4MuonMinus::MuonMinus() or track->GetDefinition()==G4MuonPlus::MuonPlus() or track->GetKineticEnergy() > filter_threshold_.value_or(0.) or no_more_particles_above_threshold_) {
    return fUrgent;
  }
  /**
   * track is below the threshold and so we push it onto
   * the waiting stack
   */
  return fWaiting;
}

void PersistParticles::PreUserTrackingAction(const G4Track* track) {
  if (track->GetCreatorProcess() == nullptr and not incident_.is_valid()) {
    incident_ = track;
    return;
  }

  /**
   * If the track is a muon, we inform our storage mechanism that we have found them.
   */
  if (track->GetDefinition() == G4MuonMinus::MuonMinus()) {
    if (mu_minus_.is_valid()) {
      AbortEvent("more than one muon conversion (second mu- found)");
      return;
    }
    mu_minus_ = track;
  } else if (track->GetDefinition() == G4MuonPlus::MuonPlus()) {
    if (mu_plus_.is_valid()) {
      AbortEvent("more than one muon conversion (second mu+ found)");
      return;
    }
    mu_plus_ = track;
  }
}

static G4String GetVolumeName(const G4StepPoint* point) {
  const G4VPhysicalVolume* pv{point->GetPhysicalVolume()};
  return (pv == nullptr ? "nullptr" : pv->GetName());
}

static bool IsTransiting(const G4String& from_volume, const G4String& to_volume, const G4Step* step) {
  G4String pre{GetVolumeName(step->GetPreStepPoint())},
           post{GetVolumeName(step->GetPostStepPoint())};
  return (pre == from_volume and post == to_volume);
}

void PersistParticles::UserSteppingAction(const G4Step* step) {
  // get the track weights before this step and after this step
  //  ** these weights include the factors of all upstream step weights **
  double track_weight_pre_step = step->GetPreStepPoint()->GetWeight();
  double track_weight_post_step = step->GetPostStepPoint()->GetWeight();
  //  so, to get _this_ step's weight, we divide post_weight by pre_weight
  double weight_of_this_step_alone = track_weight_post_step / track_weight_pre_step;
  // increment the event weight multiplicatively
  weight_ *= weight_of_this_step_alone;
  /**
   * look for "extra" particles leaving Hunk and entering World
   *
   * The goal of a calibration target is to absorb _all_ of the 
   * normal particles _except_ for the muons we want to use for
   * calibration, so we put any particle that is not either of
   * the muons and is leaving the Hunk into the list of "extra"
   * particles.
   */
  int id{step->GetTrack()->GetTrackID()};
  if (IsTransiting("Hunk", "World", step) and
      id != mu_minus_.id() and
      id != mu_plus_.id()) {
    extra_.emplace_back(step->GetTrack());
  }
  /**
   * if we are below the filtering threshold (when filtering) then 
   * we should just process like normal
   */
  if (no_more_particles_above_threshold_) return;
  /**
   * suspend tracks that step from above the threshold to
   * below it in order to get to the decision as fast as possible
   */
  auto pre_energy{step->GetPreStepPoint()->GetKineticEnergy()};
  auto post_energy{step->GetPostStepPoint()->GetKineticEnergy()};
  if (pre_energy >= filter_threshold_.value_or(0.) and 
      post_energy < filter_threshold_.value_or(0.)) {
    step->GetTrack()->SetTrackStatus(fSuspend);
  }
  /**
   * Further checks are only searching for the muon-conversion,
   * so we don't bother with any non-photons.
   */
  if (step->GetTrack()->GetParticleDefinition() != G4Gamma::Gamma()) return;
  auto secondaries{step->GetSecondaryInCurrentStep()};
  if (secondaries == nullptr or secondaries->size() == 0) return;
  for (const G4Track* secondary : *secondaries) {
    const G4VProcess* creator{secondary->GetCreatorProcess()};
    if (creator == nullptr) continue;
    const G4String& creator_name{creator->GetProcessName()};
    if (creator_name.contains("GammaToMuPair")) {
      // this step was a muon-conversion, try to assign the
      // current track to be the parent.
      if (parent_.is_valid()) {
        AbortEvent("more than one muon conversion (second parent found)");
        return;
      }
      parent_ = step->GetTrack();
      return; // no need to check other secondaries
    }
  }
}

void PersistParticles::NewScoringPlaneHit(const G4String&, const G4Step* step) {
  ecal_.emplace_back(step->GetTrack());
}

void PersistParticles::PostUserTrackingAction(const G4Track* /*track*/) {
}

void PersistParticles::NewStage() {
  no_more_particles_above_threshold_ = true;
  if (not success()) {
    AbortEvent("unsuccessful generation (no muon-conv found or both muons below threshold)");
    return;
  }
}

void PersistParticles::EndOfEventAction(const G4Event*) {
  if (success()) {
    ++events_completed_;
    events_->Fill();
  }
}
