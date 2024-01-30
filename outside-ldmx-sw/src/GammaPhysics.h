#pragma once

#include "G4VPhysicsConstructor.hh"
#include "G4GammaConversionToMuons.hh"

/**
 * basic physics constructor which simply constructs the GammaConversionToMuons
 * process and adds it to the Gamma process table.
 */
class GammaPhysics : public G4VPhysicsConstructor {
  /// handle to the process, cleaned up when the physics list is desctructed
  std::unique_ptr<G4GammaConversionToMuons> the_process_;
 public:
  /// create the physics
  GammaPhysics() = default;

  /**
   * We don't construct any particles since we are just
   * attaching a new process to an existing particle
   */
  void ConstructParticle() final override;

  /**
   * Construct and configure the muon-conversion process
   *
   * We own the process and clean it up when the physics constructor
   * is cleaned up by Geant4 after registration.
   */
  void ConstructProcess() final override;
};
