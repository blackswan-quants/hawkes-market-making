/**
 * @file
 * @brief Deterministic tests of the long-horizon GLFT strategy.
 *
 * @details Covers the Task 3 quote checks, the paper's reference depths,
 * and validation of parameters, inputs, and inventory limits.
 */

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include "common/types.hpp"
#include "market_making/glft.hpp"

namespace
{
    using hawkes_market_making::Decimal;
    using hawkes_market_making::Quantity;
    using hawkes_market_making::Time;
    using hawkes_market_making::market_making::GLFT;
    using hawkes_market_making::market_making::GLFTParameterError;
    using hawkes_market_making::market_making::GLFTParameters;
    using hawkes_market_making::market_making::GLFTQuoteError;
    using hawkes_market_making::market_making::MarketMakerBase;

    /// Default timestamp in seconds for tests that hold time fixed.
    constexpr Time kQuoteTime = 0.0;

    /// Reference mid-price in ticks for the fixed quote examples.
    constexpr Decimal kMidPrice = 100.0;

    /// Absolute tolerance in ticks for rounding of reference depths and quoted prices.
    constexpr Decimal kPriceTolerance = 1e-10;

    /// Absolute tolerance for the small, binary-exact order sizes and timestamps in these tests.
    constexpr Decimal kExactValueTolerance = 1e-12;

    /**
     * @brief Check both quote depths against an independent numerical reference.
     *
     * @details Use the parameters from the left panel of Figure 4 and call
     * MarketMakerBase with inventory +5. Comparing both depths catches errors in the
     * coefficients and in the bid and ask signs.
     */
    TEST(GLFTTest, MatchesPaperReferenceDepths)
    {
        const GLFT glft;
        const MarketMakerBase& strategy = glft;
        constexpr Quantity inventory = 5;

        const auto quote = strategy.quote(kQuoteTime, kMidPrice, inventory);

        // The Section 4 reference depths are rounded to twelve decimal places.
        constexpr Decimal expected_bid_depth = 3.776665054192;
        constexpr Decimal expected_ask_depth = 2.871787287114;
        EXPECT_NEAR(kMidPrice - quote.bid, expected_bid_depth, kPriceTolerance);
        EXPECT_NEAR(quote.ask - kMidPrice, expected_ask_depth, kPriceTolerance);
    }

    /**
     * @brief Check that the quotes are centered at zero inventory.
     *
     * @details The bid and ask depths must match within the price tolerance and
     * both must be positive. A flat position has no reason to favor a side.
     */
    TEST(GLFTTest, CentersQuotesAtZeroInventory)
    {
        const GLFT glft;

        const auto quote = glft.quote(kQuoteTime, kMidPrice, 0);

        EXPECT_NEAR(kMidPrice - quote.bid, quote.ask - kMidPrice, kPriceTolerance);
        EXPECT_LT(quote.bid, kMidPrice);
        EXPECT_GT(quote.ask, kMidPrice);
    }

    /**
     * @brief Check that both quote prices decrease at each step from -5 to +5.
     *
     * @details A larger position moves the bid farther below the mid and the ask
     * downward, discouraging another buy and making a sale more attractive.
     */
    TEST(GLFTTest, LowersQuotesAsInventoryIncreases)
    {
        const GLFT glft;
        constexpr Quantity first_position = -5;
        constexpr Quantity last_position = 5;

        auto previous = glft.quote(kQuoteTime, kMidPrice, first_position);
        for (Quantity position = first_position + 1; position <= last_position; ++position)
        {
            // Include the inventory step in the failure output.
            SCOPED_TRACE(position);
            const auto quote = glft.quote(kQuoteTime, kMidPrice, position);

            EXPECT_LT(quote.bid, previous.bid);
            EXPECT_LT(quote.ask, previous.ask);
            previous = quote;
        }
    }

    /**
     * @brief Check that reversing inventory swaps the bid and ask depths.
     *
     * @details Compare the depths for +5 and -5. The symmetric model should apply
     * the same inventory adjustment in opposite directions.
     */
    TEST(GLFTTest, MirrorsDepthsForOppositePositions)
    {
        const GLFT glft;
        constexpr Quantity inventory = 5;

        const auto long_quote = glft.quote(kQuoteTime, kMidPrice, inventory);
        const auto short_quote = glft.quote(kQuoteTime, kMidPrice, -inventory);

        EXPECT_NEAR(kMidPrice - long_quote.bid, short_quote.ask - kMidPrice, kPriceTolerance);
        EXPECT_NEAR(long_quote.ask - kMidPrice, kMidPrice - short_quote.bid, kPriceTolerance);
    }

