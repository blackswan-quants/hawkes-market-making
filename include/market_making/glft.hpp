#pragma once

#include "market_making/market_maker_base.hpp"

namespace hawkes_market_making::market_making {

class GLFT : public MarketMakerBase {
public:
    GLFT() = default;
    GLFT(Decimal risk_aversion, Decimal spread_penalty, Decimal inventory_cap);

    Quote quote(Time time, Decimal mid_price, std::int64_t inventory) const override;

private:
    Decimal risk_aversion_ = 1.0;
    Decimal spread_penalty_ = 1.0;
    Decimal inventory_cap_ = 100.0;
};

} // namespace hawkes_market_making::market_making
