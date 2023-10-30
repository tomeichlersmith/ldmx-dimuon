#include "PersistParticles.h"

#include "G4Gamma.hh"
#include "G4VProcess.hh"

PersistParticles::PersistParticles(const std::string& out_file)
  : out_{out_file.c_str(), "RECREATE"} {
    out_.cd();
    events_ = new TTree("dimuon_events","dimuon_events");
    events_->Branch("incident", &incident_);
    events_->Branch("parent", &parent_);
    events_->Branch("mu_plus", &mu_plus_);
    events_->Branch("mu_minus", &mu_minus_);
    events_->Branch("extra", &extra_);
    events_->Branch("ntries", ntries_);
}

PersistParticles::~PersistParticles() {
  std::cout
    << "[ dimuon-simulate ]: Generated " << events_completed_
    << " dimuon events out of " << events_started_ << " requested."
    << std::endl;
  events_->Write();
  out_.Close();
}

void PersistParticles::BeginOfEventAction(const G4Event*) {
  extra_.clear();
  incident_.clear();
  parent_.clear();
  mu_plus_.clear();
  mu_minus_.clear();
  ++events_started_;
  ++ntries_;
}

void PersistParticles::PreUserTrackingAction(const G4Track*) {}

void PersistParticles::UserSteppingAction(const G4Step* step) {
  // don't bother with non-photons
  if (step->GetTrack()->GetParticleDefinition() != G4Gamma::Gamma()) return;
  // check secondaries to see if the process was muon-conversion
  auto secondaries{step->GetSecondaryInCurrentStep()};
  // leave if no secondaries exist
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

void PersistParticles::ScoringPlaneHit(const G4String& name, const G4Step* step) {
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

void PersistParticles::EndOfEventAction(const G4Event*) {
  if (incident_.is_valid() and parent_.is_valid() and mu_minus_.is_valid() and mu_plus_.is_valid()) {
    ++events_completed_;
    events_->Fill();
    ntries_ = 0;
  }
}
