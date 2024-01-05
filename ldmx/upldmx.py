"""load LDMX event data into awkward arrays in memory"""

import uproot
import numpy as np
import awkward as ak

import vector
vector.register_awkward()

@ak.mixin_class(ak.behavior)
class WithEcalDetID:
    @property
    def layer(self):
        return (self.id >> 17) & 0x3f


@ak.mixin_class(ak.behavior)
class EcalRecHit(WithEcalDetID):
    """awkward behavior mixin adding functionality to EcalRecHit data"""
    pass


def as_rec_hit(events, branch):
    return ak.zip({
        m : events[f'{branch}.{m}_']
        for m in [
            'id', 'amplitude', 'energy', 'time',
            'xpos', 'ypos', 'zpos', 'isNoise'
        ]
    }, with_name = EcalRecHit.__name__)


@ak.mixin_class(ak.behavior)
class SimCalHit(WithEcalDetID):
    def has_contrib(self, pdg):
        return ak.any(self.contrib.pdgCode==pdg, axis=-1)
    def ncontribs(self):
        return ak.count(self.contib.pdgCode, axis=-1)

def as_sim_hit(events, branch):
    form = {
        m : events[f'{branch}.{m}_']
        for m in [
            'id', 'edep'
        ]
    }
    form.update({
        'pos' : ak.zip({
            c : events[f'{branch}.{c}_']
            for c in ['x','y','z','time']
        }, with_name='Vector4D'),
        'contrib' : ak.zip({
            m : events[f'{branch}.{m}Contribs_']
            for m in [
                'trackID', 'incidentID', 'pdgCode', 'edep', 'time'
            ]
        }, with_name='SimCalHitContrib')
    })
    return ak.zip(form, depth_limit = 2, with_name = SimCalHit.__name__)


@ak.mixin_class(ak.behavior)
class SimParticle:
    pass


def as_sim_particle(events, branch):
    form = {
        m : events[f'{branch}.second.{m}_']
        for m in [
            'pdgID', 'genStatus', 'mass', 'charge', 'daughters', 'parents',
            'processType', 'vertexVolume'
        ]
    }
    form.update({
        'track_id' : events[f'{branch}.first'],
        'momentum' : ak.zip({
            c : events[f'{branch}.second.{c}_']
            for c in ['px','py','pz','energy']
        }, with_name='Momentum4D'),
        'vertex' : ak.zip({
            c : events[f'{branch}.second.{c}_']
            for c in ['x','y','z','time']
        }, with_name='Vector4D'),
        'end_vertex' : ak.zip({
            c : events[f'{branch}.second.end{c.upper()}_']
            for c in ['x','y','z']
        }, with_name='Vector3D'),
    })
    return ak.zip(form, depth_limit=2, with_name = SimParticle.__name__)


def arrays(
    fp = None, *,
    include_header_params = False,
    branches = {
        'EcalRecHits_dimuon' : as_rec_hit,
        'EcalSimHits_dimuon' : as_sim_hit,
        'SimParticles_dimuon' : as_sim_particle
    },
    remove_pass = True
):
    """load various branches into awkward arrays in memory

        Parameters
    ----------
    fp : str, optional
        path to file(s) to load
        the default is to load all files that Tom generated and put in
        a specific directory in the shared group space
    include_header_params: bool, optional
        load the *Parameters from the EventHeader
        the code for doing this is kinda complicated so I want to keep it
        around but I don't think these are necessary for this study
    branches: dict, optional
        branch name to formatter dictionary
        default includes ecal rec hits, ecal sim hits, and sim particles
    remove_pass: bool, optional
        whether to remove passname from branch names upon formatting
        default is True (yes, do that)
    """
    
    if fp is None:
        fp = '/local/cms/user/eichl008/ldmx/dimuon-calibration/*.root'

    # load the data we want from the file into memory
    events = uproot.concatenate(
        fp+':LDMX_Events',
        filter_name = ['EventHeader**']+[br+'**' for br in branches]
    )

    # reformat the data into a more helpful form
    #   (shorter names and such)
    form = {
        name : events[branch]
        for name, branch in [
            ('number', 'eventNumber_'),
            ('run', 'run_'),
            ('weight', 'weight_')
        ]
    }
    if include_header_params:
        for param_type in ['float','int','string']:
            for key in np.unique(ak.flatten(events[f'{param_type}Parameters_.first']).to_numpy()):
                header_form[key] = ak.flatten(events[f'{param_type}Parameters_.second'][events[f'{param_type}Parameters_.first']==key])
    for branch, formatter in branches.items():
        name = branch
        if remove_pass:
            name = name[:name.index('_')]
        form[name]  = formatter(events, branch)
    return ak.zip(form, depth_limit=1)


def load_rec_hits(fp = None, **kwargs):
    """load ecal rec hits and event header into memory for analysis

    Parameters
    ----------
    fp : str, optional
        path to file(s) to load
        the default is to load all files that Tom generated and put in
        a specific directory in the shared group space
    kwargs: dict, optional
        key word arguments passed to arrays
    """
    return arrays(fp, branches = {'EcalRecHits_dimuon':as_rec_hit}, **kwargs)