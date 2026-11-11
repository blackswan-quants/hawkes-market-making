#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hawkes_market_making {

using Decimal = double;
using Time = Decimal;
using Quantity = std::int64_t;

struct Timestamp {
    Decimal value = 0.0;
};

enum class OrderSide : std::uint8_t {
    Buy = 0,
    Sell = 1,
};

enum class EventType : std::uint8_t {
    MarketOrder = 0,
    LimitOrder = 1,
    Cancel = 2,
    Trade = 3,
    BookUpdate = 4,
};

struct BookLevel {
    Decimal price = 0.0;
    Quantity volume = 0;
};

struct BookSnapshot {
    std::vector<BookLevel> bids;
    std::vector<BookLevel> asks;
};

struct Event {
    Time time = 0.0;
    EventType type = EventType::BookUpdate;
    OrderSide side = OrderSide::Buy;
    Decimal price = 0.0;
    Quantity size = 0;
    std::string message;
};

} // namespace hawkes_market_making
