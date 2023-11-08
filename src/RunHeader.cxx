#include "RunHeader.h"

#include "Version.h"

ClassImp(RunHeader);

RunHeader::RunHeader(
    int tries,
    std::optional<double> filter_threshold,
    std::optional<double> bias_factor,
    const std::string& target,
    double depth,
    double beam,
    bool photons
)
  : tries_{tries},
    filter_{filter_threshold.has_value()},
    filter_threshold_{filter_threshold.value_or(0.)},
    bias_factor_{bias_factor.value_or(1.)},
    target_{target},
    depth_{depth},
    beam_{beam},
    photons_{photons},
    version_major_{version::MAJOR},
    version_minor_{version::MINOR},
    version_patch_{version::PATCH}
{}
