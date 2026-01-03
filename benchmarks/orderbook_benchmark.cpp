#include <benchmark/benchmark.h>

#include "orderbook/orderbook.h"

template<class OrderBookPolicy>
class OrderBookFixture : public benchmark::Fixture
{
public:
    OrderBookPolicy orderbook_;
};

BENCHMARK_TEMPLATE_METHOD_F(OrderBookFixture, BM_AddOrder)(benchmark::State& state)
{
    OrderId id = 0;

    for (auto _ : state)
    {
        this->orderbook_.addOrder(
            OrderType::GoodTillCancel, OrderId{++id}, Side::Buy, Price{100}, Size{10}
        );
    }
}

BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<MapLevelPolicy, DequeOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<MapLevelPolicy, ListOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<MapLevelPolicy, VectorOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<VectorLevelPolicy, DequeOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<VectorLevelPolicy, ListOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<VectorLevelPolicy, VectorOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<ListLevelPolicy, DequeOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<ListLevelPolicy, ListOrderPolicy>);
BENCHMARK_TEMPLATE_INSTANTIATE_F(OrderBookFixture, BM_AddOrder, OrderBook<ListLevelPolicy, VectorOrderPolicy>);

BENCHMARK_MAIN();
