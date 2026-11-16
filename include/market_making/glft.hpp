/**
 * @file
 * @brief Long-horizon GLFT strategy and model parameters.
 */

#pragma once

#include <stdexcept>

#include "common/constants.hpp"
#include "common/types.hpp"
#include "market_making/market_maker_base.hpp"

namespace hawkes_market_making::market_making
{
    /**
     * @brief Configuration for the inventory-bounded GLFT approximation.
     *
     * @details The risk and liquidity defaults match the left panel of Figure 4
     * in Guéant, Lehalle and Fernandez-Tapia, "Dealing with the Inventory Risk".
     * They use ticks for price and seconds for time. If you use other units,
     * convert the model inputs together and use the same price unit for the
     * mid-price and quote depths.
     */
    struct GLFTParameters
    {
        Decimal risk_aversion = 0.01;  ///< Gamma, in inverse price units.
        Decimal fill_rate_decay = 0.3; ///< Kappa, in inverse price units.
        Decimal fill_rate_scale = 0.9; ///< A, arrival rate at zero depth, per unit of time.
        Decimal volatility = 0.4;      ///< Sigma, in price units per square root of time.

        /**
         * @brief Maximum absolute position in whole inventory units.
         *
         * @details The model assumes one inventory unit per fill. The default
         * is the project's inventory cap.
         */
        Quantity inventory_cap = kMaxInventory;
    };

    /**
     * @brief Error for invalid GLFT parameters or non-finite derived coefficients.
     */
    class GLFTParameterError : public std::invalid_argument
    {
      public:
        using std::invalid_argument::invalid_argument;
    };

    /**
     * @brief Error for an inventory outside the configured cap or a non-finite quote price.
     */
    class GLFTQuoteError : public std::invalid_argument
    {
      public:
        using std::invalid_argument::invalid_argument;
    };

    /**
     * @brief Market-making strategy using the long-horizon GLFT approximation.
     *
     * @details The strategy stores its configuration and caches the stationary
     * quote coefficients for reuse across quote calculations.
     */
    class GLFT : public MarketMakerBase
    {
      public:
        /**
         * @brief Validate and store the configuration and compute the stationary coefficients.
         *
         * @param parameters Model parameters. Risk aversion and volatility must
         * be finite and nonnegative. Fill-rate decay and fill-rate scale must
         * be finite and positive. The whole-unit inventory cap must be positive.
         * @throws GLFTParameterError If a parameter fails validation or either
         * derived coefficient is non-finite.
         * @note At zero risk aversion, c1 is the reciprocal of the fill-rate
         * decay and c2 is zero. At zero volatility, c2 is also zero.
         */
        explicit GLFT(GLFTParameters parameters = {});

        /**
         * @brief Retrieve the configuration used to construct this strategy.
         * @return A copy of the stored parameters for recording or reproducing a run.
         */
        GLFTParameters parameters() const noexcept { return parameters_; }

        /**
         * @brief Generate a stationary GLFT quote for the supplied mid-price and inventory.
         *
         * @details A side quotes one inventory unit when a fill would keep
         * the position within the cap. A disabled side has zero size. Depths
         * are signed, so the price of an active side can pass the reference mid.
         *
         * @param time Timestamp to associate with the quote.
         * @param mid_price Reference mid-price in the configuration's price unit.
         * @param inventory Current signed position in inventory units.
         * @return Bid and ask prices in the configuration's price unit, quote
         * sizes in inventory units, and the supplied timestamp. Consumers use
         * prices only for sides with a positive size.
         * @throws GLFTQuoteError If inventory is outside the cap or a resulting
         * price is non-finite.
         */
        Quote quote(Time time, Decimal mid_price, Quantity inventory) const override;

      private:
        GLFTParameters parameters_; ///< Configuration used to compute the coefficients.
        Decimal c1_ = 0.0;          ///< Inventory-independent term in each quote depth.
        Decimal c2_ = 0.0;          ///< Change in depth per unit of inventory.
    };
} // namespace hawkes_market_making::market_making
