#pragma once

#include <map>
#include <vector>
#include <list>

template <typename OrderContainer>
struct Level {
    Price price_;
    OrderContainer orders_;
};

template <typename Compare, typename OrderContainer>
struct MapPolicy {
    std::map<Price, Level<OrderContainer>, Compare> levels_;
};

template <typename Compare, typename OrderContainer>
struct VectorPolicy {
    std::vector<Level<OrderContainer>> levels_;
};

template <typename Compare, typename OrderContainer>
struct ListPolicy {
    std::list<Level<OrderContainer>> levels_;
};

