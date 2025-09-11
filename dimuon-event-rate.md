# Di-Muon Event Rates

The estimate of di-muon event rates is very rough, but I'm leaving my notes here so I don't forget what we did.

The equivalent EoT for the different simulation samples (and how its calculated) is shown below.

(This is output from running the `eot.py` script which scans all of the files in the sample.)

physics-8gev-smear with 20 runs
Sim EoT    S = 2.0e+07
N Events   N = 1.3e+05
Weight Sum W = 1.5e-01
Avg Bias B = N/W = 8.6e+05
EoT  = B * S = 1.7e+13

calib-8gev with 250 runs
Sim EoT    S = 8.6e+06
N Events   N = 2.5e+06
Weight Sum W = 1.5e+01
Avg Bias B = N/W = 1.7e+05
EoT  = B * S = 1.4e+12

We assume that (for one electron per bunch `mu=1`) we can collect 1e12 EoT per day.[^1]

[^1]: This assumes a duty cycle of about 33% https://numbat.dev/?q=37+MHz+*+1+day+*+0.33%E2%8F%8E

Then we can calculate the effective di-muon rate (= N Events / EoT)
and the number of di-muon events per "mu=1 day" (= Rate * 1e12)

Target  | Rate per EoT | Events per mu=1 Day
--------|--------------|--------------------
Physics | 7.6e-9       | 7600
Calib   | 1.7e-6       | 1.8e6

For the calibration target, we will run with a higher number of electrons in each bunch since
most electrons will just be swallowed by the thicker target.
We don't want to overwhelm our DAQ system which has a rate limit of ~ 25 kHz, so the maximum
number of electrons per bunch we would target is[^2]
```
calib mu = (25 kHz) / ((calib rate) * 37 MHz) = 380
```


[^2]: https://numbat.dev/?q=let+calib_rate+%3D+2.5e6%2F1.4e12%E2%8F%8E25+kHz+%2F+%28calib_rate+*+37+MHz%29%E2%8F%8E

