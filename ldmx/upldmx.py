"""load LDMX event data into awkward arrays in memory"""

import uproot
import numpy as np
import awkward as ak

@ak.mixin_class(ak.behavior)
class EcalRecHit:
    """awkward behavior mixin adding functionality to EcalRecHit data"""
    def layer(self):
        """deduce the layer a hit is in from its detector ID"""
        return (self.id >> 17) & 0x3f

def load_rec_hits(
    fp = None, *,
    include_header_params = False,
):
    """load ecal rec hits and event header into memory for analysis

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
    """
    
    if fp is None:
        fp = '/local/cms/user/eichl008/ldmx/dimuon-calibration/*.root'

    # load the data we want from the file into memory
    events = uproot.concatenate(
        fp+':LDMX_Events',
        filter_name = [ 'EventHeader**', 'EcalRecHits**' ]
    )

    # reformat the data into a more helpful form
    #  (basically just making shorter names and attaching helper functions)
    rec_hit_form = {
        m : events[f'EcalRecHits_dimuon.{m}_']
        for m in [
            'id', 'amplitude', 'energy', 'time', 'xpos','ypos','zpos','isNoise'
        ]
    }
    header_form = {
        name : events[branch]
        for name, branch in [
            ('number', 'eventNumber_'),
            ('run', 'run_'),
            ('weight', 'weight_')
        ]
    }
    header_form['EcalRecHits'] = ak.zip(rec_hit_form, with_name=EcalRecHit.__name__)
    if include_header_params:
        for param_type in ['float','int','string']:
            for key in np.unique(ak.flatten(events[f'{param_type}Parameters_.first']).to_numpy()):
                header_form[key] = ak.flatten(events[f'{param_type}Parameters_.second'][events[f'{param_type}Parameters_.first']==key])
    return ak.zip(header_form, depth_limit=1)