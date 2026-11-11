#pragma once

#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::control {

Decimal linear_interpolate(Decimal x0, Decimal y0, Decimal x1, Decimal y1, Decimal x);

} // namespace hawkes_market_making::control
