#include "data/book_reconstructor.hpp"

namespace hawkes_market_making::data {

BookSnapshot BookReconstructor::reconstruct(const std::vector<Event>& events) const {
    (void)events;
    return BookSnapshot{};
}

void BookReconstructor::reset() {
    snapshot_ = BookSnapshot{};
}

} // namespace hawkes_market_making::data