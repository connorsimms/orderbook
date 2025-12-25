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
    std::vector<Trade> match(OrderId orderId, Side side, Price price, Size volume)
    {
        if (side == Side::Buy) { return askLevels_.match(orderId, side, price, volume); }
        else { return bidLevels_.match(price, volume); }
    }

    bool canFullyFill(Side side, Price price, Size volume)
    {
        if (side == Side::Buy) { return askLevels_.hasEnough(price, volume); }
        else { return bidLevels_.hasEnough(price, volume); }
    }
    /* 
     * Will add functionality to return matched trades
     */
    std::vector<Trade> addOrder(OrderType orderType, OrderId orderId, Side side, Price price, Size volume)
    {
        // deal with FOK immediately without adding to book
        if (orderType == OrderType::FillOrKill)
        {
            if (canFullyFill(side, price, volume))
            {
                // match and return trades
                return match(orderId, side, price, volume);
            }
            else
            {
                return {};
            }
        }

        // deal with IOC/FAK immediately, Market is similar
        if (orderType == OrderType::FillAndKill || orderType == OrderType::Market)
        {
            // Fill as much as possible
             
            // remaining will not be added to book

            return match(orderId, side, price, volume);
        }

        // for GFD and GTC, match as much as possible
        // then add remainder to the books
        auto orderPointer = std::make_shared<Order>(orderType, orderId, side, price, volume);
        order_[orderId] = orderPointer;

        if (side == Side::Buy)
        {
            bidLevels_.add(orderPointer);
        }
        else
        {
            askLevels_.add(orderPointer);
        }
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
