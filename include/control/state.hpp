#pragma once

#include "common/types.hpp"

namespace hawkes_market_making::control {

struct State {
    std::int64_t inventory = 0;
    Decimal excitation_buy = 0.0;
    Decimal excitation_sell = 0.0;
};

} // namespace hawkes_market_making::control
