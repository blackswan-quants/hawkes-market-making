#pragma once

#include "control/state.hpp"
#include "control/value_function.hpp"

namespace hawkes_market_making::control {

class HJBProcessor {
public:
    HJBProcessor();
    ValueFunction solve(Time horizon, Decimal dt, std::size_t grid_size) const;
};

} // namespace hawkes_market_making::control
