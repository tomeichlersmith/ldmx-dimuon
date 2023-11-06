#pragma once

#include <optional>

#include "G4MuonMinus.hh"
#include "G4MuonPlus.hh"
#include "G4Track.hh"
#include "G4Event.hh"
#include "G4ClassificationOfNewTrack.hh"

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
  /// particles entering the ECal
  std::vector<Particle> ecal_;
  /**
   * weight for the event
   *
   * This calculated by taking the product over all weights
   * of individual steps. The individual step weights are
   * calculated by dividing the post-step track weight by
   * the pre-step track weight. The track weights are propagated
   * by Geant4.
   */
  double weight_{1.};
  /// number of events that we simulated
  long unsigned int events_started_{0};
  /// number of events with a dark brem in it
  long unsigned int events_completed_{0};
  /// minimum energy of a muon to have to keep the event
  std::optional<double> filter_threshold_;
  /// factor to bias muon-conversion by in material target'
  std::optional<double> bias_factor_;
  /// target material (as named in G4NistManager)
  std::string target_;
  /// depth of target in mm
  double depth_;
  /// energy of beam in GeV
  double beam_;
  /// whether photons were used (true) or electrons (false)
  bool photons_;
  /// flag keeping track of current stage of simulated event
  bool no_more_particles_above_threshold_;
 public:
  /**
   * Open the output file and set whether we filter or not
   *
   * In addition to opening the output file, we create the event tree
   * and set up the branches we will write our member variables to.
   */
  PersistParticles(const std::string& out_file, std::optional<double> filter_threshold, std::optional<double> bias_factor, const std::string& target, double depth, double beam, bool photons);

  /**
   * Print out the number of events with a dark brem compared to the requested number
   *
   * Additionally, we make sure to write the output events tree and
   * close the output file.
   */
  ~PersistParticles();

  /**
   * Check on if an event is successful
   *
   * An event is defined as successful if either
   * of the following are true.
   * 1. We are not filtering in which case all events
   *    should be considered successful and thus kept
   * 2. The incident, parent, mu+, and mu- particles
   *    are all valid (i.e. we have found a muon-conversion)
   */
  bool success();

  /**
   * Clear the map of particles so that the new event doesn't
   * copy any data leftover from the last event
   *
   * @param[in] event unused
   */
  void BeginOfEventAction(const G4Event* event);

  /**
   * Classify a track that is about to be processed
   *
   * Geant4 slightly mis-names this function since sometimes
   * tracks that aren't new will end up being put into this
   * function. This occurs if they are put onto the waiting stack
   * after being partially processed.
   *
   * What we do here is we put all the tracks below our filtering
   * threshold onto the waiting stack until there are no more
   * particles above the threshold. This forces the particles
   * above the filtering threshold to be processed first and
   * gives us an opportunity in NewStage to check if the event
   * has been successful before the entire shower has been
   * simulated.
   */
  G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* track);

  /**
   * Currently does nothing
   *
   * Called when a track is about to be processed
   *
   * @param[in] track unused
   */
  void PreUserTrackingAction(const G4Track* track);

  /**
   * If a particle is a photon, check if it just underwent
   * the muon-conversion process. If it did, label it as the
   * parent.
   *
   * @param[in] step current step being processed
   */
  void UserSteppingAction(const G4Step* step);

  /**
   * A scoring plane has a hit that should be handled by us
   *
   * @param[in] name name of the scoring plane hit
   * @param[in] step current G4Step that happened in the plane
   */
  void NewScoringPlaneHit(const G4String& name, const G4Step* step);

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
   * Abort the event early if it hasn't been successful yet
   *
   * NewStage is called when all tracks on the Urgent stack
   * have been processed. For our purposes, this means all
   * particles with total energy above the filtering threshold
   * have been processed and therefore if we have not successfully
   * produced a muon-conversion yet, we do not have enough energy
   * to do so and so we should abort the event.
   */
  void NewStage();

  /**
   * Check and write if successful
   *
   * @see success for how successful is defined
   */
  void EndOfEventAction(const G4Event* event);
};  // PersistDarkBremProducts
