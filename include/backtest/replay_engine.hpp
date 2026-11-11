#pragma once

#include <vector>

#include "common/types.hpp"
#include "market_making/market_maker_base.hpp"

namespace hawkes_market_making::backtest {

class ReplayEngine {
public:
    ReplayEngine(const market_making::MarketMakerBase& strategy);

    Decimal run(const std::vector<Event>& events, Decimal starting_mid_price) const;

private:
    const market_making::MarketMakerBase& strategy_;
};

} // namespace hawkes_market_making::backtest
