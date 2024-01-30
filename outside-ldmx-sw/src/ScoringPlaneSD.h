#pragma once

#include "G4VSensitiveDetector.hh"

#include "PersistParticles.h"

/**
 * Class defining a basic sensitive detector for scoring planes.
 */
class ScoringPlaneSD : public G4VSensitiveDetector {
 public:
  /**
   * Constructor
   *
   * @param name The name of the sensitive detector.
   */
  ScoringPlaneSD(const std::string& name, PersistParticles& persist);

  /** Destructor */
  virtual ~ScoringPlaneSD() = default;

  /**
   * This is Geant4's handle to tell us that a particle has stepped
   * through our sensitive detector and we should process its interaction with
   * us.
   *
   * @param[in] step the step that happened within one of our logical volumes
   * @param[in] hist the touchable history of the step
   */
  virtual G4bool ProcessHits(G4Step* step,
                             G4TouchableHistory* hist) final;

  /**
   * Geant4's handle to tell us that the event is ending.
   *
   * Since we are handling the Hit Collections (HC) directly,
   * the input to this function is of no use to us. We simply
   * use this function to make sure 
  virtual void EndOfEvent(G4HCofThisEvent*) final;   
   */

 private:
  /// handle to the central persistence
  PersistParticles& persist_;
};  // ScoringPlaneSD

