#pragma once

#include <array>

#include "TObject.h"

#include "G4Track.hh"

/**
 * The object that we will use to store particle information
 */
class Particle {
  bool valid;
  int track_id;
  int pdg_id;
  std::array<double, 4> momentum;
  ClassDef(Particle, 1);
 public:
  Particle() = default;
  ~Particle() = default;
  /**
   * reset the particle to blank state
   */
  void clear();
  /**
   * Check if the particle is valid
   */
  bool is_valid() const {
    return valid;
  }

  /**
   * "Assign" a G4Track to this Particle.
   *
   * This is where we define how a G4Track's variables
   * are copied into a Particle's variables
   */
  void operator=(const G4Track* track);
};