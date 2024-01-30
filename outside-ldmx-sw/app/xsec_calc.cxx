/**
 * @file xsec_calc.cxx
 * definition of dimuon-xsec-calc executable
 */

#include <fstream>
#include <iostream>
#include <unistd.h>

#include "G4GammaConversionToMuons.hh"

/**
 * print out how to use g4db-xsec-calc
 */
void usage() {
  std::cout <<
    "USAGE:\n"
    "  dimuon-xsec-calc [options] OUTPUT\n"
    "\n"
    "Calculate dimuon (muon-conversion) cross sections and write them out to a CSV table\n"
    "\n"
    "ARGUMENTS\n"
    "  OUTPUT       : output file to write CSV table of total cross section calculated to\n"
    "\n"
    "OPTIONS\n"
    "  -h,--help    : produce this help and exit\n"
    "  --energy     : python-like arange for input energies in GeV (stop, start stop, start stop step)\n"
    "                 default start is 0, default stop is 10, and default step is 0.01 GeV\n"
    "  --target     : define target material with two parameters (atomic units): Z A\n"
    "                 default target material is tungsten (A=183.84 and Z=74)\n"
    << std::flush;
}

/**
 * definition of dimuon-xsec-calc
 */
int main(int argc, char* argv[]) try {
  std::string output_filename{};
  double min_energy{0.};
  double max_energy{10.};
  double energy_step{0.01};
  double target_Z{74.};
  double target_A{183.84};
  for (int i_arg{1}; i_arg < argc; ++i_arg) {
    std::string arg{argv[i_arg]};
    if (arg == "-h" or arg == "--help") {
      usage();
      return 0;
    } else if (arg == "--energy") {
      std::vector<std::string> args;
      while (i_arg+1 < argc and argv[i_arg+1][0] != '-') {
        args.push_back(argv[++i_arg]);
      }
      if (args.size() == 0) {
        std::cerr << arg << " requires arguments after it" << std::endl;
        return 1;
      } else if (args.size() == 1) {
        max_energy = std::stod(args[0]);
      } else if (args.size() == 2) {
        min_energy = std::stod(args[0]);
        max_energy = std::stod(args[1]);
      } else if (args.size() == 3) {
        min_energy = std::stod(args[0]);
        max_energy = std::stod(args[1]);
        energy_step = std::stod(args[2]);
      }
    } else if (arg == "--target") {
      std::vector<std::string> args;
      while (i_arg+1 < argc and argv[i_arg+1][0] != '-') {
        args.push_back(argv[++i_arg]);
      }
      if (args.size() != 2) {
        std::cerr << arg << " requires two arguments: Z A" << std::endl;
        return 1;
      }
      target_Z       = std::stod(args[0]);
      target_A       = std::stod(args[1]);
    } else if (argv[i_arg][0] == '-') {
      std::cout << arg << " is an unrecognized option" << std::endl;
      return 1;
    } else {
      if (not output_filename.empty()) {
        std::cerr 
          << "OUTPUT is already set to " << output_filename << "\n"
          << arg << " is either an extra argument (not allowed) or a mis-typed option\n"
          << std::flush;
        return 1;
      }
      output_filename = arg;
    }
  }

  if (output_filename.empty()) {
    usage();
    std::cerr << "\nOUTPUT has not been provided!\n" << std::flush;
    return 1;
  }

  std::ofstream table_file(output_filename);
  if (!table_file.is_open()) {
    std::cerr << "File '" << output_filename << "' was not able to be opened." << std::endl;
    return 2;
  }

  // start at max and work our way down
  //    this mimics the actual progress of a simulation slightly better
  max_energy *= CLHEP::GeV;
  energy_step *= CLHEP::GeV;
  min_energy *= CLHEP::GeV;

  std::cout 
    << "Parameter         : Value\n"
    << "Min Energy [MeV]  : " << min_energy  << "\n"
    << "Max Energy [MeV]  : " << max_energy  << "\n"
    << "Energy Step [MeV] : " << energy_step << "\n"
    << "Target A [amu]    : " << target_A << "\n"
    << "Target Z [amu]    : " << target_Z << "\n"
    << "Destination       : " << output_filename << "\n"
    << std::flush;

  /**
   * Initialize the process
   */
  G4GammaConversionToMuons process;
  
  table_file << "A [au],Z [protons],Energy [MeV],Xsec [pb]\n";

  G4double current_energy = max_energy;
  while (current_energy > min_energy - energy_step and current_energy > 0) {
    double xsec = process.ComputeCrossSectionPerAtom(current_energy, target_A, target_Z);
    table_file 
        << target_A << ","
        << target_Z << ","
        << current_energy << ","
        << xsec / CLHEP::picobarn << "\n";
    current_energy -= energy_step;
  }

  table_file.flush();
  table_file.close();

  return 0;
} catch (const std::exception& e) {
  std::cerr << "ERROR: " << e.what() << std::endl;
  return 127;
}
