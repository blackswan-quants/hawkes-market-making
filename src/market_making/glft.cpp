#include "market_making/glft.hpp"

#include <cmath>

namespace hawkes_market_making::market_making
{
    GLFT::GLFT(GLFTParameters parameters) : parameters_(parameters)
    {
        const Decimal gamma = parameters_.risk_aversion;
        const Decimal kappa = parameters_.fill_rate_decay;
        const Decimal arrival_rate = parameters_.fill_rate_scale;
        const Decimal sigma = parameters_.volatility;

        // Gamma and sigma may be zero, which removes the inventory adjustment.
        // The liquidity parameters must be positive because they enter denominators.
        if (!std::isfinite(gamma) || gamma < 0.0)
        {
            throw GLFTParameterError("GLFT risk_aversion must be finite and nonnegative");
        }
        if (!std::isfinite(kappa) || kappa <= 0.0)
        {
            throw GLFTParameterError("GLFT fill_rate_decay must be finite and positive");
        }
        if (!std::isfinite(arrival_rate) || arrival_rate <= 0.0)
        {
            throw GLFTParameterError("GLFT fill_rate_scale must be finite and positive");
        }
        if (!std::isfinite(sigma) || sigma < 0.0)
        {
            throw GLFTParameterError("GLFT volatility must be finite and nonnegative");
        }
        if (parameters_.inventory_cap <= 0)
        {
            throw GLFTParameterError("GLFT inventory_cap must be positive");
        }

        // Section 4 of Guéant, Lehalle and Fernandez-Tapia gives the long-horizon
        // approximation. Set x = gamma / kappa. log1p preserves log(1 + x) for
        // small x, and log(gamma) - log(kappa) handles ratios that overflow:
        // at that magnitude, log(1 + x) and log(x) round to the same value.
        const Decimal ratio = gamma / kappa;
        const Decimal log_one_plus_ratio =
            std::isfinite(ratio) ? std::log1p(ratio) : std::log(gamma) - std::log(kappa);

        // log(1 + x) / x tends to 1 as x tends to zero. Use the limit when
        // gamma is zero or the division underflows. For small x, computing
        // c1 as this factor divided by kappa avoids losing precision in the
        // intermediate ratio. For larger x, use c1 = log(1 + x) / gamma.
        const Decimal scaled_log = ratio == 0.0 ? 1.0 : log_one_plus_ratio / ratio;
        c1_ = ratio <= 1.0 ? scaled_log / kappa : log_one_plus_ratio / gamma;
        
        if (gamma > 0.0 && sigma > 0.0)
        {
            // c2 = sigma * sqrt(gamma / (2 * kappa * A) * (1 + x)^(1 + 1/x)).
            // In log space the power contributes log(1 + x) + scaled_log.
            // This avoids forming sigma squared, the product in the denominator,
            // or 1/x, each of which can exceed the numeric range on its own.
            const Decimal log_inventory_factor =
                std::log(gamma) - std::log(2.0) - std::log(kappa) - std::log(arrival_rate)
                + log_one_plus_ratio + scaled_log;
            c2_ = std::exp(std::log(sigma) + 0.5 * log_inventory_factor);
        }

        // The continuous gamma = 0 limit gives c1 = 1 / kappa and c2 = 0.
        // With sigma = 0, c2 is also zero. Reject any coefficient overflow
        // before the strategy can be used to generate quotes.
        if (!std::isfinite(c1_) || !std::isfinite(c2_))
        {
            throw GLFTParameterError("GLFT parameters produce non-finite quote coefficients");
        }
    }

    Quote GLFT::quote(Time time, Decimal mid_price, Quantity inventory) const
    {
        // The constructor requires a positive cap, so both symmetric limits
        // are representable as Quantity and can be compared exactly.
        const Quantity max_position = parameters_.inventory_cap;
        const Quantity min_position = -max_position;
        if (inventory < min_position || inventory > max_position)
        {
            throw GLFTQuoteError("GLFT quote inventory is outside the configured cap");
        }

        // Evaluate the two signed depths from Section 4. A fused multiply-add
        // rounds each product and sum together. The offsets are in the same
        // price unit as the supplied mid-price.
        const Decimal position = static_cast<Decimal>(inventory);
        const Decimal bid_depth = std::fma(position + 0.5, c2_, c1_);
        const Decimal ask_depth = std::fma(0.5 - position, c2_, c1_);
        const Decimal bid = mid_price - bid_depth;
        const Decimal ask = mid_price + ask_depth;
        if (!std::isfinite(bid) || !std::isfinite(ask))
        {
            throw GLFTQuoteError("GLFT bid and ask prices must be finite");
        }

        // A bid fill adds one inventory unit and an ask fill removes one.
        // Compare against the bounds directly to avoid incrementing or
        // decrementing inventory at the limits of Quantity.
        constexpr Decimal order_size = 1.0;
        const Decimal bid_size = inventory < max_position ? order_size : 0.0;
        const Decimal ask_size = inventory > min_position ? order_size : 0.0;
        return Quote{bid, ask, bid_size, ask_size, time};
    }
} // namespace hawkes_market_making::market_making
