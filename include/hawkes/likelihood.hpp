#pragma once

#include <vector>

#include "common/types.hpp"
#include "hawkes/hawkes_process.hpp"

namespace hawkes_market_making::hawkes {

class Likelihood {
public:
    explicit Likelihood(HawkesParameters parameters);

    Decimal log_likelihood(const std::vector<Time>& event_times) const;

private:
    HawkesParameters parameters_;
};

} // namespace hawkes_market_making::hawkes
