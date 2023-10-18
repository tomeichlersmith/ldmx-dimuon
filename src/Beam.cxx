#include "Beam.h"

Beam::Beam(double energy, bool photons)
  : G4VUserPrimaryGeneratorAction() {
    if (photons) gun_.SetParticleDefinition(G4Gamma::Gamma());
    else gun_.SetParticleDefinition(G4Electron::Electron());
    gun_.SetParticleEnergy(energy*CLHEP::GeV);
    gun_.SetParticlePosition(G4ThreeVector());
    gun_.SetParticleMomentumDirection(G4ThreeVector(0.,0.,1.));
  }

void Beam::GeneratePrimaries(G4Event* event) {
  gun_.GeneratePrimaryVertex(event);
}
