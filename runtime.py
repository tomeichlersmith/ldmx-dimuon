"""estimate run time of a sample"""

from datetime import timedelta

from position import samples
import uproot
import awkward as ak

def pretty_time(seconds):
    td = timedelta(seconds = int(seconds))
    return str(td)

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('sample', help='sample to analyze', choices=list(samples.keys()))
    args = parser.parse_args()

    timestamps = uproot.concatenate(
        {samples[args.sample]['filepath']:'LDMX_Run'},
        expressions = ['RunHeader/runStart_','RunHeader/runEnd_']
    )

    runtime = timestamps['RunHeader/runEnd_']-timestamps['RunHeader/runStart_']

    mean = ak.mean(runtime)
    stdd = ak.std(runtime)

    print(samples[args.sample]['note'])
    print(f'Run Time: {mean:.1f} +/- {stdd:.1f} s')
    print(f'HH:MM:SS: {pretty_time(mean)} +/- {pretty_time(stdd)}')


if __name__ == '__main__':
    main()
