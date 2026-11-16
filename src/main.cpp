#include <cstdint>
#include <iomanip>
#include <iostream>

#include "market_making/avellaneda_stoikov.hpp"

// Small demo: prints the Avellaneda-Stoikov quotes for a few inventory levels, so the
// inventory skew can be eyeballed without writing a backtest. Uses the default parameters.
int main()
{
    using hawkes_market_making::market_making::AvellanedaStoikov;
    using hawkes_market_making::market_making::Quote;

    const AvellanedaStoikov market_maker;
    const double mid_price = 100.0;
    const double time = 0.0;
    const int column_width = 12;

    std::cout << std::fixed << std::setprecision(4);
    std::cout << std::setw(column_width) << "inventory" << std::setw(column_width) << "bid"
              << std::setw(column_width) << "ask" << '\n';

    for (std::int64_t inventory = -4; inventory <= 4; inventory += 2)
    {
        const Quote q = market_maker.quote(time, mid_price, inventory);
        std::cout << std::setw(column_width) << inventory << std::setw(column_width) << q.bid
                  << std::setw(column_width) << q.ask << '\n';
    }

    return 0;
}
