
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

G4bool ScoringPlaneSD::ProcessHits(G4Step* step, G4TouchableHistory*) {
  persist_.NewScoringPlaneHit(this->GetName(), step);
  return true;
}