    /**
     * @brief Check that a signed depth can put a quote across the mid-price.
     *
     * @details At +50 the ask is below the mid, and at -50 the bid is above it.
     * These cases catch accidental clamping of the raw GLFT prices.
     */
    TEST(GLFTTest, PreservesSignedDepthsAcrossTheMidPrice)
    {
        const GLFT glft;
        constexpr Quantity inventory = 50;

        const auto long_quote = glft.quote(kQuoteTime, kMidPrice, inventory);
        const auto short_quote = glft.quote(kQuoteTime, kMidPrice, -inventory);

        EXPECT_LT(long_quote.ask, kMidPrice);
        EXPECT_GT(short_quote.bid, kMidPrice);
    }

    /**
     * @brief Check that the quote prices are independent of time.
     *
     * @details Hold parameters, mid-price, and inventory fixed and change only the
     * timestamp. The long-horizon formula gives the same prices at both times.
     */
    TEST(GLFTTest, QuotePricesAreStationary)
    {
        const GLFT glft;
        constexpr Quantity inventory = 5;

        const auto first = glft.quote(kQuoteTime, kMidPrice, inventory);
        const auto later = glft.quote(23.5, kMidPrice, inventory);

        EXPECT_NEAR(first.bid, later.bid, kPriceTolerance);
        EXPECT_NEAR(first.ask, later.ask, kPriceTolerance);
    }

    /**
     * @brief Check that the quote carries the timestamp supplied by the caller.
     *
     * @details The replay layer can use this value to associate the quote with its event.
     */
    TEST(GLFTTest, PreservesTheSuppliedTimestamp)
    {
        const GLFT glft;
        constexpr Time timestamp = 23.5;

        const auto quote = glft.quote(timestamp, kMidPrice, 0);

        EXPECT_NEAR(quote.timestamp, timestamp, kExactValueTolerance);
    }

    /**
     * @brief Check the analytic depth at exactly zero risk aversion.
     *
     * @details Use nonzero inventory and compare both depths with 1 / kappa. This
     * checks the zero-risk limit where the inventory adjustment vanishes.
     */
    TEST(GLFTTest, ZeroRiskAversionGivesLiquidityOnlyDepths)
    {
        GLFTParameters parameters;
        parameters.risk_aversion = 0.0;
        const GLFT glft{parameters};
        const Decimal expected_depth = 1.0 / parameters.fill_rate_decay;

        const auto quote = glft.quote(kQuoteTime, kMidPrice, 5);

        EXPECT_NEAR(kMidPrice - quote.bid, expected_depth, kPriceTolerance);
        EXPECT_NEAR(quote.ask - kMidPrice, expected_depth, kPriceTolerance);
    }

    /**
     * @brief Check that zero volatility makes the prices independent of inventory.
     *
     * @details With mid-price and time fixed, compare positions 0 and +5. The
     * inventory adjustment vanishes when the price has zero volatility.
     */
    TEST(GLFTTest, ZeroVolatilityRemovesInventorySkew)
    {
        GLFTParameters parameters;
        parameters.volatility = 0.0;
        const GLFT glft{parameters};

        const auto flat = glft.quote(kQuoteTime, kMidPrice, 0);
        const auto long_quote = glft.quote(kQuoteTime, kMidPrice, 5);

        EXPECT_NEAR(long_quote.bid, flat.bid, kPriceTolerance);
        EXPECT_NEAR(long_quote.ask, flat.ask, kPriceTolerance);
    }

