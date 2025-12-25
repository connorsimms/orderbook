#pragma once

#include <map>
#include <vector>
#include <list>
#include <memory>

#include "types.h"
#include "order.h"

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

template <typename Compare, typename OrderContainer>
struct MapPolicy {
    std::map<Price, Level<OrderContainer>, Compare> levels_;

    bool hasEnough(Price price, Size volumeNeeded) {
        Compare comp;
        for (auto it = begin(levels_); it != end(levels_); ++it)
        {
            if (comp(price, it->first)) break;

            if (volumeNeeded <= it->size_) return true;

            volumeNeeded -= it->size_;
        }
        return false;
    }

    void add(std::shared_ptr<Order> order)
    {
        auto [it, inserted] = levels_.try_emplace(order->getPrice(), 0, OrderContainer{});
        it->second.size_ += order->getRemainingSize();
        it->second.orders_.emplace_back(std::move(order));
    }
};

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

