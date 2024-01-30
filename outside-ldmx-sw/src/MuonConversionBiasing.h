#pragma once

#include "G4VBiasingOperator.hh"
#include "G4BOptnChangeCrossSection.hh"

class MuonConversionBiasing : public G4VBiasingOperator {
  /// the configured factor we will use to bias the muon-conversion process
  double factor_;
  /// energy threshold above which photons need to be to be biased
  double threshold_;
  /// the operation we can give to Geant4 when we want to bias
  G4BOptnChangeCrossSection* operation_;
 private:
  /**
   * propose a biasing operation that will be used to change the occurence of the physics process
   * 
   * @param[in] track stepping through volumes this operator has been attached to
   * @param[in] callingProcess process that may be happening to the track
   * @return pointer to operation to use for biasing (0 or nullptr if no biasing should be applied)
   */
  virtual G4VBiasingOperation* ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess) final;
  /// return 0 and don't propose final-state biasing
  virtual G4VBiasingOperation* ProposeFinalStateBiasingOperation(const G4Track*, const G4BiasingProcessInterface*) final;
  /// return 0 and don't propose non-physics biasing
  virtual G4VBiasingOperation* ProposeNonPhysicsBiasingOperation(const G4Track*, const G4BiasingProcessInterface*) final;
 public:
  /**
   * Create this biasing operator with the input factor to increase the muon-conversion xsec by
   */
  MuonConversionBiasing(double factor, double threshold);
  /**
   * Close up this operator and delete the operation if it exists
   */
  virtual ~MuonConversionBiasing();
  /**
   * Initialize the operator during the start of the run.
   *
   * We double check that the process we want to bias is wrapped by the biasing
   * interface and then create the biasing operation if it is.
   */
  virtual void StartRun() final;
};
