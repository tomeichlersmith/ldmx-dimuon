from types import SimpleNamespace
from dataclasses import dataclass, field

import uproot
import awkward as ak

from analysis import samples


@dataclass
class EoT:
    """Calculate equivalent EoT and hold results

    This is a helper class so that other analyses can calculate
    the EoT in the same way and get the results. This class
    also holds all of the ingredients for the EoT as well as
    the number of runs there were.
    """

    nruns: int
    nevents: int
    weight_sum: float
    tot_sim_eot: int
    avg_bias: float = field(default=None, init=False)
    eot: float = field(default=None, init=False)


    def __post_init__(self):
        """Calculate the average bias factor and the EoT estimate
        from the other ingredients"""

        self.avg_bias = self.nevents / self.weight_sum
        self.eot = self.avg_bias*self.tot_sim_eot


    def __str__(self):
        """Override str operator to just print the EoT estimate in scientific notation
        
        The original dataclass print method is still available via the repr operator.

            print(repr(eot_obj))

        gives the full details.
        """
        return f'{self.eot:.1e}'


    @classmethod
    def from_sample(cls, sample, weights = None):
        """Estimate the EoT from the sample name using the set of samples to get the filepath

            EoT.from_sample("sample_name")
        
        This can take some time since we have to access the files in order to get the weightsum,
        number of events, number of runs, and total number of tries. If the weights are already
        loaded (for example, since the events were loaded as part of another analysis), then
        you can provide the weights to avoid re-accessing the files for that information.

            EoT.from_sample("sample_name", weights = events.weight)

        """

        fp = samples[sample]['filepath']
        
        if weights is None:
            weights = uproot.concatenate(
                { fp : 'LDMX_Events' },
                expressions = [ 'EventHeader/weight_' ]
            )['EventHeader/weight_']

        if 'physics-target' in fp:
            # older physics target doesn't have numTries stored in RunHeader
            tries = ak.Array([1e6]*20)
        else:
            tries = uproot.concatenate(
                { fp : 'LDMX_Run' },
                expressions = [ 'RunHeader/numTries_' ]
            )['RunHeader/numTries_']

        return cls(
            nruns = ak.count(tries),
            nevents = ak.count(weights),
            weight_sum = ak.sum(weights),
            tot_sim_eot = ak.sum(tries),
        )

    

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'sample',
        choices = list(samples.keys()),
        help='sample to estimate EoT for'
    )
    args = parser.parse_args()

    r = EoT.from_sample(args.sample)

    print(f'{args.sample} with {r.nruns} runs')
    print(f'Sim EoT    S = {r.tot_sim_eot:.1e}')
    print(f'N Events   N = {r.nevents:.1e}')
    print(f'Weight Sum W = {r.weight_sum:.1e}')
    print(f'Avg Bias B = N/W = {r.avg_bias:.2g}')
    print(f'EoT  = B * S = {r.eot:.1e}')


if __name__ == '__main__':
    main()
