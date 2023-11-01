#include "Particle.h"

ClassImp(Particle);

Particle::Particle(const G4Track* track)
  : Particle() {
  *this = track;
}

void Particle::clear() {
  valid = false;
}

void Particle::operator=(const G4Track* track) {
  this->valid = true;
  this->track_id = track->GetTrackID();
  this->pdg_id = track->GetParticleDefinition()->GetPDGEncoding();
  const G4ThreeVector& direction{track->GetMomentumDirection()};
  G4double kinetic_energy = track->GetKineticEnergy();
  G4double mass = track->GetParticleDefinition()->GetPDGMass();
  G4double momentum_mag = sqrt(kinetic_energy*(kinetic_energy+2*mass));
  this->px = direction.x() * momentum_mag;
  this->py = direction.y() * momentum_mag;
  this->pz = direction.z() * momentum_mag;
  this->energy = kinetic_energy + mass;
  const G4ThreeVector& position{track->GetPosition()};
  this->x = position.x();
  this->y = position.y();
  this->z = position.z();
  this->t = track->GetGlobalTime();
}
