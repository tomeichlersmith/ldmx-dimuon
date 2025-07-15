# Di-Muon Events for LDMX ECal Calibration
We want to study this in more depth to see how we need to scale the resulting simulated events.

This project has two parts.
1. A light program that simply calls the G4GammaConversionToMuons cross section calculation
  function so that it can be compared to literature values for the purposes of scaling.
2. A simulation that mimics an LDMX target where the origin of the system is the downstream
  face of the target of configurable thickness and there is a scoring plane at the front
  face of the calorimeters.

## Set-Up
We use [denv](https://tomeichlersmith.github.io/denv/) to fix the development environment 
and share a pre-built version of Geant4.
In addition, this denv uses the LDMX container image so the results are for the specific version
of Geant4 built there.

Similar to ldmx-sw, we use [just](https://just.systems/man/en/) to share command recipets.
If you don't want to install `just`, you can view the [justfile](justfile) for the specific
commands referenced by name.
```
just init # once per machine, defines the denv
just config build # configures the build and compiles the code
just xsec-calc -h # run xsec-calc (prints help)
just simulate -h # run simulation (prints help)
```
The [ana](ana) subdirectory contains a Python module which can be used to load the run parameters
and events into memory for use with `awkward` arrays. It uses `uproot` to do this loading from a
ROOT file.

## References
- [Dimuon production by laser-wakefield accelerated electrons](https://journals.aps.org/prab/pdf/10.1103/PhysRevSTAB.12.111301)

## Sample Generation
Many of these samples are relatively easy to produce since the time it takes to
generate a certain EoT-worth is relatively short. We can use GNU parallel to roughly
parallelize the generation on a single machine.

Each target thickness needs two samples at each desired beam energy.
1. An inclusive sample without any filtering or biasing so the amount of "leakage" out
  the back of the target can be characterized.
2. A dimuon sample of a specific filtering threshold and biasing factor to characterize
  the outgoing kinematic distribution and the rate of muons coming out of the target.

For LDMX, we choose to stick to an 8GeV beam and tungsten target for simplicity sake. 
The 4GeV beam can be checked after a specific calibration target thickness has been 
chosen since we expect less leakthrough in the 4GeV case relative to the 8GeV case. 
This beam energy and target material assumption has been coded into the program as
the defaults and so we will not use the `--beam` or `--target` command line options below.

The radiation length of tungsten is 3.50259mm which is a helpful scale for checking
different target depths, so we often use this scale to define the different depths
of the target we are studying leading to the argument `--depth $(python -c 'print(XX*3.50259)')`.

From LDMX ECal-as-Target studies, midshower dimuon production
requires a biasing factor of ~1e4 and a filtering rate of ~1/1000 to have a decent
longitudinal distribution of origination vertices. In addition, we want to keep the muons
high above any capture/interaction thresholds and safely in the Minimum-Ionizing-Particle
(MIP) region, motivating a filter threshold of 1GeV. This is why you'll see the command
line arguments `--bias 1e4 --filter 1000`.

For example
```
depth=$(python -c 'print(X*3.50259)')
./build/dimuon-simulate --depth ${depth} 10000 inclusive_X.root
./build/dimuon-simulate --depth ${depth} --bias 1e4 --filter 1000 1000000 dimuon_X.root
```
This pair of simulations is coded into the [app/gen-samples](app/gen-samples) script
and they are both run at once.
```
just gen-samples -h
```
