from pathlib import Path
import pickle
import matplotlib as mpl
import mplhep
mpl.style.use(mplhep.style.ROOT)
import matplotlib.pyplot as plt
import hist
import upldmx
import awkward as ak
import numpy as np


def title_bar(text=None, *, by_ldmx='Internal', exp_text_kw = dict(), lumitext_kw = dict(), **kwargs):
    mplhep.label.exp_text('LDMX', by_ldmx, **exp_text_kw, **kwargs)
    if text is not None:
        mplhep.label.lumitext(text, **lumitext_kw, **kwargs)


def fill(
    fp, *,
    hit_sl = None,
):
    events = upldmx.arrays(fp)
    hits = events.EcalSimHits
    if hit_sl is not None:
        hits = hits[hit_sl(hits)]

    tot_eot = ak.count(events.weight)/ak.sum(events.weight)*20*1e6
    per_day = 1e12/tot_eot

    h = (
        hist.Hist.new
        .IntCategory([], growth=True, name='cellid')
        .StrCategory(['electron','muon','other'],name='cause',label='Particle Causing Hit')
        .Double()
    ).fill(
        cellid = ak.flatten(hits[hits.has_contrib(13)|hits.has_contrib(-13)].id),
        cause = 'muon'
    ).fill(
        cellid = ak.flatten(hits[hits.has_contrib(11)].id),
        cause = 'electron',
    ).fill(
        cellid = ak.flatten(
            hits[~(hits.has_contrib(13)|hits.has_contrib(-13)|hits.has_contrib(11))].id
        ),
        cause = 'other'
    )

    # define a lookup table (LUT) for hit IDs to transverse cell centers
    # this only needs to be redefined if the `hits` array changes because there might
    # be an ID in the new selection that wasn't in the old
    sorted_by_id = ak.flatten(hits)[ak.argsort(ak.flatten(hits).id)]
    id_to_pos_lut = {
        hit.id : (hit.pos.x, hit.pos.y)
        for hit in ak.drop_none(ak.firsts(ak.unflatten(sorted_by_id, ak.run_lengths(sorted_by_id.id))))
    }

    return {
        'hist': h,
        'to_cell_center': np.vectorize(id_to_pos_lut.get),
        'tot_eot': tot_eot,
        'per_day': per_day
    }


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
        )
    ]
    for b in boxes:
        plt.gca().add_patch(b)

def hit_map(
    cell_centers, counts, filename, tot_eot, *,
    title = None
):
    plt.figure(figsize=(12,10))
    plt.scatter(
        cell_centers[0], cell_centers[1],
        c = counts,
        marker='H',
        norm='log',
        # vmax=1000
    )
    plt.colorbar(label='Count per Day')
    plt.xlabel('X [mm]')
    plt.ylabel('Y [mm]')
    # plt.text(-260,255,'Highest Energy Hit Caused by Muons',ha='left',va='top',size='small')
    title_bar(f'{tot_eot:.1e} EoT')
    draw_boxes()
    plt.legend(title=title, frameon=True)
    plt.savefig(filename, bbox_inches='tight')
    plt.close()

def plot(
    o,
    outdir
):    
    # convert the array of IDs stored in the histogram to the array of cell centers
    # this needs to be done for each histogram because the lookup depends on the _order_
    cell_centers = o['to_cell_center'](o['hist'].axes[0])

    hit_map(
        cell_centers,
        o['hist'][:,'muon'].values()*o['per_day'],
        outdir / 'muon-hits.pdf',
        o['tot_eot']
    )

    o['hist'][sum,:].plot(density=True)
    plt.ylabel('Hit Fraction')
    title_bar(f'{o["tot_eot"]:.1e} EoT')
    plt.savefig(outdir / 'hit-fraction-by-particle-cause.pdf')
    plt.close()


base = Path('/local/cms/user/eichl008/ldmx/dimuon-calibration')

samples = {
    'physics-8gev-smear' : str(base / 'physics-target' / 'with-beam-smear' / '*.root'),
    'physics-8gev-point' : str(base / 'physics-target' / 'no-smear' / '*.root'),
    'calib-8gev' : str(base / 'calib-target' / '*.root'),
}

def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument('hist', type=Path, help='path to pickle of histogram info')
    parser.add_argument('sample', help='sample to analyze', choices=list(samples.keys()))
    parser.add_argument('--replot', help='just re-plot do not refill', action='store_true')
    args = parser.parse_args()

    args.hist.parent.mkdir(exist_ok=True, parents=True)

    if args.replot:
        with open(args.hist, 'rb') as f:
            o = pickle.load(f)
    else:
        o = fill(samples[args.sample])
        with open(args.hist, 'wb') as f:
            pickle.dump(o, f)

    plot(o, args.hist.parent)


if __name__ == '__main__':
    main()
