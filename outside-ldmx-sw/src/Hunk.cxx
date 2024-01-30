#include "Hunk.h"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"

Hunk::Hunk(double depth, const std::string& material, ScoringPlaneSD* ecal, MuonConversionBiasing* muon_conv_bias)
  : G4VUserDetectorConstruction(),
    depth_{depth},
    material_{material},
    ecal_{ecal},
    muon_conversion_biasing_{muon_conv_bias}
{}

G4VPhysicalVolume* Hunk::Construct() {
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

  static const double ecal_sp = 240*mm;
  G4double world_half_z = (1+depth_+ecal_sp+1);
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

  G4Box* solidBox = new G4Box("Hunk",
      box_half_x, box_half_y, box_half_z);

  G4LogicalVolume* logicBox = new G4LogicalVolume(solidBox,
      box_mat, "Hunk");
  if (muon_conversion_biasing_) muon_conversion_biasing_->AttachTo(logicBox);

  // providing mother volume attaches us to the world volume
  new G4PVPlacement(0, //no rotation
      G4ThreeVector(0.,0.,-box_half_z), //at (0,0,-d/2)
      logicBox,        //its logical volume
      "Hunk",          //its name
      logicWorld,      //its mother  volume
      false,           //no boolean operation
      0,               //copy number
      false);          //overlaps checking
 
  G4Box* solidScoringPlane = new G4Box("ScoringPlane",
      box_half_x, box_half_y, 1*mm);

  G4LogicalVolume* ecalScoringPlane = new G4LogicalVolume(solidScoringPlane,
      world_mat, "EcalScoringPlane");
  ecalScoringPlane->SetSensitiveDetector(ecal_);

  // ECal Scoring Plane
  new G4PVPlacement(0,
      G4ThreeVector(0.,0.,ecal_sp),
      ecalScoringPlane,
      "EcalScoringPlane",
      logicWorld,
      false,
      0,
      false);

  //always return the physical World
  return physWorld;
}
