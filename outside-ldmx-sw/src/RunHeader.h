#pragma once

#include <optional>

#include "TObject.h"

/**
 * The object that we use to store data about how the
 * run was produced.
 */
class RunHeader {
  /// total number of events started (pre-filtering)
  int tries_;
  /// was muon filtering activated?
  bool filter_;
  /// the filter threshold if it was active [MeV]
  double filter_threshold_;
  /// biasing factor applied to muon-conversion within the target
  /// (set to 1. if no biasing was done)
  double bias_factor_;
  /// target material as named in G4NistManager
  std::string target_;
  /// depth of target in mm
  double depth_;
  /// energy of beam in GeV
  double beam_;
  /// whether photons were used (true) or electrons (false)
  bool photons_;
  /// integer used to seed Geant4's RNG
  long seed_;
  /// major version number used to produce this run
  int version_major_;
  /// minor version number used to produce this run
  int version_minor_;
  /// patch version number used to produce this run
  int version_patch_;
  ClassDef(RunHeader, 1);
 public:
  /// default constructor necessary for ROOT serialization
  RunHeader() = default;
  /// virtual destructor necessary for ROOT serialization
  virtual ~RunHeader() = default;
  /**
   * Actual constructor used for creating the object during processing
   *
   * @param[in] tries total number of events begun during production
   * @param[in] filter_threshold optional filter threshold
   * @param[in] bias_factor factor applied to muon-conversion within the target
   */
  RunHeader(
      int tries,
      std::optional<double> filter_threshold,
      std::optional<double> bias_factor,
      const std::string& target,
      double depth,
      double beam,
      bool photons,
      long seed
  );
};
