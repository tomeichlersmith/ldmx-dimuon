cmake_minimum_required(VERSION 3.12)

project(ldmx-dimuon
  VERSION 0.5.0
  DESCRIPTION "Studying usage of thick, calibration target to generate di-muon events"
  LANGUAGES CXX)

find_package(ROOT 6.16 CONFIG REQUIRED)
include("${ROOT_DIR}/RootMacros.cmake")

find_package(Geant4 10.2.3 REQUIRED)
include(${Geant4_USE_FILE})

add_executable(dimuon-xsec-calc app/xsec_calc.cxx)
target_link_libraries(dimuon-xsec-calc PRIVATE ${Geant4_LIBRARIES})

configure_file(
  ${PROJECT_SOURCE_DIR}/src/Version.h.in
  ${PROJECT_BINARY_DIR}/include/Version.h)

add_library(DimuonSimulation
  SHARED
  src/RunHeader.cxx
  src/Particle.cxx
  src/Hunk.cxx
  src/PersistParticles.cxx
  src/Beam.cxx
  src/GammaPhysics.cxx
  src/ScoringPlaneSD.cxx
  src/MuonConversionBiasing.cxx
)
target_include_directories(DimuonSimulation PUBLIC src ${PROJECT_BINARY_DIR}/include)
target_link_libraries(DimuonSimulation PUBLIC ${Geant4_LIBRARIES} ROOT::Core ROOT::TreePlayer)
target_compile_definitions(DimuonSimulation PUBLIC "DEBUG=$<IF:$<CONFIG:Debug>,1,0>")
root_generate_dictionary(
  DimuonSimulationEventDict
  src/RunHeader.h
  src/Particle.h
  LINKDEF src/LinkDef.h
  MODULE DimuonSimulation
)

add_executable(dimuon-simulate app/simulate.cxx)
target_link_libraries(dimuon-simulate PRIVATE DimuonSimulation)

set_target_properties(
  DimuonSimulation DimuonSimulationEventDict dimuon-simulate
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED YES
             CXX_EXTENSIONS NO
)

