#pragma once

#include "common/constants.hpp"
#include "market_making/market_maker_base.hpp"

namespace hawkes_market_making::market_making
{

    // Model parameters, with the symbols of Avellaneda & Stoikov (2008) in the comments.
    // Volatility and horizon must use the same time unit as the `time` passed to quote().
    struct AvellanedaStoikovParameters
    {
        Decimal risk_aversion = kAsDefaultRiskAversion; // gamma (0 = risk neutral)
        Decimal volatility = kAsDefaultVolatility;      // sigma, mid follows dS = sigma dW
        Decimal arrival_scale = kAsDefaultArrivalScale; // A in lambda(delta) = A exp(-kappa delta)
        Decimal arrival_decay = kAsDefaultArrivalDecay; // kappa
        Time horizon = kAsDefaultHorizon;               // T
    };

    // Finite-horizon Avellaneda-Stoikov market maker, closed-form solution (eq. 29-30).
    // Quotes are placed symmetrically around a reservation price that depends on inventory.
    class AvellanedaStoikov : public MarketMakerBase
    {
      public:
        // Default parameters are the ones of the simulation in the paper.
        AvellanedaStoikov() = default;

        // Throws std::invalid_argument if the parameters are not valid.
        explicit AvellanedaStoikov(const AvellanedaStoikovParameters& parameters);

        // Bid and ask for one unit each. Inventory is signed: positive means long.
        Quote quote(Time time, Decimal mid_price, std::int64_t inventory) const override;

        // r = s - q * gamma * sigma^2 * (T - t)
        Decimal reservation_price(Time time, Decimal mid_price, std::int64_t inventory) const;

        // gamma * sigma^2 * (T - t) + (2 / gamma) * ln(1 + gamma / kappa)
        Decimal optimal_spread(Time time) const;

        // A * exp(-kappa * depth): fill rate of a quote `depth` away from the mid.
        // Not used by quote(), it will be needed by the backtest and the calibration.
        Decimal fill_intensity(Decimal depth) const;

        const AvellanedaStoikovParameters& parameters() const { return parameters_; }

      private:
        // T - t, never negative.
        Decimal time_to_horizon(Time time) const;

        AvellanedaStoikovParameters parameters_{};
    };

} // namespace hawkes_market_making::market_making
