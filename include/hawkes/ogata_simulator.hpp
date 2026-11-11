#pragma once

#include <vector>

#include "common/types.hpp"
#include "hawkes/hawkes_process.hpp"

namespace hawkes_market_making::hawkes {

class OgataSimulator {
public:
    explicit OgataSimulator(HawkesParameters parameters);

    std::vector<Event> simulate(Time horizon, std::size_t seed = 0) const;

private:
    HawkesParameters parameters_;
};

} // namespace hawkes_market_making::hawkes