    /**
     * @brief Check the direction of convergence as risk aversion approaches zero.
     *
     * @details At inventory +5, each depth must be closer to 1 / kappa at risk
     * aversion 1e-12 than at 1e-6. This exercises the positive-risk formula
     * near its analytic limit.
     */
    TEST(GLFTTest, DepthsApproachLiquidityOnlyLimitAsRiskAversionDecreases)
    {
        GLFTParameters parameters;
        parameters.risk_aversion = 1e-6;
        const GLFT higher_risk_aversion{parameters};
        parameters.risk_aversion = 1e-12;
        const GLFT lower_risk_aversion{parameters};
        const Decimal limit = 1.0 / parameters.fill_rate_decay;
        constexpr Quantity inventory = 5;

        const auto higher = higher_risk_aversion.quote(kQuoteTime, kMidPrice, inventory);
        const auto lower = lower_risk_aversion.quote(kQuoteTime, kMidPrice, inventory);

        EXPECT_LT(std::abs((kMidPrice - lower.bid) - limit),
                  std::abs((kMidPrice - higher.bid) - limit));
        EXPECT_LT(std::abs((lower.ask - kMidPrice) - limit),
                  std::abs((higher.ask - kMidPrice) - limit));
    }

    /**
     * @brief Check that a bid is allowed at Q - 1 and disabled at Q.
     *
     * @details A one-unit buy can reach the cap from Q - 1. At Q, only the sell
     * can fill without increasing the absolute inventory.
     */
    TEST(GLFTTest, DisablesBidAtLongInventoryCap)
    {
        GLFTParameters parameters;
        parameters.inventory_cap = 12;
        const GLFT glft{parameters};

        const auto inside = glft.quote(kQuoteTime, kMidPrice, parameters.inventory_cap - 1);
        const auto at_cap = glft.quote(kQuoteTime, kMidPrice, parameters.inventory_cap);

        EXPECT_NEAR(inside.size_bid, 1.0, kExactValueTolerance);
        EXPECT_NEAR(at_cap.size_bid, 0.0, kExactValueTolerance);
        EXPECT_NEAR(at_cap.size_ask, 1.0, kExactValueTolerance);
    }

    /**
     * @brief Check that an ask is allowed at -Q + 1 and disabled at -Q.
     *
     * @details A one-unit sell can reach the cap from -Q + 1. At -Q, only the buy
     * can fill without increasing the absolute inventory.
     */
    TEST(GLFTTest, DisablesAskAtShortInventoryCap)
    {
        GLFTParameters parameters;
        parameters.inventory_cap = 12;
        const GLFT glft{parameters};

        const auto inside = glft.quote(kQuoteTime, kMidPrice, 1 - parameters.inventory_cap);
        const auto at_cap = glft.quote(kQuoteTime, kMidPrice, -parameters.inventory_cap);

        EXPECT_NEAR(inside.size_ask, 1.0, kExactValueTolerance);
        EXPECT_NEAR(at_cap.size_ask, 0.0, kExactValueTolerance);
        EXPECT_NEAR(at_cap.size_bid, 1.0, kExactValueTolerance);
    }

    /**
     * @brief Check that already out-of-range positions raise GLFTQuoteError.
     *
     * @details With Q = 12, try +13 and -13 so replay can surface an invalid
     * position on either side before using its quotes.
     */
    TEST(GLFTTest, RejectsInventoryOutsideTheCap)
    {
        GLFTParameters parameters;
        parameters.inventory_cap = 12;
        const GLFT glft{parameters};

        EXPECT_THROW(glft.quote(kQuoteTime, kMidPrice, 13), GLFTQuoteError);
        EXPECT_THROW(glft.quote(kQuoteTime, kMidPrice, -13), GLFTQuoteError);
    }

    /**
     * @brief Check that large adjacent inventories still give distinct cap decisions.
     *
     * @details Set Q to the first integer that double cannot represent exactly.
     * The bid is active one unit inside the cap and disabled at the cap.
     * Zero risk aversion isolates the size check from the inventory price skew.
     */
    TEST(GLFTTest, DistinguishesAdjacentInventoryAboveDoubleIntegerPrecision)
    {
        constexpr Quantity cap = (Quantity{1} << std::numeric_limits<Decimal>::digits) + 1;
        GLFTParameters parameters;
        parameters.inventory_cap = cap;
        parameters.risk_aversion = 0.0;
        const GLFT glft{parameters};

        const auto inside = glft.quote(kQuoteTime, kMidPrice, cap - 1);
        const auto at_cap = glft.quote(kQuoteTime, kMidPrice, cap);

        EXPECT_NEAR(inside.size_bid, 1.0, kExactValueTolerance);
        EXPECT_NEAR(at_cap.size_bid, 0.0, kExactValueTolerance);
    }

