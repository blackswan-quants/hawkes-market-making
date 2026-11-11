#pragma once

#include "market_making/market_maker_base.hpp"

namespace hawkes_market_making::market_making {

class HawkesMarketMaker : public MarketMakerBase {
public:
    HawkesMarketMaker() = default;
    HawkesMarketMaker(Decimal risk_aversion, Decimal spread_penalty, Decimal inventory_cap, Decimal excitation_strength);

    Quote quote(Time time, Decimal mid_price, std::int64_t inventory) const override;

    Decimal excitation_strength() const { return excitation_strength_; }

private:
    Decimal risk_aversion_ = 1.0;
    Decimal spread_penalty_ = 1.0;
    Decimal inventory_cap_ = 100.0;
    Decimal excitation_strength_ = 0.0;
};

} // namespace hawkes_market_making::market_making
