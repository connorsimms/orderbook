#pragma once

#include <map>
#include <vector>
#include <list>
#include <memory>
#include <concepts>

#include "types.h"
#include "order.h"
#include "trade.h"

template <typename T>
concept HasPopFront = requires (T t) {
    t.pop_front();
};

/**
 * @brief Represents orders at a given price level.
 *
 * Orders are stored as shared pointers, following Price-time priority
 *
 * @tparam OrderContainer   the type of container storing Order pointers 
 */
template <typename OrderContainer>
struct Level {
    Price price_;
    Size size_;
    OrderContainer orders_;

    Level (Price price, Size size, OrderContainer container)
    : price_{ price }
    , size_{ size }
    , orders_{ container }
    {}
};

/**
 * @brief Orderbook price levels are stored in a map
 *
 * @tparam Compare  the comparator used to order the map
 * @tparam OrderContainer   the type of container storing Order pointers
 */
template <typename Compare, typename OrderContainer>
struct MapPolicy {
    std::map<Price, Level<OrderContainer>, Compare> levels_;

    bool hasEnough(Price price, Size volumeNeeded) const
    {
        Compare comp;
        for (auto it = begin(levels_); it != end(levels_); ++it)
        {
            if (comp(price, it->first)) break;

            if (volumeNeeded <= it->size_) return true;

            volumeNeeded -= it->size_;
        }
        return false;
    }

    // this currently does not account for All or None orders !!
    bool canMatch(Price price) const
    {
        Compare comp;
        return (price == MARKET_PRICE) || (comp(begin(levels_)->first, price));
    }

    void add(std::shared_ptr<Order> order)
    {
        auto [it, inserted] = levels_.try_emplace(order->getPrice(), 0, OrderContainer{});
        it->second.size_ += order->getRemainingSize();
        it->second.orders_.emplace_back(std::move(order));
    }

    std::vector<Trade> match(OrderId orderId, Side side, Price price, Size& volumeRemaining)
    {
        Compare comp;
        std::vector<Trade> matches;

        for (auto lvl = begin(levels_); lvl != end(levels_) && volumeRemaining > 0; )
        {
            if (!canMatch(price)) break;

            auto& orders = lvl->second.orders_;

            while (!orders.empty() && volumeRemaining > 0)
            {
                auto& resting = orders.front();
                Size tradeSize = std::min(volumeRemaining, resting->getRemainingSize());

                TradeData incomingData{ orderId, lvl->first, tradeSize };
                TradeData restingData{ resting->getOrderId(), lvl->first, tradeSize };

                if (side == Side::Buy) { matches.emplace_back(incomingData, restingData); }
                else { matches.emplace_back(restingData, incomingData); }

                volumeRemaining -= tradeSize;
                lvl->second.size_ -= tradeSize;
                resting->fill(tradeSize);

                if (resting->isFilled()) {
                    if constexpr (HasPopFront<OrderContainer>) {
                        orders.pop_front();
                    } else {
                        orders.erase(begin(orders)); 
                    }
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
};

/**
 * @brief Orderbook price levels are stored in a vector
 *
 * @tparam Compare  the comparator used to order the vector
 * @tparam OrderContainer   the type of container storing Order pointers
 */
template <typename Compare, typename OrderContainer>
struct VectorPolicy {
    std::vector<Level<OrderContainer>> levels_;

    bool hasEnough(Price price, Size volumeNeeded) {
        Compare comp;
        for (auto it = rbegin(levels_); it != rend(levels_); ++it)
        {
            if (comp(price, it->price_)) break;

            if (volumeNeeded <= it->size_) return true;

            volumeNeeded -= it->size_;
        }
        return false;
    }

    void add(std::shared_ptr<Order> order)
    {
        Price orderPrice = order->getPrice();
        Compare comp;

        auto it = std::lower_bound(begin(levels_), end(levels_), orderPrice, 
            [&comp](const Level<OrderContainer>& level, Price price) {
                return comp(price, level.price_);
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
};

template <typename Compare, typename OrderContainer>
struct ListPolicy {
    std::list<Level<OrderContainer>> levels_;

    bool hasEnough(Price price, Size volumeNeeded) {
        Compare comp;
        for (auto it = begin(levels_); it != end(levels_); ++it)
        {
            if (comp(price, it->price_)) break;

            if (volumeNeeded <= it->size_) return true;

            volumeNeeded -= it->size_;
        }
        return false;
    }

    void add(std::shared_ptr<Order> order)
    {
        Price orderPrice = order->getPrice();
        Compare comp;

        auto it = std::lower_bound(begin(levels_), end(levels_), orderPrice,
            [&comp](const Level<OrderContainer>& level, Price price) {
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
};

