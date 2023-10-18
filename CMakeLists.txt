cmake_minimum_required(VERSION 3.12)

project(ldmx-dimuon
  VERSION 0.0.0
  DESCRIPTION "Test running of the G4GammaConversionToMuons class"
  LANGUAGES CXX)

# Search for Geant4 and load its settings
find_package(Geant4 10.2.3 REQUIRED)

include(${Geant4_USE_FILE})

add_executable(dimuon-xsec-calc app/xsec_calc.cxx)
target_link_libraries(dimuon-xsec-calc PRIVATE ${Geant4_LIBRARIES})

