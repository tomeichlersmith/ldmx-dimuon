/**
 * @file simulate.cxx
 * definition of dimuon-simulate executable
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <array>

#include "QBBC.hh"
#include "G4PhysListFactory.hh"
#include "G4VPhysicsConstructor.hh"
#include "G4VUserDetectorConstruction.hh"
#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4UserEventAction.hh"
#include "G4UserTrackingAction.hh"
#include "G4RunManager.hh"
#include "G4MuonMinus.hh"
#include "G4Electron.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4NistManager.hh"
#include "G4GammaConversionToMuons.hh"

#include "TFile.h"
#include "TTree.h"

/**
 * basic physics constructor which simply constructs the GammaConversionToMuons
 * process and adds it to the Gamma process table.
 */
class GammaPhysics : public G4VPhysicsConstructor {
  /// handle to the process, cleaned up when the physics list is desctructed
  std::unique_ptr<G4GammaConversionToMuons> the_process_;
 public:
  /// create the physics and store the parameters
  GammaPhysics() = default;

  /**
   * We don't construct any particles since we are just
   * attaching a new process to an existing particle
   */
  void ConstructParticle() final override {}

  /**
   * Construct and configure the muon-conversion process
   *
   * We own the process and clean it up when the physics constructor
   * is cleaned up by Geant4 after registration.
   *
   * Besides the required configuration parameters (the path
   * to the dark brem library and whether or not we are dark-bremming off muons),
   * we leave the rest of the sim parameters to their default values.
   * Look at G4DarkBreMModel::G4DarkBreMModel for the default value
   * definitions to see if your situation requires changing any of them.
   */
  void ConstructProcess() final override {
    the_process_ = std::make_unique<G4GammaConversionToMuons>();
    G4int ret = G4Gamma::Gamma()->GetProcessManager()->AddDiscreteProcess(the_process_.get());
    if (ret < 0) {
      throw std::runtime_error(
          "Error attempting to register the muon-conversion process. Code: "+std::to_string(ret));
    }
  }
};  // APrimePhysics

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
  Hunk(double d, const std::string& m)
    : G4VUserDetectorConstruction(), depth_{d}, material_{m} {}

  /**
   * Construct the geometry
   *
   * We build the world only slighly larger than the single hunk
   * of material at its center. The hunk is shifted to be
   * downstream (along z) of the origin so that the primary generator
   * can simply shoot from the origin along z.
   */
  virtual G4VPhysicalVolume* Construct() final override {
    // Get nist material manager
    G4NistManager* nist = G4NistManager::Instance();
    using CLHEP::mm;

    G4double box_half_x{500*mm},
             box_half_y{500*mm},
             box_half_z{depth_/2*mm};
    G4Material* box_mat = nist->FindOrBuildMaterial(material_);
    if (not box_mat) {
      throw std::runtime_error("Material '"+material_+"' unknown to G4NistManager.");
    }

    G4Material* world_mat = nist->FindOrBuildMaterial("G4_AIR");
    if (not world_mat) {
      throw std::runtime_error("Material 'G4_AIR' unknown to G4NistManager.");
    }

    G4double world_half_z = 2*box_half_z+2;
    G4Box* solidWorld =
      new G4Box("World", 1.1*box_half_x, 1.1*box_half_y, world_half_z);

    G4LogicalVolume* logicWorld =
      new G4LogicalVolume(solidWorld,
          world_mat,
          "World");

    G4VPhysicalVolume* physWorld =
      new G4PVPlacement(0, //no rotation
          G4ThreeVector(), // center nudged upstream a bit
          logicWorld,      //its logical volume
          "World",         //its name
          0,               //its mother  volume
          false,           //no boolean operation
          0,               //copy number
          false);          //overlaps checking

    G4Box* solidBox = new G4Box("Box",
        box_half_x, box_half_y, box_half_z);

    G4LogicalVolume* logicBox = new G4LogicalVolume(solidBox,
        box_mat, "Box");

    // providing mother volume attaches us to the world volume
    new G4PVPlacement(0, //no rotation
        G4ThreeVector(0.,0.,box_half_z+1), //at (0,0,box_half_z+1)
        logicBox,        //its logical volume
        "Envelope",      //its name
        logicWorld,      //its mother  volume
        false,           //no boolean operation
        0,               //copy number
        false);          //overlaps checking

    //always return the physical World
    return physWorld;
  }
};

/**
 * the primary generator, a simple particle gun restricted to electrons or photons
 * along the z axis
 */
class Beam : public G4VUserPrimaryGeneratorAction {
  /// the gun we use for the beam
  G4ParticleGun gun_;
 public:
  /**
   * Configure the beam to be of the input energy and particle
   *
   * Shoot along the z axis, the energy is in GeV and we
   * shoot from the origin.
   */
  Beam(double energy, bool photons)
    : G4VUserPrimaryGeneratorAction() {
      if (photons) gun_.SetParticleDefinition(G4Gamma::Gamma());
      else gun_.SetParticleDefinition(G4Electron::Electron());
      gun_.SetParticleEnergy(energy*CLHEP::GeV);
      gun_.SetParticlePosition(G4ThreeVector());
      gun_.SetParticleMomentumDirection(G4ThreeVector(0.,0.,1.));
    }

