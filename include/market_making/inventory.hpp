#pragma once

#include <cstdint>

namespace hawkes_market_making::market_making {

class Inventory {
public:
    Inventory() = default;
    explicit Inventory(std::int64_t initial_inventory) : inventory_(initial_inventory) {}

    std::int64_t position() const { return inventory_; }
    void add(std::int64_t delta) { inventory_ += delta; }
    void set(std::int64_t value) { inventory_ = value; }

private:
    std::int64_t inventory_ = 0;
};

} // namespace hawkes_market_making::market_making
