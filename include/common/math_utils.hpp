#pragma once

#include <algorithm>
#include <cmath>

namespace hawkes_market_making {

inline double clamp(double value, double lower, double upper) {
    return std::max(lower, std::min(value, upper));
}

inline double safe_exp(double x) {
    if (x > 700.0) {
        return std::exp(700.0);
    }
    return std::exp(x);
}

} // namespace hawkes_market_making
