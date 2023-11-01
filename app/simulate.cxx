/**
 * @file simulate.cxx
 * definition of dimuon-simulate executable
 */

#include <fstream>
#include <iostream>
#include <memory>

#include "QBBC.hh"
#include "G4PhysListFactory.hh"
#include "G4UserEventAction.hh"
#include "G4UserTrackingAction.hh"
#include "G4UserSteppingAction.hh"
#include "G4UserStackingAction.hh"
#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4UIsession.hh"
#include "G4UImanager.hh"
#include "G4GenericBiasingPhysics.hh"

#include "Beam.h"
#include "GammaPhysics.h"
#include "Hunk.h"
#include "PersistParticles.h"

class SilenceGeant : public G4UIsession {
  G4UIsession* SessionStart() { return nullptr; }
  G4int ReceiveG4cout(const G4String&) { return 0; }
  G4int ReceiveG4cerr(const G4String&) { return 0; }
};

class SteppingAction : public G4UserSteppingAction {
  PersistParticles& persister_;
 public:
  SteppingAction(PersistParticles& persister)
    : G4UserSteppingAction(), persister_{persister} {}
  void UserSteppingAction(const G4Step* step) final {
    persister_.UserSteppingAction(step);
  }
};

class TrackingAction : public G4UserTrackingAction {
  PersistParticles& persister_;
 public:
  TrackingAction(PersistParticles& persister)
    : G4UserTrackingAction(), persister_{persister} {}
  void PreUserTrackingAction(const G4Track* track) final {
    persister_.PreUserTrackingAction(track);
  }
  void PostUserTrackingAction(const G4Track* track) final {
    persister_.PostUserTrackingAction(track);
  }
};

class EventAction : public G4UserEventAction {
  PersistParticles& persister_;
 public:
  EventAction(PersistParticles& persister)
    : G4UserEventAction(), persister_{persister} {}
  void BeginOfEventAction(const G4Event* event) final {
    persister_.BeginOfEventAction(event);
  }
  void EndOfEventAction(const G4Event* event) final {
    persister_.EndOfEventAction(event);
  }
};

class StackingAction : public G4UserStackingAction {
  PersistParticles& persister_;
 public:
  StackingAction(PersistParticles& persister)
    : G4UserStackingAction(), persister_{persister} {}
  G4ClassificationOfNewTrack ClassifyNewTrack(const G4Track* track) final {
    return persister_.ClassifyNewTrack(track);
  }
  void NewStage() final {
    persister_.NewStage();
  }
};


/**
 * print out how to use g4db-simulate
 */
void usage() {
  std::cout <<
    "\n"
    "USAGE\n"
    "  g4db-simulate [options] NUM-EVENTS OUTPUT\n"
    "\n"
    "ARGUMENTS\n"
    "  NUM-EVENTS : number of events to **request**\n"
    "               since Geant4 decides when a muon-conv will occurr, it is important\n"
    "               to allow some beam leptons to /not/ muon-conv in the target so a realistic\n"
    "               distribution is simulated.\n"
    "  OUTPUT     : output ROOT file to write muon-conversion events to\n"
    "\n"
    "OPTIONS\n"
    "  -h, --help    : print this usage and exit\n"
    "  --photons     : run using photons (without this flag, assumes electrons)\n"
    "  -d, --depth   : thickness of target in mm\n"
    "                  default is 0.350259mm (or 0.1X0 of tungsten)\n"
    "  -t, --target  : target material, must be findable by G4NistManager\n"
    "                  defaults to G4_W (tungsten)\n"
    "  -f, --filter  : define the filtering threshold in MeV above which one (or both) of the muons must be\n"
    "                  default is no filtering (i.e. there can be no muons or muons with any energy)\n"
    "  -b, --bias    : biasing factor to use to encourage muon-conv\n"
    "                  default if this flag is not provided is no biasing\n"
    "  -e, --beam    : Beam energy in GeV (defaults to 8)\n"
    "  --mat-list    : print the full list from G4NistManager and exit\n"
    "\n"
    "EXAMPLES\n"
    "  Run an unbiased and unfiltered simulation, keeping all events which can help us\n"
    "  determine how often particles 'leak' out of the back of the target of the input thickness.\n"
    "\n"
    "    g4db-simulate --depth 10*3.50259 --target G4_W --beam 8.0 10000 inclusive_10X0.root\n"
    "\n"
    "  Run a simulation specifically biasing and filtering for muon-conversion within the target.\n"
    "\n"
    "    g4db-simulate --depth 10*3.50259 --target G4_W --beam 8.0 --bias 1e4 --filter 1000 10000 dimuon_10X0.root\n"
    "\n"
    "  Print the list of materials in G4NistManager so we can get find the name for a material we want.\n"
    "\n"
    "    g4db-simulate --mat-list | less\n"
    "\n"
    << std::flush;
}

