#pragma once

#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4Track.hh"
#include "G4Event.hh"

#include "TFile.h"
#include "TTree.h"

#include "Particle.h"

/**
 * user action used to store the sim particles *if* a muon-conversion occurred
 *
 * We don't do any caching, just trusting the std::ofstream to handle the caching,
 * only flushing when necessary and when destructed.
 *
 * We also print out the number of events that successfully had a dimuon compared
 * to the number of events requested. This is helpful for the user so that they
 * know (1) there is not a problem and (2) potential tuning of the bias factor.
 */
class PersistParticles {
  /// the output file we are writing to
  TFile out_;
  /// the events tree in the output file we are writing to
  TTree* events_;
  /// the incident particle
  Particle incident_;
  /// the parent particle of the mu+mu-
  Particle parent_;
  /// the outgoing mu+
  Particle mu_plus_;
  /// the outgoing mu-
  Particle mu_minus_;
  /// other extra particles leaving the target
  std::vector<Particle> extra_;
  /// number of events that we simulated
  long unsigned int events_started_{0};
  /// number of events with a dark brem in it
  long unsigned int events_completed_{0};
 public:
  /**
   * Open the output CSV and write the header row
   */
  PersistParticles(const std::string& out_file);

  /**
   * Print out the number of events with a dark brem compared to the requested number
   */
  ~PersistParticles();

  /**
   * Clear the map of particles so that the new event doesn't
   * copy any data leftover from the last event
   *
   * @param[in] event unused
   */
  void BeginOfEventAction(const G4Event* event);

  void PreUserTrackingAction(const G4Track* track);

  /**
   * Check each track after it is processed.
   *
   * If the track has a secondary via the muon-conversion process
   * or if the track is a secondary of the muon-conversion process
   * or if the track's endpoint is outside the material hunk,
   * we choose to keep the track.
   */
  void PostUserTrackingAction(const G4Track* track);

  /**
   * Check and write if successful
   */
  void EndOfEventAction(const G4Event* event);
};  // PersistDarkBremProducts
