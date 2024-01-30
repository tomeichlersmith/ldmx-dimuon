#include "MuonConversionBiasing.h"

#include "G4Gamma.hh"
#include "G4BiasingProcessInterface.hh"
#include "G4Track.hh"

G4VBiasingOperation* MuonConversionBiasing::ProposeOccurenceBiasingOperation(const G4Track* track, const G4BiasingProcessInterface* callingProcess) {
  // only biasing photons
  if (track->GetDefinition() != G4Gamma::Gamma()) return 0;
  // only biasing photons above the configured threshold in energy
  if (track->GetKineticEnergy() < threshold_) return 0;
  // only biasing the muon-conversion process
  std::string process_name = callingProcess->GetWrappedProcess()->GetProcessName();
  if (process_name.compare("GammaToMuPair")!=0) return 0;
  // got here with a photon and the muon-conversion process
  double interaction_length = callingProcess->GetWrappedProcess()->GetCurrentInteractionLength();
  double unbiased_xsec = 1./interaction_length;
  double biased_xsec = unbiased_xsec * factor_;
  operation_->SetBiasedCrossSection(biased_xsec);
  operation_->Sample();
  return operation_;
}
G4VBiasingOperation* MuonConversionBiasing::ProposeFinalStateBiasingOperation(const G4Track*, const G4BiasingProcessInterface*) {
  return 0;
}
G4VBiasingOperation* MuonConversionBiasing::ProposeNonPhysicsBiasingOperation(const G4Track*, const G4BiasingProcessInterface*) {
  return 0;
}
MuonConversionBiasing::MuonConversionBiasing(double factor, double threshold)
  : G4VBiasingOperator("bias-muon-conv"), factor_{factor}, threshold_{threshold}, operation_{nullptr} {}
MuonConversionBiasing::~MuonConversionBiasing() {
  if (operation_) delete operation_;
}
void MuonConversionBiasing::StartRun() {
  // re-starting run somehow so we are already configured
  if (operation_) return;
  operation_ = new G4BOptnChangeCrossSection("xsec-bias-muon-conv");
}