/**
 * definition of g4db-simulate
 *
 * After parsing the command line arguments, we simply do the 
 * standard initialization and running procedure for Geant4.
 */
int main(int argc, char* argv[]) try {
  bool photons{false};
  double depth{0.350259};
  std::string target{"G4_W"};
  std::optional<double> bias{};
  std::optional<double> filter_threshold{};
  double beam{8.};
  std::vector<std::string> positional;
  for (int i_arg{1}; i_arg < argc; ++i_arg) {
    std::string arg{argv[i_arg]};
    if (arg == "-h" or arg == "--help") {
      usage();
      return 0;
    } else if (arg == "--mat-list") {
      auto nist{G4NistManager::Instance()};
      nist->ListMaterials("simple");
      nist->ListMaterials("compound");
      nist->ListMaterials("hep");
      return 0;
    } else if (arg == "--photons") {
      photons = true;
    } else if (arg == "-t" or arg == "--target") {
      if (i_arg+1 >= argc) {
        std::cerr << arg << " requires an argument after it" << std::endl;
        return 1;
      }
      target = argv[++i_arg];
    } else if (arg == "-d" or arg == "--depth") {
      if (i_arg+1 >= argc) {
        std::cerr << arg << " requires an argument after it" << std::endl;
        return 1;
      }
      depth = std::stod(argv[++i_arg]);
    } else if (arg == "-b" or arg == "--bias") {
      if (i_arg+1 >= argc) {
        std::cerr << arg << " requires an argument after it" << std::endl;
        return 1;
      }
      bias = std::stod(argv[++i_arg]);
    } else if (arg == "-f" or arg == "--filter") {
      if (i_arg+1 >= argc) {
        std::cerr << arg << " requires an argument after it" << std::endl;
        return 1;
      }
      filter_threshold = std::stod(argv[++i_arg]);
    } else if (arg == "-e" or arg == "--beam") {
      if (i_arg+1 >= argc) {
        std::cerr << arg << " requires an argument after it" << std::endl;
        return 1;
      }
      beam = std::stod(argv[++i_arg]);
    } else if (arg[0] == '-') {
      std::cerr << arg << " is not a recognized option" << std::endl;
      return 1;
    } else {
      positional.push_back(arg);
    }
  }

  if (positional.size() != 2) {
    usage();
    std::cerr << "Exactly two positional arguments are required: NUM-EVENTS OUTPUT" << std::endl;
    return 1;
  }

  int num_events = std::stoi(positional[0]);
  std::string output = positional[1];

#if(DEBUG == 0)
  SilenceGeant silence;
  G4UImanager::GetUIpointer()->SetCoutDestination(&silence);
#endif

  auto run = std::unique_ptr<G4RunManager>(new G4RunManager);

  PersistParticles persister(output, filter_threshold);
  ScoringPlaneSD ecal("ecal", persister);

  run->SetUserInitialization(
      new Hunk(
        depth,
        target,
        new ScoringPlaneSD("ecal", persister),
        bias ? new MuonConversionBiasing(bias.value(), filter_threshold.value_or(0.)) : nullptr
      )
  );

  G4VModularPhysicsList* physics = new QBBC;
  physics->RegisterPhysics(new GammaPhysics);
  if (bias) {
    G4GenericBiasingPhysics* biased_physics = new G4GenericBiasingPhysics;
    biased_physics->Bias("gamma", {"GammaToMuPair"});
    physics->RegisterPhysics(biased_physics);
  }
  run->SetUserInitialization(physics);

  run->Initialize();
  run->SetUserAction(new SteppingAction(persister));
  run->SetUserAction(new TrackingAction(persister));
  run->SetUserAction(new EventAction(persister));
  run->SetUserAction(new Beam(beam, depth, photons));
  run->SetUserAction(new StackingAction(persister));

  run->BeamOn(num_events);

  return 0;
} catch (const std::exception& e) {
  std::cerr << "ERROR: " << e.what() << std::endl;
  return 127;
}
