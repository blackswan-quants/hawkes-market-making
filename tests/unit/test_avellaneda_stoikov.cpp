#include <gtest/gtest.h>

#include <cmath>
#include <stdexcept>

#include "common/types.hpp"
#include "market_making/avellaneda_stoikov.hpp"

namespace
{

    using hawkes_market_making::Decimal;
    using hawkes_market_making::Time;
    using hawkes_market_making::market_making::AvellanedaStoikov;
    using hawkes_market_making::market_making::AvellanedaStoikovParameters;
    using hawkes_market_making::market_making::Quote;

    // Prices are around 100, so rounding errors are of order 1e-13: 1e-9 is safely above.
    constexpr Decimal kTolerance = 1e-9;

    // Mid price and time used by most tests.
    constexpr Decimal kMid = 100.0;
    constexpr Time kHalfway = 0.5;

    TEST(AvellanedaStoikov, ZeroInventoryReservationPriceEqualsMid)
    {
        const AvellanedaStoikov mm;

        const Decimal reservation = mm.reservation_price(kHalfway, kMid, 0);

        EXPECT_NEAR(reservation, kMid, kTolerance);
    }

    TEST(AvellanedaStoikov, ZeroInventoryQuotesAreCenteredOnMid)
    {
        const AvellanedaStoikov mm;

        const Quote q = mm.quote(kHalfway, kMid, 0);

        EXPECT_NEAR(kMid - q.bid, q.ask - kMid, kTolerance);
        EXPECT_LT(q.bid, q.ask);
    }

    TEST(AvellanedaStoikov, PositiveInventoryShiftsQuotesDownward)
    {
        const AvellanedaStoikov mm;

        const Quote flat = mm.quote(kHalfway, kMid, 0);
        const Quote long_position = mm.quote(kHalfway, kMid, 5);

        // A long dealer wants to sell, so both bid and ask move down.
        EXPECT_LT(long_position.bid, flat.bid);
        EXPECT_LT(long_position.ask, flat.ask);
    }

    TEST(AvellanedaStoikov, NegativeInventoryShiftsQuotesUpward)
    {
        const AvellanedaStoikov mm;

        const Quote flat = mm.quote(kHalfway, kMid, 0);
        const Quote short_position = mm.quote(kHalfway, kMid, -5);

        EXPECT_GT(short_position.bid, flat.bid);
        EXPECT_GT(short_position.ask, flat.ask);
    }

    TEST(AvellanedaStoikov, SpreadDoesNotDependOnInventory)
    {
        const AvellanedaStoikov mm;

        const Quote flat = mm.quote(kHalfway, kMid, 0);
        const Quote long_position = mm.quote(kHalfway, kMid, 25);

        EXPECT_NEAR(long_position.ask - long_position.bid, flat.ask - flat.bid, kTolerance);
    }

    TEST(AvellanedaStoikov, QuotesMatchClosedForm)
    {
        AvellanedaStoikovParameters p;
        p.risk_aversion = 0.5;
        p.volatility = 1.2;
        p.arrival_decay = 2.0;
        p.horizon = 2.0;
        const AvellanedaStoikov mm(p);

        const Decimal reservation = mm.reservation_price(0.5, kMid, 3);
        const Decimal spread = mm.optimal_spread(0.5);

        // r = 100 - 3 * 0.5 * 1.44 * 1.5 = 96.76
        // spread = 0.5 * 1.44 * 1.5 + (2 / 0.5) * ln(1 + 0.5 / 2)
        EXPECT_NEAR(reservation, 96.76, kTolerance);
        EXPECT_NEAR(spread, 1.08 + 4.0 * std::log(1.25), kTolerance);
    }

    TEST(AvellanedaStoikov, SpreadShrinksAsTimeApproachesHorizon)
    {
        const AvellanedaStoikov mm;
        const Time horizon = mm.parameters().horizon;

        const Decimal at_start = mm.optimal_spread(0.0);
        const Decimal halfway = mm.optimal_spread(0.5 * horizon);
        const Decimal at_end = mm.optimal_spread(horizon);

        EXPECT_GT(at_start, halfway);
        EXPECT_GT(halfway, at_end);
    }

    TEST(AvellanedaStoikov, TimeBeyondHorizonBehavesLikeHorizon)
    {
        const AvellanedaStoikov mm;
        const Time horizon = mm.parameters().horizon;

        const Quote at_horizon = mm.quote(horizon, kMid, 3);
        const Quote after_horizon = mm.quote(horizon + 10.0, kMid, 3);

        EXPECT_NEAR(after_horizon.bid, at_horizon.bid, kTolerance);
        EXPECT_NEAR(after_horizon.ask, at_horizon.ask, kTolerance);
    }

    TEST(AvellanedaStoikov, SmallRiskAversionGivesLiquidityOnlySymmetricQuotes)
    {
        AvellanedaStoikovParameters p;
        p.risk_aversion = 1e-8;
        const AvellanedaStoikov mm(p);

        const Quote q = mm.quote(0.0, kMid, 20);

        // With gamma = 1e-8 the terms that vanish in the limit are below 1e-6.
        const Decimal small_gamma_tolerance = 1e-5;
        EXPECT_NEAR(q.ask - q.bid, 2.0 / p.arrival_decay, small_gamma_tolerance);
        EXPECT_NEAR(kMid - q.bid, q.ask - kMid, small_gamma_tolerance);
    }

    TEST(AvellanedaStoikov, ZeroRiskAversionGivesTheLimitSpread)
    {
        AvellanedaStoikovParameters p;
        p.risk_aversion = 0.0;
        const AvellanedaStoikov mm(p);

        const Decimal spread = mm.optimal_spread(0.0);

        EXPECT_NEAR(spread, 2.0 / p.arrival_decay, kTolerance);
    }

    TEST(AvellanedaStoikov, AverageSpreadMatchesPaperTable)
    {
        // Table 1 of the paper (gamma = 0.1, sigma = 2, kappa = 1.5, T = 1) reports an
        // average spread of 1.49. The spread is linear in t, so the average is its value at T/2.
        const AvellanedaStoikov mm;
        const Decimal paper_average_spread = 1.49;
        const Decimal two_decimals_tolerance = 0.005;

        const Decimal average = mm.optimal_spread(0.5 * mm.parameters().horizon);

        EXPECT_NEAR(average, paper_average_spread, two_decimals_tolerance);
    }

    TEST(AvellanedaStoikov, FillIntensityDecreasesWithDepth)
    {
        const AvellanedaStoikov mm;

        const Decimal at_mid = mm.fill_intensity(0.0);
        const Decimal far_from_mid = mm.fill_intensity(1.0);

        EXPECT_NEAR(at_mid, mm.parameters().arrival_scale, kTolerance);
        EXPECT_LT(far_from_mid, at_mid);
    }

    TEST(AvellanedaStoikov, InvalidParametersAreRejected)
    {
        AvellanedaStoikovParameters negative_gamma;
        negative_gamma.risk_aversion = -0.1;
        AvellanedaStoikovParameters zero_kappa;
        zero_kappa.arrival_decay = 0.0;
        AvellanedaStoikovParameters zero_horizon;
        zero_horizon.horizon = 0.0;

        EXPECT_THROW(AvellanedaStoikov{negative_gamma}, std::invalid_argument);
        EXPECT_THROW(AvellanedaStoikov{zero_kappa}, std::invalid_argument);
        EXPECT_THROW(AvellanedaStoikov{zero_horizon}, std::invalid_argument);
    }

} // namespace
