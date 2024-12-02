from pathlib import Path
import pickle
import argparse

import matplotlib as mpl
import mplhep
mpl.style.use(mplhep.style.ROOT)
import matplotlib.pyplot as plt
import hist
import upldmx
import awkward as ak
import numpy as np
import uproot


def title_bar(
    text=None, *,
    by_ldmx='Internal',
    exp_text_kw = dict(),
    lumitext_kw = dict(),
    **kwargs
):
    mplhep.label.exp_text('LDMX', by_ldmx, **exp_text_kw, **kwargs)
    if text is not None:
        mplhep.label.lumitext(text, **lumitext_kw, **kwargs)


def draw_boxes():
    boxes = [
        mpl.patches.Rectangle(
            (-25,-40),45,80,
            facecolor='none',edgecolor='tab:red',linestyle='--',
            label='$\\sim10^{12}$ cm$^{-2}$ 1MeV n Fluence'
        ),
        mpl.patches.Rectangle(
            (-10,-40),20,80,
            facecolor='none',edgecolor='tab:red',linestyle='-',
            label='Target'
        ),
        mpl.patches.Rectangle(
            (-1047.75,-177.8),2*1047.75,2*177.8,
            facecolor='none',edgecolor='gray', linestyle=':',
            label='Magnet Opening'
        )
    ]
    for b in boxes:
        plt.gca().add_patch(b)


base = Path('/local/cms/user/eichl008/ldmx/dimuon-calibration')


samples = {
    'physics-8gev-smear' : {
        'filepath': str(base / 'physics-target' / 'with-beam-smear' / '*.root'),
        'note' : 'Physics Target; 8GeV'
    },
    'physics-8gev-point' : {
        'filepath': str(base / 'physics-target' / 'no-smear' / '*.root'),
        'note': 'Physics Target; 8GeV; Point'
    },
    'calib-8gev' : {
        'filepath': str(base / 'calib-target' / '*.root'),
        'note': 'Calib Target; 8GeV'
    },
}


class Analysis:

    @classmethod
    def base_parser(cls):
        parser = argparse.ArgumentParser()
        parser.add_argument(
            'output', type=Path,
            help='output directory to store histograms and plots'
        )
        parser.add_argument(
            '--sample', type=str,
            help='name of sample to analyze'
        )
        parser.add_argument(
            '--replot', action='store_true',
            help='just re-plot do not re-fill histograms to save time'
        )
        return parser


    def __init__(self, args = None):
        if args is None:
            args = self.__class__.base_parser().parse_args()

        self.__dict__ = vars(args)
        self.filepath = samples[self.sample]['filepath']
        self.note = samples[self.sample]['note']

        if self.relot:
            with open(self.output / 'hist.pkl', 'rb') as f:
                o = pickle.load(f)
        else:
            o = self.fill()
            o['sample'] = self.sample
            with open(self.output / 'hist.pkl', 'wb') as f:
                pickle.dump(o, f)

        self.output.mkdir(exist_ok=True, parents=True)
        self.plot(o)

    
    def fill(self):
        raise NotImplemented


    def plot(self, o):
        raise NotImplemented
