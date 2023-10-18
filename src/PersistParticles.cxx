#include "PersistParticles.h"

PersistParticles::PersistParticles(const std::string& out_file)
  : out_{out_file.c_str(), "RECREATE"} {
    out_.cd();
    events_ = new TTree("dimuon_events","dimuon_events");
    events_->Branch("incident", &incident_);
    //events_->Branch("parent", &parent_);
    events_->Branch("mu_plus", &mu_plus_);
    events_->Branch("mu_minus", &mu_minus_);
    events_->Branch("extra", &extra_);
}

PersistParticles::~PersistParticles() {
  std::cout << "[dimuon-simulate] Able to generate a muon conversion " 
    << events_completed_ << " / " << events_started_ 
    << " events" << std::endl;
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
}

void PersistParticles::PreUserTrackingAction(const G4Track*) {}

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

  // TODO deduce parent gamma of mu+mu-

  if (track->GetVolume()->GetName() == "World_PV") {
    // track is ending outside of the box of material
    extra_.emplace_back();
    extra_.back() = track;
    return;
  }
}

void PersistParticles::EndOfEventAction(const G4Event*) {
  // ignoring parent_ right now
  if (incident_.is_valid() and mu_minus_.is_valid() and mu_plus_.is_valid()) {
    ++events_completed_;
    events_->Fill();
  }
}
