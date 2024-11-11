# Di-Muon Calibration Samples within ldmx-sw
In order to begin di-muon calibration studies, we want to first look at
how dimuon events present themselves within the LDMX detector as designed.

This workspace is focused on this study: using ldmx-sw to study dimuon events.
We are using a pinned-version of ldmx-sw to replicate the first batch of
samples that were created, but we can hopefully move to a version of ldmx-sw
when I get around to validating that this stuff works.

### Table of Contents
- `config-physics-target-*.py`: ldmx-sw configuration script used to generate simulated dimuon conversion within thin "physics" target
- `config-calib-target.py`: ldmx-sw config used to generate simulated dimuon conversion within thick "calib" target
  - This config relies on the custom `detector.gdml` as well which is a copy of the GDML file that comes with
    the ldmx-det-v14-8gev detector but with the additional calibration target inserted upstream of everything else
  - Biasing objects upstream of the target requires a patch to the underlying Geant4 biasing framework which is
    available with version 4.2.1 of the ldmx/dev image or sha-39fc257 of the ldmx/pro image.

## Notes

- Unbiased Exclusive (pure EM: no PN, no muon prod), 1M events
- Unbiased Inclusive (no restrictions), 1M events
- 4GeV versions of these samples as well

