#include "data/book_reconstructor.hpp"

#include <algorithm>
#include <map>
#include <unordered_map>

namespace hawkes_market_making::data {

namespace {

struct OrderRecord {
    OrderSide side = OrderSide::Buy;
    Decimal price = 0.0;
    Quantity size = 0;
};

} 
// namespace

BookSnapshot BookReconstructor::reconstruct(const std::vector<Event>& events) const {
    std::unordered_map<std::int64_t, OrderRecord> orders;
    std::map<Decimal, Quantity> bids;
    std::map<Decimal, Quantity> asks;

    for (const Event& event : events) {
        auto& ladder = (event.side == OrderSide::Buy) ? bids : asks;

        switch (event.type) {

            case EventType::LimitOrder: {
                orders[event.order_id] = OrderRecord{event.side, event.price, event.size};
                ladder[event.price] += event.size;
                break;
            }

            case EventType::Cancel:

            case EventType::Trade: {
                auto it = orders.find(event.order_id);
                if (it == orders.end()) {
                    break;
                }

                OrderRecord& order = it->second;
                const Quantity consumed = std::min(event.size, order.size);
                order.size -= consumed;

                auto& side_ladder = (order.side == OrderSide::Buy) ? bids : asks;
                side_ladder[order.price] -= consumed;
                if (side_ladder[order.price] <= 0) {
                    side_ladder.erase(order.price);
                }

                if (order.size == 0) {
                    orders.erase(it);
                }
                break;
            }
            
            case EventType::Deletion: {
                auto it = orders.find(event.order_id);
                if (it == orders.end()) {
                    break;
                }

                const OrderRecord order = it->second;
                orders.erase(it);

                auto& side_ladder = (order.side == OrderSide::Buy) ? bids : asks;
                side_ladder[order.price] -= order.size;
                if (side_ladder[order.price] <= 0) {
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

    BookSnapshot snapshot{};

    for (auto it = asks.begin(); it != asks.end(); ++it) {
        snapshot.asks.push_back(BookLevel{it->first, it->second});
    }
    for (auto it = bids.rbegin(); it != bids.rend(); ++it) {
        snapshot.bids.push_back(BookLevel{it->first, it->second});
    }

    return snapshot;
}

void BookReconstructor::reset() {
    snapshot_ = BookSnapshot{};
}

} // namespace hawkes_market_making::data