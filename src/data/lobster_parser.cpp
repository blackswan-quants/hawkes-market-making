#include "data/lobster_parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace hawkes_market_making::data
{

    LobsterParser::LobsterParser(std::string file_path) : file_path_(std::move(file_path))
    {

        std::ifstream input(file_path_);
        if (!input)
        {
            throw std::invalid_argument("LobsterParser: invalid file path: " + file_path_);
        }
    }

    namespace
    {

        // LOBSTER encodes the event kind as a compact integer code; we normalize it to the app's
        // enum.
        EventType parse_event_type(const std::string& type_token)
        {
            const std::string normalized = [&type_token]()
            {
                std::string value = type_token;
                while (!value.empty() && (value.front() == ' ' || value.front() == '\t'))
                {
                    value.erase(value.begin());
                }
                while (!value.empty() &&
                       (value.back() == ' ' || value.back() == '\t' || value.back() == '\r'))
                {
                    value.pop_back();
                }
                return value;
            }();

            if (normalized == "MarketOrder" || normalized == "0")
            {
                return EventType::MarketOrder;
            }
            if (normalized == "LimitOrder" || normalized == "1")
            {
                return EventType::LimitOrder;
            }
            if (normalized == "Cancel" || normalized == "2")
            {
                return EventType::Cancel;
            }
            if (normalized == "Deletion" || normalized == "Delete" || normalized == "3")
            {
                return EventType::Deletion;
            }
            if (normalized == "Trade" || normalized == "4")
            {
                return EventType::Trade;
            }
            if (normalized == "HiddenTrade" || normalized == "5")
            {
                return EventType::HiddenTrade;
            }
            if (normalized == "TradingHalt" || normalized == "7")
            {
                return EventType::TradingHalt;
            }

            return EventType::BookUpdate;
        }

    } // namespace

    std::vector<Event> LobsterParser::parse() const
    {
        // Each row is expected to follow the LOBSTER feed layout:
        // time, type, order_id, size, price_ticks, direction, [optional message].
        std::ifstream input(file_path_);
        std::vector<Event> events;
        std::string line;

        while (std::getline(input, line))
        {
            if (line.empty())
            {
                continue;
            }

            std::stringstream ss(line);
            std::string token;
            std::vector<std::string> fields;

            while (std::getline(ss, token, ','))
            {
                fields.push_back(token);
            }

            if (fields.size() < 6)
            {
                continue;
            }

            // Convert the raw CSV pieces into the internal event representation used by the book
            // logic.
            Event event{};
            event.time = std::stod(fields[0]);
            event.type = parse_event_type(fields[1]);
            event.order_id = std::stoll(fields[2]);
            event.size = std::stoll(fields[3]);

            // LOBSTER stores price as integer ticks; divide by 10,000 to recover the real dollar
            // price.
            const auto raw_price = std::stoll(fields[4]);
            event.price = static_cast<Decimal>(raw_price) / 10000.0;

            const std::string& side_token = fields[5];
            if (side_token == "Buy" || side_token == "buy" || side_token == "B" ||
                side_token == "1" || side_token == "+1")
            {
                event.side = OrderSide::Buy;
            }
            else if (side_token == "Sell" || side_token == "sell" || side_token == "S" ||
                     side_token == "-1" || side_token == "-1.0")
            {
                event.side = OrderSide::Sell;
            }
            else
            {
                event.side = (std::stoi(side_token) >= 0) ? OrderSide::Buy : OrderSide::Sell;
            }

            if (fields.size() > 6)
            {
                event.message = fields[6];
            }

            events.push_back(event);
        }

        return events;
    }

} // namespace hawkes_market_making::data
