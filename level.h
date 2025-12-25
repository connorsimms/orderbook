#pragma once

#include <map>
#include <vector>
#include <list>

template <typename OrderContainer>
struct Level {
    Price price_;
    Size size_;
    OrderContainer orders_;
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
};

template <typename Compare, typename OrderContainer>
struct VectorPolicy {
    std::vector<Level<OrderContainer>> levels_;

    bool hasEnough(Price price, Size volumeNeeded) {
        Compare comp;
        for (auto it = rbegin(levels_); it != rend(levels_); ++it)
        {
            if (comp(price, it->first)) break;

            if (volumeNeeded <= it->size_) return true;

            volumeNeeded -= it->size_;
        }
        return false;
    }
};

template <typename Compare, typename OrderContainer>
struct ListPolicy {
    std::list<Level<OrderContainer>> levels_;

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
};

