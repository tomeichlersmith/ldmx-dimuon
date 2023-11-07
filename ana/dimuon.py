"""small module for formatting the in-memory awkward array into a helpful form

Examples
========

    import dimuon
    events = dimuon.loadup(fp)

"""

import uproot
import awkward as ak
import numpy as np
import vector
# register vector with awkward so we can just use special names
# to get the lorentz behavior we want
vector.register_awkward()


__version__ = '0.1.1'


def _create_subbranch(tree, name, single = True):
    """Create a local Callable object which can be used to access subbranches
    of the input branch 'name'

    Parameters
    ----------
    tree : uproot's TTree wrapper
        tree we are reading from
    name : str
        branch we want to get subbranches for
    single: bool, optional
        if this branch is a single instance of an object per event or a collection
    

    Returns
    -------
    Callable
        a function that can be called with a member name to retrieve an array
        of that member
    """
    if single:
        def _subbranch(member_name):
            return tree[f'{name}/{member_name}'].array()
        return _subbranch

    def _subbranch(member_name):
        return tree[f'{name}/{name}.{member_name}'].array()
    return _subbranch


def _particle(subbranch):
    """get a particle formatting using the input subbranch function

    Parameter
    ---------
    subbranch: Callable
        function we can use to get the members of a Particle class
    """
    d = {
        member : subbranch(member)
        for member in [
            'valid', 'track_id', 'pdg_id'
        ]
    }
    d.update({
        'momentum' : ak.zip({
            c : subbranch(c)
            for c in ['px','py','pz','energy']
        }, with_name = 'Momentum4D'),
        'position' : ak.zip({
            c : subbranch(c)
            for c in ['x','y','z','t']
        }, with_name = 'Vector4D'),
    })
    return ak.zip(d, with_name='Particle')


def loadup(fp):
    """Main loading function for dimuon analysis

    We put together the various uproot branches into the
    awkward form that we then ak.zip together into a final
    events awkward array which we return.

    Parameters
    ----------
    fp : str, pathlib.Path
        file path to file we want to open and read
    """

    with uproot.open(fp) as f:
        event_tree = f['events']
        d = {
            name : _particle(_create_subbranch(event_tree, name))
            for name in [
                'incident', 'parent', 'mu_plus', 'mu_minus'
            ]
        }
        d.update({
            'weight' : event_tree['weight'].array(),
            'extra' : _particle(_create_subbranch(event_tree, 'extra', single=False)),
            'ecal' : _particle(_create_subbranch(event_tree, 'ecal', single=False))
        })
        events = ak.zip(d, depth_limit=1)
        run_params = f['run'].members
        run_params['eot'] = ak.count(events.weight)/ak.sum(events.weight)*run_params['tries_']
        # divide depth by tungsten radiation length to get a nice
        # labeling number for the sample
        run_params['depth_x0'] = round(run_params['depth_']/3.50259,1)
        return run_params, events
