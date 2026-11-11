#pragma once

#include <vector>

#include "common/types.hpp"

namespace hawkes_market_making::control {

struct Grid1D {
    Decimal min = 0.0;
    Decimal max = 1.0;
    std::size_t size = 64;
    Decimal dx() const { return (max - min) / static_cast<Decimal>(size - 1); }
};

class Grid {
public:
    Grid();
    Grid(Grid1D buy_axis, Grid1D sell_axis, std::size_t inventory_levels = 9);

    std::vector<Decimal> values_buy() const;
    std::vector<Decimal> values_sell() const;
    std::size_t inventory_levels() const { return inventory_levels_; }

private:
    Grid1D buy_axis_;
    Grid1D sell_axis_;
    std::size_t inventory_levels_ = 9;
};

} // namespace hawkes_market_making::control
