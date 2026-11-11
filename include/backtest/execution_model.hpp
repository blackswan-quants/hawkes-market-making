#pragma once

#include "common/types.hpp"

namespace hawkes_market_making::backtest {

struct ExecutionModel {
    Decimal fill_probability = 0.95;
    Decimal fee_per_unit = 0.0001;
    Decimal rebate_per_unit = 0.0;
};

} // namespace hawkes_market_making::backtest
