#include "data/book_reconstructor.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>

namespace hawkes_market_making::data
{

    namespace
    {

        struct OrderRecord
        {
            OrderSide side = OrderSide::Buy;
            Decimal price = 0.0;
            Quantity size = 0;
        };

    } // namespace
    // namespace

    BookSnapshot BookReconstructor::reconstruct(const std::vector<Event>& events) const
    {
        // Keep the live order state by ID so we can know which price and side an order belongs to.
        std::unordered_map<std::int64_t, OrderRecord> orders;
        // Aggregate visible liquidity by price level to mimic the order book's ladder structure.
        std::map<Decimal, Quantity> bids;
        std::map<Decimal, Quantity> asks;

        for (const Event& event : events)
        {
            // The side tells us which ladder to update; the event type tells us how to adjust it.
            auto& ladder = (event.side == OrderSide::Buy) ? bids : asks;

            switch (event.type)
            {

            case EventType::LimitOrder:
            {
                // A new order creates a tracked live order and increments the corresponding price
                // level.
                orders[event.order_id] = OrderRecord{event.side, event.price, event.size};
                ladder[event.price] += event.size;
                break;
            }

            case EventType::Cancel:

            case EventType::Trade:
            {
                // Partial cancellations and visible executions reduce the remaining size of a live
                // order.
                auto it = orders.find(event.order_id);
                if (it == orders.end())
                {
                    break;
                }

                OrderRecord& order = it->second;
                const Quantity consumed = std::min(event.size, order.size);
                order.size -= consumed;

                auto& side_ladder = (order.side == OrderSide::Buy) ? bids : asks;
                side_ladder[order.price] -= consumed;
                if (side_ladder[order.price] <= 0)
                {
                    side_ladder.erase(order.price);
                }

                if (order.size == 0)
                {
                    orders.erase(it);
                }
                break;
            }

            case EventType::Deletion:
            {
                // A full deletion removes the order and its entire remaining quantity from the
                // ladder.
                auto it = orders.find(event.order_id);
                if (it == orders.end())
                {
                    break;
                }

                const OrderRecord order = it->second;
                orders.erase(it);

                auto& side_ladder = (order.side == OrderSide::Buy) ? bids : asks;
                side_ladder[order.price] -= order.size;
                if (side_ladder[order.price] <= 0)
                {
                    side_ladder.erase(order.price);
                }
                break;
            }

            case EventType::HiddenTrade:
            case EventType::TradingHalt:
            case EventType::MarketOrder:
            case EventType::BookUpdate:
            default:
                break;
            }
        }

        // Convert the aggregated price maps into the snapshot structure used by the rest of the
        // project.
        BookSnapshot snapshot{};

        for (auto it = asks.begin(); it != asks.end(); ++it)
        {
            snapshot.asks.push_back(BookLevel{it->first, it->second});
        }
        for (auto it = bids.rbegin(); it != bids.rend(); ++it)
        {
            snapshot.bids.push_back(BookLevel{it->first, it->second});
        }

        return snapshot;
    }

    void BookReconstructor::reset() { snapshot_ = BookSnapshot{}; }

} // namespace hawkes_market_making::data