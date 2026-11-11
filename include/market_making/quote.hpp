#pragma once

#include "common/types.hpp"

namespace hawkes_market_making::market_making {

struct Quote {
    Decimal bid = 0.0;
    Decimal ask = 0.0;
    Decimal size_bid = 0.0;
    Decimal size_ask = 0.0;
    Time timestamp = 0.0;
};

} // namespace hawkes_market_making::market_making
