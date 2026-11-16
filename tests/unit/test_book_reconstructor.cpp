#include <gtest/gtest.h>

#include "data/book_reconstructor.hpp"

namespace {

using hawkes_market_making::BookLevel;
using hawkes_market_making::BookSnapshot;
using hawkes_market_making::Event;
using hawkes_market_making::EventType;
using hawkes_market_making::OrderSide;
using hawkes_market_making::Quantity;
using hawkes_market_making::data::BookReconstructor;

TEST(BookReconstructorTest, ReconstructsBidAndAskLevels) {
    BookReconstructor reconstructor;

    std::vector<Event> events = {
        Event{0.0, EventType::LimitOrder, 10, 50, 100.0, OrderSide::Buy, ""},
        Event{0.0, EventType::LimitOrder, 11, 30, 101.0, OrderSide::Sell, ""},
        Event{0.0, EventType::LimitOrder, 12, 20, 100.0, OrderSide::Buy, ""},
    };

    const BookSnapshot book = reconstructor.reconstruct(events);

    ASSERT_EQ(book.bids.size(), 1u);
    ASSERT_EQ(book.asks.size(), 1u);
    EXPECT_DOUBLE_EQ(book.bids[0].price, 100.0);
    EXPECT_EQ(book.bids[0].volume, 70);
    EXPECT_DOUBLE_EQ(book.asks[0].price, 101.0);
    EXPECT_EQ(book.asks[0].volume, 30);
}

TEST(BookReconstructorTest, AppliesPartialCancelsAndFullDeletions) {
    BookReconstructor reconstructor;

    std::vector<Event> events = {
        Event{1.0, EventType::LimitOrder, 20, 50, 99.5, OrderSide::Buy, ""},
        Event{1.1, EventType::Cancel, 20, 20, 99.5, OrderSide::Buy, ""},
        Event{1.2, EventType::Deletion, 20, 0, 99.5, OrderSide::Buy, ""},
    };

    const BookSnapshot book = reconstructor.reconstruct(events);

    EXPECT_TRUE(book.bids.empty());
    EXPECT_TRUE(book.asks.empty());
}

TEST(BookReconstructorTest, ReducesBookOnVisibleExecution) {
    BookReconstructor reconstructor;

    std::vector<Event> events = {
        Event{2.0, EventType::LimitOrder, 30, 40, 102.0, OrderSide::Buy, ""},
        Event{2.1, EventType::Trade, 30, 15, 102.0, OrderSide::Buy, ""},
    };

    const BookSnapshot book = reconstructor.reconstruct(events);

    ASSERT_EQ(book.bids.size(), 1u);
    EXPECT_DOUBLE_EQ(book.bids[0].price, 102.0);
    EXPECT_EQ(book.bids[0].volume, 25);
}

TEST(BookReconstructorTest, IgnoresHiddenTradesAndHalts) {
    BookReconstructor reconstructor;

    std::vector<Event> events = {
        Event{3.0, EventType::LimitOrder, 40, 10, 105.0, OrderSide::Sell, ""},
        Event{3.1, EventType::HiddenTrade, 40, 5, 105.0, OrderSide::Sell, ""},
        Event{3.2, EventType::TradingHalt, 0, 0, 0.0, OrderSide::Buy, ""},
    };

    const BookSnapshot book = reconstructor.reconstruct(events);

    ASSERT_EQ(book.asks.size(), 1u);
    EXPECT_DOUBLE_EQ(book.asks[0].price, 105.0);
    EXPECT_EQ(book.asks[0].volume, 10);
}

}  // namespace
