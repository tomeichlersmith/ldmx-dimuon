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
    
    t = uproot.open(f'{fp}:dimuon_events')
    
    def _create_subbranch(name, single):
        if single:
            def _subbranch(b):
                return t[f'{name}/{b}'].array()
            return _subbranch
    
        def _subbranch(b):
            return t[f'{name}/{name}.{b}'].array()
        return _subbranch
    
    
    def particle(name, single = True):
        subbranch = _create_subbranch(name, single)
        d = {
            b : subbranch(b)
            for b in [
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
    
    d = {
        name : particle(name)
        for name in [
            'incident', 'parent', 'mu_plus', 'mu_minus'
        ]
    }
    d.update({
        'ntries' : t['ntries'].array(),
        'weight' : t['weight'].array(),
        'extra' : particle('extra', single=False),
        'ecal' : particle('ecal', single=False)
    })
    return ak.zip(d, depth_limit=1)