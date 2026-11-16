#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

#include "data/lobster_parser.hpp"

namespace
{

    using hawkes_market_making::Decimal;
    using hawkes_market_making::Event;
    using hawkes_market_making::EventType;
    using hawkes_market_making::OrderSide;
    using hawkes_market_making::data::LobsterParser;

    TEST(LobsterParserTest, ParsesValidCsvRows)
    {
        const auto temp_path = std::filesystem::temp_directory_path() / "lobster_parser_test.csv";

        {
            std::ofstream file(temp_path);
            ASSERT_TRUE(file.is_open());
            file << "1.5,4,42,10,1002500,1,ok\n";
            file << "2.0,1,99,25,1015000,-1,\n";
            file.close();
        }

        LobsterParser parser(temp_path.string());
        const std::vector<Event> events = parser.parse();

        ASSERT_EQ(events.size(), 2u);

        EXPECT_DOUBLE_EQ(events[0].time, 1.5);
        EXPECT_EQ(events[0].type, EventType::Trade);
        EXPECT_EQ(events[0].order_id, 42);
        EXPECT_EQ(events[0].size, 10);
        EXPECT_DOUBLE_EQ(events[0].price, 100.25);
        EXPECT_EQ(events[0].side, OrderSide::Buy);
        EXPECT_EQ(events[0].message, "ok");

        EXPECT_DOUBLE_EQ(events[1].time, 2.0);
        EXPECT_EQ(events[1].type, EventType::LimitOrder);
        EXPECT_EQ(events[1].order_id, 99);
        EXPECT_EQ(events[1].size, 25);
        EXPECT_DOUBLE_EQ(events[1].price, 101.5);
        EXPECT_EQ(events[1].side, OrderSide::Sell);
        EXPECT_TRUE(events[1].message.empty());

        std::filesystem::remove(temp_path);
    }

    TEST(LobsterParserTest, ParsesLobsterNumericEventCodes)
    {
        const auto temp_path =
            std::filesystem::temp_directory_path() / "lobster_parser_numeric_types.csv";

        {
            std::ofstream file(temp_path);
            ASSERT_TRUE(file.is_open());
            file << "1.0,1,100,10,1000000,1\n";
            file << "2.0,2,101,12,1010000,-1\n";
            file << "3.0,3,102,8,1020000,1\n";
            file << "4.0,4,103,4,1030000,-1\n";
            file << "5.0,5,104,6,1040000,1\n";
            file << "6.0,7,105,2,1050000,-1\n";
            file.close();
        }

        LobsterParser parser(temp_path.string());
        const std::vector<Event> events = parser.parse();

        ASSERT_EQ(events.size(), 6u);
        EXPECT_EQ(events[0].type, EventType::LimitOrder);
        EXPECT_EQ(events[1].type, EventType::Cancel);
        EXPECT_EQ(events[2].type, EventType::Deletion);
        EXPECT_EQ(events[3].type, EventType::Trade);
        EXPECT_EQ(events[4].type, EventType::HiddenTrade);
        EXPECT_EQ(events[5].type, EventType::TradingHalt);

        std::filesystem::remove(temp_path);
    }

    TEST(LobsterParserTest, ThrowsForInvalidPath)
    {
        const auto missing_path =
            std::filesystem::temp_directory_path() / "definitely_missing_lobster_file.csv";

        EXPECT_THROW({ LobsterParser parser(missing_path.string()); }, std::invalid_argument);
    }

} // namespace
