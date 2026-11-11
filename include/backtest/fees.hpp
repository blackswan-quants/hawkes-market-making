#pragma once

#include "common/types.hpp"

namespace hawkes_market_making::backtest {

struct FeeSchedule {
    Decimal taker_fee = 0.0001;
    Decimal maker_rebate = 0.00002;
    Decimal market_impact = 0.0;
};

} // namespace hawkes_market_making::backtest
