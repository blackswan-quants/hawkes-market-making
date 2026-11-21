#include "market_making/avellaneda_stoikov.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace hawkes_market_making::market_making
{

    namespace
    {

        // In the paper the dealer quotes one share per side.
        constexpr Decimal kUnitQuoteSize = 1.0;

        // Checked once here so that quote() does not have to check anything.
        void validate(const AvellanedaStoikovParameters& p)
        {
            const bool all_finite = std::isfinite(p.risk_aversion) && std::isfinite(p.volatility) &&
                                    std::isfinite(p.arrival_scale) &&
                                    std::isfinite(p.arrival_decay) && std::isfinite(p.horizon);
            if (!all_finite)
            {
                throw std::invalid_argument("AvellanedaStoikov: parameters must be finite");
            }
            // gamma = 0 is accepted, it is the risk-neutral case.
            if (p.risk_aversion < 0.0)
            {
                throw std::invalid_argument(
                    "AvellanedaStoikov: risk aversion must be non-negative");
            }
            if (p.volatility < 0.0)
            {
                throw std::invalid_argument("AvellanedaStoikov: volatility must be non-negative");
            }
            if (p.arrival_scale <= 0.0)
            {
                throw std::invalid_argument("AvellanedaStoikov: arrival scale must be positive");
            }
            if (p.arrival_decay <= 0.0)
            {
                throw std::invalid_argument("AvellanedaStoikov: arrival decay must be positive");
            }
            if (p.horizon <= 0.0)
            {
                throw std::invalid_argument("AvellanedaStoikov: horizon must be positive");
            }
        }

    } // namespace

    AvellanedaStoikov::AvellanedaStoikov(const AvellanedaStoikovParameters& parameters)
        : parameters_(parameters)
    {
        validate(parameters_);
    }

    Decimal AvellanedaStoikov::time_to_horizon(Time time) const
    {
        // After T we behave as if we were at T, otherwise the inventory skew would change sign.
        return std::max(parameters_.horizon - time, 0.0);
    }

    Decimal AvellanedaStoikov::reservation_price(Time time, Decimal mid_price,
                                                 std::int64_t inventory) const
    {
        const Decimal gamma = parameters_.risk_aversion;
        const Decimal variance = parameters_.volatility * parameters_.volatility;
        const Decimal q = static_cast<Decimal>(inventory);

        // Long inventory lowers the reservation price, short inventory raises it.
        return mid_price - q * gamma * variance * time_to_horizon(time);
    }

    Decimal AvellanedaStoikov::optimal_spread(Time time) const
    {
        const Decimal gamma = parameters_.risk_aversion;
        const Decimal kappa = parameters_.arrival_decay;
        const Decimal variance = parameters_.volatility * parameters_.volatility;

        // Part of the spread that pays for the inventory risk until T.
        const Decimal inventory_risk_term = gamma * variance * time_to_horizon(time);

        // Part that depends only on the liquidity of the book. For gamma -> 0 it tends to
        // 2 / kappa; the formula itself would give 0 / 0, so that case is handled apart.
        Decimal liquidity_term = 2.0 / kappa;
        if (gamma > kEpsilon)
        {
            // log1p(x) = log(1 + x), more accurate when x is small.
            liquidity_term = (2.0 / gamma) * std::log1p(gamma / kappa);
        }

        return inventory_risk_term + liquidity_term;
    }

    Decimal AvellanedaStoikov::fill_intensity(Decimal depth) const
    {
        return parameters_.arrival_scale * std::exp(-parameters_.arrival_decay * depth);
    }

    Quote AvellanedaStoikov::quote(Time time, Decimal mid_price, std::int64_t inventory) const
    {
        const Decimal reservation = reservation_price(time, mid_price, inventory);
        const Decimal half_spread = 0.5 * optimal_spread(time);

        // Symmetric around the reservation price, not around the mid.
        Quote result;
        result.bid = reservation - half_spread;
        result.ask = reservation + half_spread;
        result.size_bid = kUnitQuoteSize;
        result.size_ask = kUnitQuoteSize;
        result.timestamp = time;
        return result;
    }

} // namespace hawkes_market_making::market_making
