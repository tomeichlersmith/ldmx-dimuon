#pragma once

#include "G4VUserDetectorConstruction.hh"

/**
 * basic 'hunk' of material in air, the material and its thickness is configurable
 *
 * The transverse (x,y) dimensions are set arbitrarily to 1m just to make
 * absolutely sure that we can contain the shower that may contain a dark brem.
 */
class Hunk : public G4VUserDetectorConstruction {
  /// depth along beam direction
  double depth_;
  /// name of material to use for volume (findable by G4NistManager)
  std::string material_;
 public:
  /**
   * Create our detector constructor, storing the configuration variables
   */
  Hunk(double d, const std::string& m);

  /**
   * Construct the geometry
   *
   * We build the world only slighly larger than the single hunk
   * of material at its center. The hunk is shifted to be
   * downstream (along z) of the origin so that the primary generator
   * can simply shoot from the origin along z.
   */
  virtual G4VPhysicalVolume* Construct() final override;
};