  /**
   * Start an event by providing primaries
   */
  void GeneratePrimaries(G4Event* event) final override {
    gun_.GeneratePrimaryVertex(event);
  }
};

/**
 * The object that we will use to store particle information
 */
struct Particle {
  int pdg_id;
  std::array<float, 4> momentum;
};

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
  /// number of events that we simulated
  long unsigned int events_started_{0};
  /// number of events with a dark brem in it
  long unsigned int events_completed_{0};
  /// the map of particles that we are going to store
  std::map<int, Particle> particles_;
  /// have we found a muon-conversion in this event?
  bool found_{false};
 public:
  /**
   * Open the output CSV and write the header row
   */
  PersistParticles(const std::string& out_file)
    : out_{out_file.c_str(), "RECREATE"} {
      out_.cd();
      events_ = new TTree("dimuon_events","dimuon_events");
      events_->Branch("particles", &particles_);
  }

  /**
   * Print out the number of events with a dark brem compared to the requested number
   */
  ~PersistParticles() {
    std::cout << "[dimuon-simulate] Able to generate a muon conversion " 
      << events_completed_ << " / " << events_started_ 
      << " events" << std::endl;
    events_->Write();
    out_.Close();
  }

  /**
   * Clear the map of particles so that the new event doesn't
   * copy any data leftover from the last event
   */
  void BeginOfEventAction(const G4Event*) {
    particles_.clear();
    found_ = false;
    ++events_started_;
  }

  /**
   * Check each track after it is processed.
   *
   * If the track has a secondary via the muon-conversion process
   * or if the track is a secondary of the muon-conversion process
   * or if the track's endpoint is outside the material hunk,
   * we choose to keep the track.
   */
  void PostUserTrackingAction(const G4Track* track) {
    
  }

  /**
   * Check and write if successful
   */
  void EndOfEventAction(const G4Event*) {
    if (found_) {
      ++events_completed_;
      events_->Fill();
    }
  }
};  // PersistDarkBremProducts

class TrackingAction : public G4UserTrackingAction {
  PersistParticles& persister_;
 public:
  TrackingAction(PersistParticles& persister)
    : G4UserTrackingAction(), persister_{persister} {}
  void PostUserTrackingAction(const G4Track* track) final override {
    persister_.PostUserTrackingAction(track);
  }
};

class EventAction : public G4UserEventAction {
  PersistParticles& persister_;
 public:
  EventAction(PersistParticles& persister)
    : G4UserEventAction(), persister_{persister} {}
  void BeginOfEventAction(const G4Event* event) final override {
    persister_.BeginOfEventAction(event);
  }
  void EndOfEventAction(const G4Event* event) final override {
    persister_.EndOfEventAction(event);
  }
};


/**
 * print out how to use g4db-simulate
 */
void usage() {
  std::cout <<
    "\n"
    "USAGE\n"
    "  g4db-simulate [options] NUM-EVENTS\n"
    "\n"
    "ARGUMENTS\n"
    "  NUM-EVENTS : number of events to **request**\n"
    "               since Geant4 decides when a dark brem will occurr, it is important\n"
    "               to allow some beam leptons to /not/ dark brem in the target so a realistic\n"
    "               distribution of dark brem vertices is sampled."
    "\n"
    "OPTIONS\n"
    "  -h, --help    : print this usage and exit\n"
    "  --photons     : run using photons (without this flag, assumes electrons)\n"
    "  -d, --depth   : thickness of target in mm\n"
    "  -t, --target  : target material, must be findable by G4NistManager\n"
    "                  (defaults to G4_W for electrons and G4_Cu for muons)\n"
    "  -b, --bias    : biasing factor to use to encourage dark brem\n"
    "                  a good starting point is generally the A' mass squared, so that is the default\n"
    "  -e, --beam    : Beam energy in GeV (defaults to 4 for electrons and 100 for muons)\n"
    "  --mat-list    : print the full list from G4NistManager and exit\n"
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
  double depth{100.};
  std::string target{"G4_W"};
  double bias{1.};
  double beam{10.};
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
    std::cerr << "Exactly two positional arguments are required: NUM-EVENTS OUTPUT" << std::endl;
    return 1;
  }

  int num_events = std::stoi(positional[0]);
  std::string output = positional[1];

  auto run = std::unique_ptr<G4RunManager>(new G4RunManager);

  run->SetUserInitialization(new Hunk(depth,target));

  G4VModularPhysicsList* physics = new QBBC;
  physics->RegisterPhysics(new GammaPhysics());
  run->SetUserInitialization(physics);

  run->Initialize();

  PersistParticles persister(output);
  run->SetUserAction(new TrackingAction(persister));
  run->SetUserAction(new EventAction(persister));

  run->BeamOn(num_events);

  return 0;
} catch (const std::exception& e) {
  std::cerr << "ERROR: " << e.what() << std::endl;
  return 127;
}
