#pragma once

#include "TObject.h"

#include "G4Track.hh"

/**
 * The object that we will use to store particle information
 */
class Particle {
  bool valid;
  int track_id;
  int pdg_id;
  int parent_id;
  double px, py, pz, energy;
  double x, y, z, t;
  ClassDef(Particle, 1);
 public:
  Particle() = default;
  /**
   * "Copy" constructor where we create a default particle
   * and then assign the current values in the passed G4Track
   * to our members
   */
  Particle(const G4Track* track);
  virtual ~Particle() = default;
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
   * Get the total energy of the particle
   */
  double total_energy() const {
    return energy;
  }
  /**
   * Get the track ID of the particle
   */
  int id() const {
    return track_id;
  }

  /**
   * "Assign" a G4Track to this Particle.
   *
   * This is where we define how a G4Track's variables
   * are copied into a Particle's variables
   */
  void operator=(const G4Track* track);
};
