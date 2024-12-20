# Di-Muon Calibration within ldmx-sw
In order to begin di-muon calibration studies, we want to first look at
how dimuon events present themselves within the LDMX detector as designed.

This repo is focused on this study: using ldmx-sw to study dimuon events.

### Table of Contents
- `outside-ldmx-sw`: a Geant4+ROOT stand-alone program separate from ldmx-sw that just models a single material hunk and a downstream plane that is roughly the distance between the physics target and the ECal.
- `production`: producing samples for this study within ldmx-sw

### Start Up
In whatever environment you choose, we should use the same versions of python packages
so the analysis code can be ensured to return the same results and behave in the same
way.
```
pip install -r .denv/requirements.txt
```
If you see a "version not available" error, that means you are probably using a python
version that is too old to use the versions listed in the requirements file.

Personally, in order to obtain newer versions of python packages, I use 
[denv](https://tomeichlersmith.github.io/denv/) to
get an image with the latest release of python in it.
```
denv python3 -m pip install -r .denv/requirements.txt
denv jupyter lab
```

## Notes

### Rate Questions
- Fraction of events still rejected after biasing? i.e. estimate muon relative rate
- Whats the max charge per bunch in lcls-ii mechanism?

This would inform how large the bunches should be. For example, if the beam delivery rate is 1MHz
and we want to get down to 10kHz being readout (i.e. a factor of 1/100), then we would want 10k electrons
per bunch if the muon relative rate is 1e-6 (i.e. also a factor of 1/100).

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
