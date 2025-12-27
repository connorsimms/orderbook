#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <list>
#include <memory>
#include <concepts>

#include "types.h"
#include "order.h"
#include "trade.h"
#include "price_level.h"

/**
 * @brief Orderbook policy for storing PriceLevel%s in std::map
 *
 * @tparam Compare          the comparator for map ordering
 * @tparam OrderContainer   the type of container storing OrderPointer%s
 */
template <typename Compare, typename OrderContainer>
class MapLevelPolicy 
{
public:
    // unsure if this is necessary
    MapLevelPolicy()
    : levels_{}
    , comp_{}
    {}

    /**
     * @brief Checks if aggressing order can be completely filled
     *
     * @details Double for-loop is used because of possibly AllOrNone resting
     *          orders that may not be matchable to aggressor.
     *          AllOrNone orders that are too big are skipped.
     */
    bool canFill(Price const& aggressorPrice, Size volumeNeeded) const
    {
        for (const auto& [restingPrice, level]: levels_)
        {
            if (comp_(aggressorPrice, restingPrice)) break;

            auto& [_, levelSize, orders] = level;
            for (auto ord = orders.cbegin(); ord != orders.cend() && volumeNeeded > 0; )
            {
                auto resting = *ord;

                if (resting->getOrderType() == OrderType::AllOrNone)
                {
                    if (resting->getRemainingSize() > volumeNeeded)
                    {
                        ++ord;
                        continue;
                    }
                }
                
                volumeNeeded -= std::min(volumeNeeded, resting->getRemainingSize());
                if (volumeNeeded == 0) return true;
            }
        }
        return false;
    }

    Trades match(OrderType const& orderType, OrderId const& orderId, Side const& side, Price const& price, Size& volumeRemaining, const auto& onRemove)
    {
        Trades matches;

        for (auto lvl = levels_.begin(); lvl != levels_.end() && volumeRemaining > 0; )
        {
            if (price != MARKET_PRICE && comp_(price, begin(levels_)->first)) break;

            auto& [restingPrice, level] = *lvl;
            auto& orders = level.orders_;

            for (auto ord = orders.begin(); ord != orders.end() && volumeRemaining > 0; )
            {
                auto resting = *ord;

                if (resting->getOrderType() == OrderType::AllOrNone)
                {
                    if (resting->getRemainingSize() > volumeRemaining)
                    {
                        ++ord;
                        continue;
                    }
                }

                Size tradeSize = std::min(volumeRemaining, resting->getRemainingSize());

                TradeData incomingData{ orderId, restingPrice, tradeSize };
                TradeData restingData{ resting->getOrderId(), restingPrice, tradeSize };

                if (side == Side::Buy) 
                { 
                    matches.emplace_back(incomingData, restingData); 
                }
                else 
                { 
                    matches.emplace_back(restingData, incomingData); 
                }

                volumeRemaining -= tradeSize;
                level.size_ -= tradeSize;
                resting->fill(tradeSize);

                if (resting->isFilled()) 
                {
                    onRemove(resting->getOrderId());
                    ord = orders.erase(ord);
                }
                else
                {
                    ++ord;
                }
            }

            if (orders.empty()) {
                lvl = levels_.erase(lvl); 
            } else {
                ++lvl;
            }
        }
        return matches;
    }

    void add(OrderPointer order)
    {
        auto [it, inserted] = levels_.try_emplace(order->getPrice(), order->getPrice());
        auto& [price, level] = *it;
        level.size_ += order->getRemainingSize();
        level.orders_.add(order);
    }

    void cancel(OrderPointer order)
    {
        if (!levels_.contains(order->getPrice())) return;

        levels_[order->getPrice()].orders_.cancel(order);
    }

private:
    std::map<Price, PriceLevel<OrderContainer>, Compare> levels_;
    Compare comp_;
};

/**
 * @brief Orderbook price levels are stored in a vector
 *
 * @tparam Compare  the comparator used to order the vector
 * @tparam OrderContainer   the type of container storing Order pointers
 */
template <typename Compare, typename OrderContainer>
class VectorLevelPolicy {
public:
    VectorLevelPolicy()
    : levels_{}
    , comp_{}
    { }

    bool hasEnough(Price const& price, Size const& volumeNeeded) const
    {
    }

    Trades match(OrderId const& orderId, Side const& side, Price const& price, Size& volumeRemaining);

    void add(OrderPointer order)
    {
        Price orderPrice = order->getPrice();
        Compare comp;

        auto it = std::lower_bound(begin(levels_), end(levels_), orderPrice, 
            [&comp](const PriceLevel<OrderContainer>& level, Price price) {
                return comp(price, level.price_);
            }
        );

        if (it != end(levels_) && it->price_ == orderPrice)
        {
            it->size_ += order->getRemainingSize();
            it->orders_.add(order);
        } 
        else 
        {
            it = levels_.emplace(it, orderPrice, order->getRemainingSize(), OrderContainer{});
            it->orders_.add(order);
        }
    }

    void cancel (OrderPointer order);

private:
    std::vector<PriceLevel<OrderContainer>> levels_;
    Compare comp_;
};

template <typename Compare, typename OrderContainer>
class ListLevelPolicy {
public:
    ListLevelPolicy()
    : levels_{}
    , comp_{}
    {}

    bool hasEnough (Price const& price, Size const& volumeNeeded) const
    {
    }

    void add(OrderPointer order)
    {
        Price orderPrice = order->getPrice();
        Compare comp;

        auto it = std::lower_bound(begin(levels_), end(levels_), orderPrice,
            [&comp](const PriceLevel<OrderContainer>& level, Price price) {
                return comp(level.price_, price);
            }
        );

        if (it != end(levels_) && it->price_ == orderPrice)
        {
            it->size_ += order->getRemainingSize();
            it->orders_.emplace_back(order);
        } 
        else 
        {
            it = levels_.emplace(it, orderPrice, order->getRemainingSize(), OrderContainer{});
            it->orders_.emplace_back(order);
        }
    }

    void cancel (OrderPointer order);

private:
    std::list<PriceLevel<OrderContainer>> levels_;
    Compare comp_;
};

