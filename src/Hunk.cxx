#include "Hunk.h"

#include "G4NistManager.hh"
#include "G4Box.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalVolume.hh"

Hunk::Hunk(double depth, const std::string& material)
  : G4VUserDetectorConstruction(),
    depth_{depth},
    material_{material}
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
