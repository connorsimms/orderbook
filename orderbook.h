#pragma once

#include <unordered_map>
#include <memory>
#include <functional>
#include <algorithm>

#include "order.h"
#include "level_policy.h"
#include "types.h"
#include "trade.h"

/*
 * @tparam LevelContainer   container used to store PriceLevel%s
 * @tparam OrderContainer   container used to store Order%s as OrderPointer%s
 */
template <template <typename, typename> class LevelContainer, typename OrderContainer> 
class OrderBook
{
public:
    OrderBook()
    : bidLevels_{}
    , askLevels_{}
    , existingOrders_{}
    {}

    /**
     * @brief Matches aggressing order against resting orders
     */
    Trades match(OrderType const& orderType, OrderId const& orderId, Side const& side, Price const& price, Size& volume, auto const& onRemove)
    {
        if (side == Side::Buy) { return askLevels_.match(orderType, orderId, side, price, volume, onRemove); }
        else { return bidLevels_.match(orderType, orderId, side, price, volume, onRemove); }
    }

    /**
     * @brief Checks if aggressing order can be completely filled
     */
    bool canFullyFill(const Side& side, const Price& price, const Size& volume) const
    {
        if (side == Side::Buy) { return askLevels_.canFill(price, volume); }
        else { return bidLevels_.canFill(price, volume); }
    }

    /*
     * @brief Matches/adds aggressing order, according to its type
     */
    Trades addOrder(OrderType orderType, OrderId orderId, Side side, Price price, Size volume)
    {
        if (existingOrders_.contains(orderId)) return {};

        if (orderType == OrderType::FillOrKill)
        {
            if (!canFullyFill(side, price, volume))
            {
                return {};
            }
        }

        Trades trades;

        // Fill as much as possible
        if (orderType != OrderType::AllOrNone || canFullyFill(side, price, volume))
        {
            trades = match(orderId, side, price, volume, [&](OrderId filledId) {
                existingOrders_.erase(filledId);
            });
        }

        // Remaining not added to book
        if (orderType == OrderType::FillAndKill || orderType == OrderType::Market || volume <= 0)
        {
            return trades;
        }

        // Add remainder to book to rest
        auto order = std::make_shared<Order>(orderType, orderId, side, price, volume);
        existingOrders_[orderId] = order;
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

    /*
     * @brief Cancels resting order
     */
    void cancelOrder(OrderId orderId)
    {
        if (!existingOrders_.contains(orderId)) return;

        auto order = existingOrders_[orderId];

        if (order->getSide() == Side::Buy)
        {
            bidLevels_.cancel(order); 
        }
        else
        {
            askLevels_.cancel(order); 
        }

        existingOrders_.erase(orderId);
    }

    /*
     * @brief Modifies existing order, requeuing at the desired price level
     */
    Trades modifyOrder(OrderType newType, OrderId orderId, Side newSide, Price newPrice, Size newVolume)
    {
        cancelOrder(orderId);

        return addOrder(newType, orderId, newSide, newPrice, newVolume);
    }


private:
    std::unordered_map<OrderId, OrderPointer> existingOrders_;
    LevelContainer<std::greater<Price>, OrderContainer> bidLevels_;
    LevelContainer<std::less<Price>, OrderContainer> askLevels_;
};
