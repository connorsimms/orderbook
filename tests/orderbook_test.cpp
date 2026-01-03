#include <gtest/gtest.h>

#include "orderbook/orderbook.h"

using OrderBookPolicies =
    ::testing::Types<OrderBook<MapLevelPolicy, DequeOrderPolicy>,
                     OrderBook<MapLevelPolicy, ListOrderPolicy>,
                     OrderBook<MapLevelPolicy, VectorOrderPolicy>,
                     OrderBook<VectorLevelPolicy, DequeOrderPolicy>,
                     OrderBook<VectorLevelPolicy, ListOrderPolicy>,
                     OrderBook<VectorLevelPolicy, VectorOrderPolicy>,
                     OrderBook<ListLevelPolicy, DequeOrderPolicy>,
                     OrderBook<ListLevelPolicy, ListOrderPolicy>,
                     OrderBook<ListLevelPolicy, VectorOrderPolicy>>;

template <typename OrderBookPolicy> class OrderBookTest : public testing::Test
{
public:
  OrderBookPolicy orderbook_;
};

TYPED_TEST_SUITE(OrderBookTest, OrderBookPolicies);

// MKT, FOK, FAK, GTC, GFD, AON

TYPED_TEST(OrderBookTest, GTCMatchingAndPartialFills)
{
  auto trades1 = this->orderbook_.addOrder(
      OrderType::GoodTillCancel, OrderId{1}, Side::Sell, Price{100}, Size{100});
  EXPECT_TRUE(trades1.empty());

  auto trades2 = this->orderbook_.addOrder(
      OrderType::GoodTillCancel, OrderId{2}, Side::Buy, Price{100}, Size{50});

  ASSERT_EQ(trades2.size(), 1);
  EXPECT_EQ(trades2[0].getBid().size_, 50);
  EXPECT_EQ(trades2[0].getAsk().orderId_, 1);
  EXPECT_EQ(trades2[0].getBid().orderId_, 2);

  auto trades3 = this->orderbook_.addOrder(
      OrderType::GoodTillCancel, OrderId{3}, Side::Buy, Price{100}, Size{60});

  ASSERT_EQ(trades3.size(), 1);
  EXPECT_EQ(trades3[0].getBid().size_, 50);
  EXPECT_EQ(trades3[0].getAsk().orderId_, 1);

  auto trades4 = this->orderbook_.addOrder(
      OrderType::GoodTillCancel, OrderId{4}, Side::Sell, Price{100}, Size{10});
  ASSERT_EQ(trades4.size(), 1);
  EXPECT_EQ(trades4[0].getBid().orderId_, 3);
  EXPECT_EQ(trades4[0].getAsk().size_, 10);
}

TYPED_TEST(OrderBookTest, PricePriority)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Sell,
                            Price{101}, Size{10});
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{2}, Side::Sell,
                            Price{100}, Size{10});
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{3}, Side::Sell,
                            Price{102}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{4},
                                          Side::Buy, Price{103}, Size{30});

  ASSERT_EQ(trades.size(), 3);
  EXPECT_EQ(trades[0].getAsk().price_, 100);
  EXPECT_EQ(trades[0].getAsk().orderId_, 2);

  EXPECT_EQ(trades[1].getAsk().price_, 101);
  EXPECT_EQ(trades[1].getAsk().orderId_, 1);

  EXPECT_EQ(trades[2].getAsk().price_, 102);
  EXPECT_EQ(trades[2].getAsk().orderId_, 3);
}

TYPED_TEST(OrderBookTest, TimePriority)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Buy,
                            Price{100}, Size{10});
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{2}, Side::Buy,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{3},
                                          Side::Sell, Price{100}, Size{10});

  ASSERT_EQ(trades.size(), 1);
  EXPECT_EQ(trades[0].getBid().orderId_, 1);
}

TYPED_TEST(OrderBookTest, MarketOrderLogic)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Sell,
                            Price{100}, Size{10});
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{2}, Side::Sell,
                            Price{101}, Size{10});

  auto trades = this->orderbook_.addOrder(
      OrderType::Market, OrderId{3}, Side::Buy, Price{MARKET_PRICE}, Size{15});

  ASSERT_EQ(trades.size(), 2);
  EXPECT_EQ(trades[0].getAsk().price_, 100);
  EXPECT_EQ(trades[0].getBid().size_, 10);

  EXPECT_EQ(trades[1].getAsk().price_, 101);
  EXPECT_EQ(trades[1].getBid().size_, 5);

  EXPECT_FALSE(this->orderbook_.empty());
}

