#include <gtest/gtest.h>

#include "orderbook.h"
#include "types.h"

TEST(OrderbookTest, AddToEmptyBook) 
{
    OrderBook<MapLevelPolicy, ListOrderPolicy> orderbook = OrderBook<MapLevelPolicy, ListOrderPolicy>();

    auto trades = orderbook.addOrder(OrderType::Market, OrderId{1}, Side::Buy, Price{1}, Size{1});

    ASSERT_EQ(trades.size(), 0);
}

int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
