import sys
import argparse

parser = argparse.ArgumentParser(
    f'ldmx fire {sys.argv[0]}',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter
)

parser.add_argument(
    'run_number', default=1,
    help='set the random seed for the simulation',
    type=int
)

arg = parser.parse_args()

from LDMX.Framework import ldmxcfg

p = ldmxcfg.Process('dimuon')
p.maxTriesPerEvent = 1
p.maxEvents = 1000000
p.termLogLevel = 1
p.logFrequency = p.maxEvents // 100
p.run = arg.run_number

from LDMX.SimCore import simulator
from LDMX.SimCore import generators
from LDMX.SimCore import bias_operators
from LDMX.Biasing import filters
from LDMX.Biasing import util
sim = simulator.simulator('target_dimuon')
sim.setDetector('ldmx-det-v14-8gev', True)
sim.description = 'gamma conversion to muons in the target'
sim.beamSpotSmear = [20., 80., 0.]
sim.generators = [ generators.single_8gev_e_upstream_tagger() ]

beam = sim.generators[0].energy*1000
tagger = 0.95*beam
minbrem = 0.625*beam

sim.biasing_operators = [ bias_operators.GammaToMuPair('target', 1e6, minbrem) ]
sim.actions = [
    filters.TaggerVetoFilter(thresh = tagger),
    filters.TargetBremFilter(recoil_max_p = beam - minbrem, brem_min_e = minbrem),
    filters.TargetGammaMuMuFilter(),
    util.TrackProcessFilter.gamma_mumu()
]

params = [
    'beam', '8gev',
    'target', 'physics',
    'tagger', str(int(tagger)),
    'minbrem', str(int(minbrem)),
    'run', f'{p.run:04d}'
]
p.outputFiles = [
  '_'.join(params)+'.root'
]

import LDMX.Ecal.EcalGeometry
import LDMX.Hcal.HcalGeometry
import LDMX.Ecal.ecal_hardcoded_conditions
import LDMX.Ecal.digi as ecal_digi

p.sequence = [
    sim,
    ecal_digi.EcalDigiProducer(),
    ecal_digi.EcalRecProducer()
    ]
