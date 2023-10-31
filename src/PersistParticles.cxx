#include "PersistParticles.h"

#include "G4RunManager.hh"
#include "G4Gamma.hh"
#include "G4VProcess.hh"

PersistParticles::PersistParticles(const std::string& out_file, std::optional<double> filter_threshold)
  : out_{out_file.c_str(), "RECREATE"}, filter_threshold_{filter_threshold} {
    out_.cd();
    events_ = new TTree("dimuon_events","dimuon_events");
    events_->Branch("incident", &incident_);
    events_->Branch("parent", &parent_);
    events_->Branch("mu_plus", &mu_plus_);
    events_->Branch("mu_minus", &mu_minus_);
    events_->Branch("extra", &extra_);
    events_->Branch("ntries", ntries_);
    events_->Branch("ecal", &ecal_);
}

PersistParticles::~PersistParticles() {
  std::cout
    << "[ dimuon-simulate ]: Generated " << events_completed_
    << " dimuon events out of " << events_started_ << " requested."
    << std::endl;
  events_->Write();
  out_.Close();
}

bool PersistParticles::success() {
  if (filter_threshold_.has_value()) return true;
  return (
      incident_.is_valid() and
      parent_.is_valid() and
      mu_plus_.is_valid() and
      mu_minus_.is_valid() and
      (
       mu_plus_.energy() > filter_threshold_.value() or
       mu_minus_.energy() > filter_threshold_.value()
      )
  );
}

void PersistParticles::BeginOfEventAction(const G4Event*) {
  no_more_particles_above_threshold_ = false;
  extra_.clear();
  incident_.clear();
  parent_.clear();
  mu_plus_.clear();
  mu_minus_.clear();
  ecal_.clear();
  ++events_started_;
  ++ntries_;
}

G4ClassificationOfNewTrack PersistParticles::ClassifyNewTrack(const G4Track* track, const G4ClassificationOfNewTrack& current_classification) {
  /**
   * track has kinetic energy above our filtering threshold
   * and so we tell Geant4 to process it as soon as possible
   */
  if (track->GetKineticEnergy() > filter_threshold_.value_or(0.)) {
    return fUrgent;
  }
  /**
   * track is below the threshold and so we push it onto
   * the waiting stack unless there are no more particles
   * above the threshold
   */
  if (no_more_particles_above_threshold_) return current_classification;
  return fWaiting;
}

void PersistParticles::PreUserTrackingAction(const G4Track*) {}

void PersistParticles::UserSteppingAction(const G4Step* step) {
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
        std::cerr << "More than one mu+mu- parent!" << std::endl;
      }
      parent_ = step->GetTrack();
      return; // no need to check other secondaries
    }
  }
}

void PersistParticles::NewScoringPlaneHit(const G4String& name, const G4Step* step) {
  ecal_.emplace_back(step);
}

void PersistParticles::PostUserTrackingAction(const G4Track* track) {
  if (track->GetCreatorProcess() == nullptr) {
    // only the primary has no creator process
    incident_ = track;
    return;
  } 

  if (track->GetParticleDefinition() == G4MuonMinus::MuonMinus()) {
    if (mu_minus_.is_valid()) {
      std::cerr << "More than one mu-!" << std::endl;
    }
    mu_minus_ = track;
    return;
  }

  if (track->GetParticleDefinition() == G4MuonPlus::MuonPlus()) {
    if (mu_plus_.is_valid()) {
      std::cerr << "More than one mu+!" << std::endl;
    }
    mu_plus_ = track;
    return;
  }

  if (track->GetVolume()->GetName() == "World_PV") {
    // track is ending outside of the box of material
    extra_.emplace_back();
    extra_.back() = track;
    return;
  }
}

void PersistParticles::NewStage() {
  no_more_particles_above_threshold_ = true;
  if (not success()) {
    G4RunManager::GetRunManager()->AbortEvent();
  }
}

void PersistParticles::EndOfEventAction(const G4Event*) {
  if (success()) {
    ++events_completed_;
    events_->Fill();
    ntries_ = 0;
  }
}
