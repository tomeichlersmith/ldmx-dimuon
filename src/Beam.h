#pragma once

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4Electron.hh"
#include "G4Gamma.hh"

/**
 * the primary generator, a simple particle gun restricted to electrons or photons
 * along the z axis
 */
class Beam : public G4VUserPrimaryGeneratorAction {
  /// the gun we use for the beam
  G4ParticleGun gun_;
 public:
  /**
   * Configure the beam to be of the input energy and particle
   *
   * Shoot along the z axis, the energy is in GeV and we
   * shoot from the origin.
   */
  Beam(double energy, bool photons);

  /**
   * Start an event by providing primaries
   */
  void GeneratePrimaries(G4Event* event) final override;
};
