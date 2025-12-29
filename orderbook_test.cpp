#include <gtest/gtest.h>

#include "orderbook.h"
#include "types.h"

using OrderBookPolicies = ::testing::Types<
    OrderBook<MapLevelPolicy, DequeOrderPolicy>,
    OrderBook<MapLevelPolicy, ListOrderPolicy>,
    OrderBook<MapLevelPolicy, VectorOrderPolicy>
>;

template <typename OrderBookPolicy>
class OrderBookTest : public testing::Test
{
protected:
    OrderBookPolicy orderbook_;
};

TYPED_TEST_SUITE(OrderBookTest, OrderBookPolicies);

TYPED_TEST(OrderBookTest, AddToEmptyBook) 
{
    auto mkt_trades = this->orderbook_.addOrder(OrderType::Market, OrderId{1}, Side::Buy, Price{1}, Size{1});
    EXPECT_TRUE(mkt_trades.empty()) << "Market order returned none-empty trades";

    auto fok_trades = this->orderbook_.addOrder(OrderType::FillOrKill, OrderId{1}, Side::Buy, Price{1}, Size{1});
    EXPECT_TRUE(fok_trades.empty()) << "FOK order returned none-empty trades";

    auto fak_trades = this->orderbook_.addOrder(OrderType::FillAndKill, OrderId{1}, Side::Buy, Price{1}, Size{1});
    EXPECT_TRUE(fak_trades.empty()) << "FAK order returned none-empty trades";

    EXPECT_TRUE(this->orderbook_.empty()) << "Orderbook is not empty";
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
