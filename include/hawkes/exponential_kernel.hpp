#pragma once

#include "common/types.hpp"

namespace hawkes_market_making::hawkes {

struct ExponentialKernel {
    Decimal alpha = 0.1;
    Decimal beta = 1.0;

    Decimal evaluate(Decimal lag) const {
        return alpha * std::exp(-beta * lag);
    }
};

} // namespace hawkes_market_making::hawkes
