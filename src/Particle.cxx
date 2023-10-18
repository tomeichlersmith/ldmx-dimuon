#include "Particle.h"

ClassImp(Particle);

void Particle::clear() {
  valid = false;
}

void Particle::operator=(const G4Track* track) {
  this->valid = true;
  this->track_id = track->GetTrackID();
  this->pdg_id = track->GetParticleDefinition()->GetPDGEncoding();
  G4ThreeVector direction = track->GetVertexMomentumDirection();
  G4double kinetic_energy = track->GetVertexKineticEnergy();
  G4double mass = track->GetParticleDefinition()->GetPDGMass();
  G4double momentum_mag = sqrt(kinetic_energy*(kinetic_energy+2*mass));
  this->momentum = {
    kinetic_energy + mass,
    direction.x() * momentum_mag,
    direction.y() * momentum_mag,
    direction.z() * momentum_mag
  };
}
