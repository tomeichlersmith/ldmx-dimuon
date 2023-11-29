# Di-Muon Calibration within ldmx-sw
In order to begin di-muon calibration studies, we want to first look at
how dimuon events present themselves within the LDMX detector as designed.

This directory is focused on this study: using ldmx-sw to study dimuon events.

### Table of Contents
- `target_mumu.py`: ldmx-sw configuration script used to generate simulated data
- `requirements.txt`: list of primary requirements and their pinned versions for python-based analysis

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
denv python3 -m pip install -r requirements
denv jupyter lab
```
