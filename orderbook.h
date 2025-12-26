#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>

#include "order.h"
#include "level.h"
#include "types.h"
#include "trade.h"

template <template <typename, typename> class LevelPolicy, typename OrderContainer> 
class OrderBook
{
public:
    std::vector<Trade> match(OrderId orderId, Side side, Price price, Size& volume)
    {
        if (side == Side::Buy) { return askLevels_.match(orderId, side, price, volume); }
        else { return bidLevels_.match(orderId, side, price, volume); }
    }

    bool canFullyFill(Side side, Price price, Size volume) const
    {
        if (side == Side::Buy) { return askLevels_.hasEnough(price, volume); }
        else { return bidLevels_.hasEnough(price, volume); }
    }
    /* 
     * Will add functionality to return matched trades
     */
    std::vector<Trade> addOrder(OrderType orderType, OrderId orderId, Side side, Price price, Size volume)
    {
        if (orderType == OrderType::FillOrKill)
        {
            if (!canFullyFill(side, price, volume))
            {
                return {};
            }
        }

        // Fill as much as possible
        std::vector<Trade> trades = match(orderId, side, price, volume);

        // remaining will not be added to book
        if (orderType == OrderType::FillAndKill || orderType == OrderType::Market)
        {
            return trades;
        }

        if (volume <= 0) { return trades; }

        // add remainder to book
        auto order = std::make_shared<Order>(orderType, orderId, side, price, volume);
        order_[orderId] = order;
        if (side == Side::Buy)
        {
            bidLevels_.add(order);
        }
        else
        {
            askLevels_.add(order);
        }

        return trades;
    }

    void cancelOrder(OrderId orderId)
    {

    }

    void modifyOrder(OrderId orderId)
    {

    }


private:
    LevelPolicy<std::greater<Price>, OrderContainer> bidLevels_;
    LevelPolicy<std::less<Price>, OrderContainer> askLevels_;
    std::unordered_map<OrderId, std::shared_ptr<Order>> order_;
};
