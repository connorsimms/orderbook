#pragma once

#include <unordered_map>
#include <memory>
#include <functional>

#include "order.h"
#include "level.h"

template <template <typename, typename> class LevelPolicy, typename OrderContainer> 
class OrderBook
{
public:
    void addOrder(Side side, Price price, Size size);
    void cancelOrder(OrderId orderId);
    void modifyOrder(OrderId orderId);

private:
    LevelPolicy<std::greater<Price>, OrderContainer> bidLevels_;
    LevelPolicy<std::less<Price>, OrderContainer> askLevels_;
    std::unordered_map<OrderId, std::shared_ptr<Order>> order_;
};