    /**
     * @brief Check the cap rule at the maximum positive Quantity.
     *
     * @details Test both symmetric limits and the smallest Quantity, which lies
     * one unit below the lower limit. This exercises the bounds where
     * adding or subtracting a unit could overflow.
     */
    TEST(GLFTTest, EnforcesTheLargestRepresentablePositiveCap)
    {
        constexpr Quantity cap = std::numeric_limits<Quantity>::max();
        GLFTParameters parameters;
        parameters.inventory_cap = cap;
        parameters.risk_aversion = 0.0;
        const GLFT glft{parameters};

        const auto upper = glft.quote(kQuoteTime, kMidPrice, cap);
        const auto lower = glft.quote(kQuoteTime, kMidPrice, -cap);

        EXPECT_NEAR(upper.size_bid, 0.0, kExactValueTolerance);
        EXPECT_NEAR(lower.size_ask, 0.0, kExactValueTolerance);
        EXPECT_THROW(glft.quote(kQuoteTime, kMidPrice, std::numeric_limits<Quantity>::min()),
                     GLFTQuoteError);
    }

    /**
     * @brief Check that the constructor rejects zero and negative caps.
     *
     * @details The model configuration requires a positive whole-unit inventory bound.
     */
    TEST(GLFTTest, RejectsNonPositiveInventoryCaps)
    {
        GLFTParameters parameters;
        parameters.inventory_cap = 0;

        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);
        parameters.inventory_cap = -1;
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);
    }

    /**
     * @brief Check the domains of the fill-rate decay and scale.
     *
     * @details Exercise zero decay and negative scale separately. The formula
     * uses positive liquidity parameters to compute finite quote depths.
     */
    TEST(GLFTTest, RejectsNonPositiveLiquidityParameters)
    {
        GLFTParameters parameters;
        parameters.fill_rate_decay = 0.0;
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);

        parameters = GLFTParameters{};
        parameters.fill_rate_scale = -1.0;
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);
    }

    /**
     * @brief Check that negative risk aversion and volatility are rejected.
     *
     * @details Exercise each field independently to verify both constructor checks.
     */
    TEST(GLFTTest, RejectsNegativeRiskParameters)
    {
        GLFTParameters parameters;
        parameters.risk_aversion = -1.0;
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);

        parameters = GLFTParameters{};
        parameters.volatility = -1.0;
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);
    }

    /**
     * @brief Check that the constructor rejects non-finite model parameters.
     *
     * @details Inject NaN or infinity into each real-valued input in turn. Reset
     * the configuration between checks so each failure identifies one field.
     */
    TEST(GLFTTest, RejectsNonFiniteModelParameters)
    {
        GLFTParameters parameters;
        parameters.risk_aversion = std::numeric_limits<Decimal>::quiet_NaN();
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);

        parameters = GLFTParameters{};
        parameters.volatility = std::numeric_limits<Decimal>::infinity();
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);

        parameters = GLFTParameters{};
        parameters.fill_rate_decay = std::numeric_limits<Decimal>::infinity();
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);

        parameters = GLFTParameters{};
        parameters.fill_rate_scale = std::numeric_limits<Decimal>::quiet_NaN();
        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);
    }

    /**
     * @brief Check that the constructor rejects a coefficient that overflows.
     *
     * @details Use individually valid finite inputs with extreme volatility to
     * exercise validation after the coefficients are computed.
     */
    TEST(GLFTTest, RejectsNonFiniteDerivedCoefficients)
    {
        GLFTParameters parameters;
        parameters.risk_aversion = 1.0;
        parameters.fill_rate_decay = 1.0;
        parameters.fill_rate_scale = 1e-3;
        parameters.volatility = std::numeric_limits<Decimal>::max();

        EXPECT_THROW((void)GLFT{parameters}, GLFTParameterError);
    }

    /**
     * @brief Check that an infinite or NaN mid-price raises GLFTQuoteError.
     *
     * @details The failure prevents a caller from receiving unusable bid and ask prices.
     */
    TEST(GLFTTest, RejectsNonFiniteMidPrices)
    {
        const GLFT glft;
        constexpr Decimal infinity = std::numeric_limits<Decimal>::infinity();
        constexpr Decimal nan = std::numeric_limits<Decimal>::quiet_NaN();

        EXPECT_THROW(glft.quote(kQuoteTime, infinity, 0), GLFTQuoteError);
        EXPECT_THROW(glft.quote(kQuoteTime, nan, 0), GLFTQuoteError);
    }
} // namespace
