#pragma once

#include "G4VUserDetectorConstruction.hh"

#include "ScoringPlaneSD.h"
#include "MuonConversionBiasing.h"

/**
 * basic 'hunk' of material in air, the material and its thickness is configurable
 *
 * The transverse (x,y) dimensions are set arbitrarily to 1m just to make
 * absolutely sure that we can contain the shower that may contain a dark brem.
 */
class Hunk : public G4VUserDetectorConstruction {
  /// depth along beam direction [mm]
  double depth_;
  /// name of material to use for volume (findable by G4NistManager)
  std::string material_;
  /// pointer to SD for ECal scoring plane
  ScoringPlaneSD* ecal_;
  /// pointer to biasing operator for muon conversion (if we are biasing)
  MuonConversionBiasing* muon_conversion_biasing_;
 public:
  /**
   * Create our detector constructor, storing the configuration variables
   */
  Hunk(double depth, const std::string& material, ScoringPlaneSD* ecal, MuonConversionBiasing* muon_conv_bias);

  /**
   * Construct the geometry
   *
   * ```
   *     |        |         |
   *     |  Hunk  |         |
   *     |        |         |
   *                     ECal SP
   * z  -d        0        240
   * ```
   *
   * We build the world to represent a rudimentary LDMX
   * fixed target. The hunk is placed such that its downstream
   * side is at z=0 similar to the LDMX thin target. Then
   * we have a few named volumes that don't have a different
   * material than the World volume but are named so we can
   * attach "detectors" to them so we can get the outgoing particle
   * properties at specific, LDMX-important z locations.
   */
  virtual G4VPhysicalVolume* Construct() final override;
};
