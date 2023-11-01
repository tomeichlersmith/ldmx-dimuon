#pragma once

#include <array>

#include "TObject.h"

#include "G4Step.hh"

/**
 * The object that we will use to store particle information
 */
class ScoringPlaneHit {
  int track_id;
  double x, y, z, t;
  ClassDef(ScoringPlaneHit, 1);
 public:
  ScoringPlaneHit() = default;
  virtual ~ScoringPlaneHit() = default;
  /**
   * "Assign" a G4Step to this ScoringPlaneHit.
   *
   * This is where we define how a G4Step's variables
   * are copied into a ScoringPlaneHit's variables
   */
  ScoringPlaneHit(const G4Step* step);
};
