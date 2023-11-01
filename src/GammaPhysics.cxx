#include "GammaPhysics.h"

#include "G4Gamma.hh"
#include "G4ProcessManager.hh"

void GammaPhysics::ConstructParticle() {}

void GammaPhysics::ConstructProcess() {
  the_process_ = std::make_unique<G4GammaConversionToMuons>();
  G4int ret = G4Gamma::Gamma()->GetProcessManager()->AddDiscreteProcess(the_process_.get());
  if (ret < 0) {
    throw std::runtime_error(
        "Error attempting to register the muon-conversion process. Code: "+std::to_string(ret));
  }
}
