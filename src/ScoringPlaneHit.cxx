#include "ScoringPlaneHit.h"

ClassImp(ScoringPlaneHit);

ScoringPlaneHit::ScoringPlaneHit(const G4Step* step)
  : ScoringPlaneHit() {
  this->track_id = step->GetTrack()->GetTrackID();

  G4ThreeVector start = step->GetPreStepPoint()->GetPosition();
  G4ThreeVector end   = step->GetPostStepPoint()->GetPosition();
  G4ThreeVector mid = 0.5 * (start + end);
  this->t = step->GetTrack()->GetGlobalTime();
  this->x = mid.x();
  this->y = mid.y();
  this->z = mid.z();
}
