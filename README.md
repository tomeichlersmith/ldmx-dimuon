# Geant4 GammaConversionToMuons
We want to study this in more depth to see how we need to scale the resulting simulated events.

Currently, this project just has a light C++ program which calls the Geant4 method calculating
this process's cross section during the simulation. This means we can extract the simulated
cross section as a function of energy and compare it to literature to help with scaling.

If there is motivation, we might evolve this project into studying the differential rate as
well in order to compare the literature recoil kinematics to what is being done within Geant4.

## Set-Up
We use [denv](https://tomeichlersmith.github.io/denv/) to fix the development environment 
and share a pre-built version of Geant4.
In addition, this denv uses the LDMX container image so the results are for the specific version
of Geant4 built there.

Inside the denv, we have a few pre-defined aliases (in `.bash_aliases`) that make the compiling
and running a bit more ergonomic. After cloning this repository, one could enter the denv and
`run` this project to start calculating cross sections.
```
denv
xsec-calc
```
