#pragma once

#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::control {

struct ValueFunction {
    std::vector<Decimal> values;
    std::size_t nx = 0;
    std::size_t ny = 0;
    std::size_t nq = 0;
};

} // namespace hawkes_market_making::control
