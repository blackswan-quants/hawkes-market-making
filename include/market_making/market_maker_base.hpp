#pragma once

#include "common/types.hpp"
#include "market_making/quote.hpp"

namespace hawkes_market_making::market_making {

class MarketMakerBase {
public:
    virtual ~MarketMakerBase() = default;
    virtual Quote quote(Time time, Decimal mid_price, std::int64_t inventory) const = 0;
};

} // namespace hawkes_market_making::market_making
