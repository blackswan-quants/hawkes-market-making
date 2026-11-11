#pragma once

#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::data {

class BookReconstructor {
public:
    BookSnapshot reconstruct(const std::vector<Event>& events) const;
    void reset();

private:
    BookSnapshot snapshot_{};
};

} // namespace hawkes_market_making::data
