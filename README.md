# Di-Muon Calibration within ldmx-sw
In order to begin di-muon calibration studies, we want to first look at
how dimuon events present themselves within the LDMX detector as designed.

This repo is focused on this study: using ldmx-sw to study dimuon events.

### Table of Contents
- `target_mumu.py`: ldmx-sw configuration script used to generate simulated dimuon conversion within thin "physics" target
- `calib-hunk.py`: ldmx-sw config used to generate simulated dimuon conversion within thick "calib" target
  - This config relies on the custom `detector.gdml` as well which is a copy of the GDML file that comes with
    the ldmx-det-v14-8gev detector but with the additional calibration target inserted upstream of everything else
  - Biasing objects upstream of the target requires a patch to the underlying Geant4 biasing framework which is
    available with version 4.2.1 of the ldmx/dev image or sha-39fc257 of the ldmx/pro image.
- `requirements.txt`: list of primary requirements and their pinned versions for python-based analysis
- `outside-ldmx-sw`: a Geant4+ROOT stand-alone program separate from ldmx-sw that just models a single material hunk and a downstream plane that is roughly the distance between the physics target and the ECal.

### Start Up
In whatever environment you choose, we should use the same versions of python packages
so the analysis code can be ensured to return the same results and behave in the same
way.
```
pip install -r requirements.txt
```
If you see a "version not available" error, that means you are probably using a python
version that is too old to use the versions listed in the requirements file.

Personally, in order to obtain newer versions of python packages, I use 
[denv](https://tomeichlersmith.github.io/denv/) to
get an image with the latest release of python in it.
```
denv init python:3
denv python3 -m pip install -r requirements.txt
denv jupyter lab
```

## Project Steps

### 0. Generate Dimuon Events
Tom will handle this step and will put the resulting data files in the shared data space for analysis.

### 1. Select Calibration Events
We need to develop a selection that is _both_ simple _and_ selects "clean" events from the simulated events.
One idea for selecting these events (as suggested by Jeremy) is to define a window of energy (both a minimum
and maximum) such that a layer isn't "too quiet" or "too loud". The thresholds are TBD but the idea is to
1. Get the total energy deposited in each layer.
2. Count the number of layers within this energy window.
3. Keep events where at least N layers lie within this energy window.

This should select events where "enough" of the ECal is able to be calibrated by the event, mimicing a
"calibration trigger" or some other analytical selection that would be necessary to prune messy events
in data.

### 2. Study Illumination Uniformity
We want to know how uniformly the muons hit the different ECal cells. This can be studied in a multitude
of ways. Some ideas are

- Count muon-caused hits binned by cell in each layer
- Distribution of truth muon polar and azimuthal angles relative to the beam direction
- To check for bias, look at truth muon energy as a function of angles (or cell transverse position)
