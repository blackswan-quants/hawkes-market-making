#include "data/lobster_parser.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace hawkes_market_making::data {

LobsterParser::LobsterParser(std::string file_path)
    : file_path_(std::move(file_path)) {

    std::ifstream input(file_path_);
    if (!input) {
        throw std::invalid_argument("LobsterParser: invalid file path: " + file_path_);
    }
}

std::vector<Event> LobsterParser::parse() const {
    
    std::ifstream input(file_path_);
    std::vector<Event> events;
    std::string line;

    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }

        std::stringstream ss(line);
        std::string token;
        std::vector<std::string> fields;

        while (std::getline(ss, token, ',')) {
            fields.push_back(token);
        }

        if (fields.size() < 5) {
            continue;
        }

        Event event{};
        event.time = std::stod(fields[0]);

        const std::string& type_token = fields[1];
        if (type_token == "MarketOrder") {
            event.type = EventType::MarketOrder;
        } else if (type_token == "LimitOrder") {
            event.type = EventType::LimitOrder;
        } else if (type_token == "Cancel") {
            event.type = EventType::Cancel;
        } else if (type_token == "Trade") {
            event.type = EventType::Trade;
        } else {
            event.type = EventType::BookUpdate;
        }

        const std::string& side_token = fields[2];
        if (side_token == "Buy" || side_token == "buy" || side_token == "0") {
            event.side = OrderSide::Buy;
        } else {
            event.side = OrderSide::Sell;
        }

        event.price = std::stod(fields[3]);
        event.size = std::stoll(fields[4]);

        if (fields.size() > 5) {
            event.message = fields[5];
        }

        events.push_back(event);
    }

    return events;
}

} // namespace hawkes_market_making::data

