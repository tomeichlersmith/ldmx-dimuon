
#include "ScoringPlaneSD.h"

/*----------------*/
/*   C++ StdLib   */
/*----------------*/
#include <iostream>

/*~~~~~~~~~~~~*/
/*   Geant4   */
/*~~~~~~~~~~~~*/
#include "G4ChargedGeantino.hh"
#include "G4Geantino.hh"
#include "G4SDManager.hh"
#include "G4Step.hh"
#include "G4StepPoint.hh"

ScoringPlaneSD::ScoringPlaneSD(const std::string& name, PersistParticles& persister)
  : G4VSensitiveDetector(name), persist_{persister} {
  G4SDManager::GetSDMpointer()->AddNewDetector(this);
}

G4bool ScoringPlaneSD::ProcessHits(G4Step* step, G4TouchableHistory* history) {
  persist_.ScoringPlaneHit(this->GetName(), step);
/*
  // Get the edep from the step.
  G4double edep = step->GetTotalEnergyDeposit();

  // Create a new hit object.
  ldmx::SimTrackerHit& hit{hits_.emplace_back()};

  // Assign track ID for finding the SimParticle in post event processing.
  hit.setTrackID(step->GetTrack()->GetTrackID());
  hit.setPdgID(step->GetTrack()->GetDynamicParticle()->GetPDGcode());

  // Set the edep.
  hit.setEdep(edep);

  // Set the start position.
  G4StepPoint* prePoint = step->GetPreStepPoint();

  // Set the end position.
  G4StepPoint* postPoint = step->GetPostStepPoint();

  G4ThreeVector start = prePoint->GetPosition();
  G4ThreeVector end = postPoint->GetPosition();

  // Set the mid position.
  G4ThreeVector mid = 0.5 * (start + end);
  hit.setPosition(mid.x(), mid.y(), mid.z());

  // Compute path length.
  G4double pathLength =
      sqrt(pow(start.x() - end.x(), 2) + pow(start.y() - end.y(), 2) +
           pow(start.z() - end.z(), 2));
  hit.setPathLength(pathLength);

  // Set the global time.
  hit.setTime(step->GetTrack()->GetGlobalTime());

  // Set the momentum
  G4ThreeVector p = postPoint->GetMomentum();
  hit.setMomentum(p.x(), p.y(), p.z());
  hit.setEnergy(postPoint->GetTotalEnergy());
*/
  return true;
}
