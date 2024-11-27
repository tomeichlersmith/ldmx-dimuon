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
import uproot


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

    if 'physics-target' in fp:
        tot_sim_eot = 20*1e6
    else:
        tot_sim_eot = ak.sum(
            uproot.concatenate(
                {fp:'LDMX_Run'},
                filter_name = 'numTries_'
            ).numTries_
        )

    tot_eot = ak.count(events.weight)/ak.sum(events.weight)*tot_sim_eot
    per_day = 1e12/tot_eot

    hits['muon'] = hits.has_contrib(13)|hits.has_contrib(-13)
    hits['electron'] = hits.has_contrib(11)
    hits['other'] = ~(hits.muon|hits.electron)
    # energy of a MIP is ~0.13MeV, require hits to have at least 0.1 of a MIP
    hits['readout'] = (hits.edep > 0.1*0.13)

    h = (
        hist.Hist.new
        .IntCategory([], growth=True, name='cellid')
        .StrCategory(['electron','muon','other'],name='cause',label='Particle Causing Hit')
        .Double()
    ).fill(
        cellid = ak.flatten(hits[hits.muon&hits.readout].id),
        cause = 'muon'
    ).fill(
        cellid = ak.flatten(hits[hits.electron&hits.readout].id),
        cause = 'electron',
    ).fill(
        cellid = ak.flatten(
            hits[hits.other&hits.readout].id
        ),
        cause = 'other'
    )

    nhits = (
        hist.Hist.new
        .Integer(0,34,name='layer')
        .Reg(501,-0.5,500.5,name='nhits')
        .Double()
    )
    layer = (hits.id >> 17) & 0x3f
    for ilayer in range(34):
        nhits.fill(
            layer = ilayer,
            nhits = ak.sum(hits.muon&hits.readout&(layer==ilayer), axis=1)
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
        'nhits_h': nhits,
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
        ),
        mpl.patches.Rectangle(
            (-1047.75,-177.8),2*1047.75,2*177.8,
            facecolor='none',edgecolor='gray', linestyle=':',
            label='Magnet Opening'
        )
    ]
    for b in boxes:
        plt.gca().add_patch(b)


import math

def range_to_bins(vals, width):
    _min = np.min(vals)-width/2
    _max = np.max(vals)+width/2
    num = int(math.ceil((_max-_min)/width))
    return num, _min, _max


def plot(
    o,
    outdir,
    note
):    
    # convert the array of IDs stored in the histogram to the array of cell centers
    # this needs to be done for each histogram because the lookup depends on the _order_
    h = o['hist']
    cell_centers = o['to_cell_center'](h.axes[0])

    def hit_map(
        sl, filename, *,
        per_day = True,
        **legend_kw
    ):
        values = h[:,'muon'].values()
        if per_day:
            values *= o['per_day']
        zlabel = (
            'Count per Day'
            if per_day else
            'Count'
        )
#        art = (
#            hist.Hist.new
#            .Reg(*range_to_bins(cell_centers[0], 6.0), label='x / mm')
#            .Reg(*range_to_bins(cell_centers[1], 6.0), label='y / mm')
#            .Double()
#        ).fill(
#            cell_centers[0][sl],
#            cell_centers[1][sl],
#            weight = values[sl]
#        ).plot(
#            cmin=1,
#            #norm='log'
#        )
#        art.cbar.set_label(zlabel)
        plt.figure(figsize=(12,10))
        art = plt.scatter(
            cell_centers[0][sl],
            cell_centers[1][sl],
            c = values[sl],
            marker = 'H'
        )
        plt.xlabel('x / mm')
        plt.ylabel('y / mm')
        plt.colorbar(art, label=zlabel)
        title_bar(f'{o["tot_eot"]:.1e} EoT')
        draw_boxes()
        plt.legend(**legend_kw)
        plt.savefig(filename, bbox_inches='tight')
        plt.close()

    hit_map(
        slice(None), outdir / 'muon-hits.pdf',
        per_day = False,
        title = 'All Muon Hits (%s)'%(note),
        loc = 'lower center',
        bbox_to_anchor = (0.5,1.05)
    )

    cid = np.array(o['hist'].axes[0])
    layer = (cid >> 17) & 0x3f
    keep = (layer == 6) #|(layer == 7)
    hit_map(
        keep, outdir / 'muon-hits-layer-6.pdf',
        per_day = False,
        title = 'Layer 6 Muon Hits (%s)'%(note),
        loc='lower center',
        bbox_to_anchor=(0.5,1.05)
    )

    h[sum,:].plot(density=True)
    plt.ylabel('Hit Fraction')
    title_bar(f'{o["tot_eot"]:.1e} EoT')
    plt.annotate(
        note,
        xy=(0.95,0.95),
        xycoords='axes fraction',
        ha='right', va='top'
    )
    plt.savefig(outdir / 'hit-fraction-by-particle-cause.pdf')
    plt.close()

    o['nhits_h'][hist.loc(6),:].plot()
    plt.xlim(xmin=-0.5,xmax=10.5)
    plt.yscale('log')
    plt.ylabel('Event Count')
    plt.xlabel('N Hits in Layer 6')
    title_bar(f'{o["tot_eot"]:.1e} EoT')
    plt.annotate(
        note,
        xy=(0.95,0.95),
        xycoords='axes fraction',
        ha='right', va='top'
    )
    plt.savefig(outdir / 'n-hits-per-event-layer-6.pdf')
    plt.close()



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
        o = fill(samples[args.sample]['filepath'])
        with open(args.hist, 'wb') as f:
            pickle.dump(o, f)

    plot(o, args.hist.parent, samples[args.sample]['note'])


if __name__ == '__main__':
    main()
