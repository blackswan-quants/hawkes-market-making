#pragma once

#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::hawkes {

struct HawkesParameters {
    Decimal baseline_buy = 1.0;
    Decimal baseline_sell = 1.0;
    Decimal alpha_buy_buy = 0.1;
    Decimal alpha_buy_sell = 0.1;
    Decimal alpha_sell_buy = 0.1;
    Decimal alpha_sell_sell = 0.1;
    Decimal beta_buy = 1.0;
    Decimal beta_sell = 1.0;
};

class HawkesProcess {
public:
    explicit HawkesProcess(HawkesParameters parameters = {});

    Decimal intensity_buy(Time time, Decimal excitation_buy, Decimal excitation_sell) const;
    Decimal intensity_sell(Time time, Decimal excitation_buy, Decimal excitation_sell) const;
    Decimal spectral_radius() const;

private:
    HawkesParameters parameters_;
};

} // namespace hawkes_market_making::hawkes
