#pragma once

#include <string>
#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::data {

class LobsterParser {
public:
    explicit LobsterParser(std::string file_path);

    std::vector<Event> parse() const;

private:
    std::string file_path_;
};

} // namespace hawkes_market_making::data
