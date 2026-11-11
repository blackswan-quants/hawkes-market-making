#pragma once

#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::backtest {

struct PnLMetrics {
    Decimal total_pnl = 0.0;
    Decimal adverse_selection = 0.0;
    Decimal realized = 0.0;
    Decimal fees = 0.0;
};

PnLMetrics compute_pnl(const std::vector<Decimal>& pnl_series);

} // namespace hawkes_market_making::backtest
