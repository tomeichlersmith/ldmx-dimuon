#include "RunHeader.h"

ClassImp(RunHeader);

RunHeader::RunHeader(
    int tries,
    std::optional<double> filter_threshold,
    std::optional<double> bias_factor
)
  : tries_{tries},
    filter_{filter_threshold.has_value()},
    filter_threshold_{filter_threshold.value_or(0.)},
    bias_factor_{bias_factor.value_or(1.)}
{}
