#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hawkes_market_making
{

    using Decimal = double;
    using Time = Decimal;
    using Quantity = std::int64_t;

    struct Timestamp
    {
        Decimal value = 0.0;
    };

    enum class OrderSide : std::uint8_t
    {
        Buy = 0,
        Sell = 1,
    };
    
    /*
    From LOBSTR we get
    1: Submission of a new limit order
    2: Cancellation (Partial deletion 
        of a limit order)
    3: Deletion (Total deletion of a limit order)
    4: Execution of a visible limit order			   	 
    5: Execution of a hidden limit order
    7: Trading halt indicator 				   
    
    If your LOB data is different some tweaks may be in order
    */
    enum class EventType : std::uint8_t
    {
        MarketOrder = 0,
        LimitOrder = 1,
        Cancel = 2,
        Deletion = 3,
        Trade = 4,
        HiddenTrade = 5,
        TradingHalt = 7,
        BookUpdate = 8,
    };

    struct BookLevel
    {
        Decimal price = 0.0;
        Quantity volume = 0;
    };

    struct BookSnapshot
    {
        std::vector<BookLevel> bids;
        std::vector<BookLevel> asks;
    };

    struct Event
    {
        Time time = 0.0;
        EventType type = EventType::BookUpdate;
        std::int64_t order_id = 0;
        Quantity size = 0;
        Decimal price = 0.0;
        OrderSide side = OrderSide::Buy;
        std::string message;
    };

} // namespace hawkes_market_making