TYPED_TEST(OrderBookTest, MarketOrderNoLiquidity)
{
  auto trades = this->orderbook_.addOrder(
      OrderType::Market, OrderId{1}, Side::Buy, Price{MARKET_PRICE}, Size{10});
  EXPECT_TRUE(trades.empty());
  EXPECT_TRUE(this->orderbook_.empty());
}

TYPED_TEST(OrderBookTest, FOKSuccess)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Sell,
                            Price{100}, Size{10});
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{2}, Side::Sell,
                            Price{101}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::FillOrKill, OrderId{3},
                                          Side::Buy, Price{101}, Size{20});

  EXPECT_EQ(trades.size(), 2);
  EXPECT_TRUE(this->orderbook_.empty());
}

TYPED_TEST(OrderBookTest, FOKNotEnoughVolume)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Sell,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::FillOrKill, OrderId{3},
                                          Side::Buy, Price{100}, Size{20});

  EXPECT_TRUE(trades.empty());
  EXPECT_FALSE(this->orderbook_.empty());
}

TYPED_TEST(OrderBookTest, FOKPriceTooHigh)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Sell,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::FillOrKill, OrderId{3},
                                          Side::Buy, Price{99}, Size{10});

  EXPECT_TRUE(trades.empty());
  EXPECT_FALSE(this->orderbook_.empty());
}

TYPED_TEST(OrderBookTest, FAK)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Sell,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::FillAndKill, OrderId{2},
                                          Side::Buy, Price{100}, Size{20});

  ASSERT_EQ(trades.size(), 1);
  EXPECT_EQ(trades[0].getBid().size_, 10);
  EXPECT_TRUE(this->orderbook_.empty());
}

TYPED_TEST(OrderBookTest, AONRestingLogic)
{
  this->orderbook_.addOrder(OrderType::AllOrNone, OrderId{1}, Side::Sell,
                            Price{100}, Size{20});

  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{2}, Side::Sell,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{3},
                                          Side::Buy, Price{100}, Size{15});

  ASSERT_EQ(trades.size(), 1);
  EXPECT_EQ(trades[0].getAsk().orderId_, 2);
  EXPECT_EQ(trades[0].getBid().size_, 10);

  auto trades2 = this->orderbook_.addOrder(
      OrderType::GoodTillCancel, OrderId{4}, Side::Buy, Price{100}, Size{20});
  ASSERT_EQ(trades2.size(), 1);
  EXPECT_EQ(trades2[0].getAsk().orderId_, 1);
}

TYPED_TEST(OrderBookTest, CancelOrder)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Buy,
                            Price{100}, Size{10});
  EXPECT_FALSE(this->orderbook_.empty());

  this->orderbook_.cancelOrder(OrderId{1});
  EXPECT_TRUE(this->orderbook_.empty());

  this->orderbook_.cancelOrder(OrderId{99});
}

TYPED_TEST(OrderBookTest, ModifyOrder)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Buy,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.modifyOrder(
      OrderType::GoodTillCancel, OrderId{1}, Side::Buy, Price{102}, Size{20});

  auto trades2 = this->orderbook_.addOrder(
      OrderType::GoodTillCancel, OrderId{2}, Side::Sell, Price{102}, Size{20});

  ASSERT_EQ(trades2.size(), 1);
  EXPECT_EQ(trades2[0].getBid().orderId_, 1);
  EXPECT_EQ(trades2[0].getBid().size_, 20);
}

TYPED_TEST(OrderBookTest, BidsAndAsksAreSeparate)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Buy,
                            Price{100}, Size{10});

  auto trades = this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{2},
                                          Side::Buy, Price{100}, Size{10});

  EXPECT_TRUE(trades.empty());
}

TYPED_TEST(OrderBookTest, ListPolicyCancel)
{
  this->orderbook_.addOrder(OrderType::GoodTillCancel, OrderId{1}, Side::Buy,
                            Price{100}, Size{10});

  this->orderbook_.cancelOrder(OrderId{1});

  EXPECT_TRUE(this->orderbook_.empty());
}
