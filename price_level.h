#pragma once

#include <memory>
#include "order_policy.h"
#include "types.h"

template <typename OrderContainer>
struct PriceLevel
{
    Price price_;
    Size size_;
    OrderContainer orders_;

    PriceLevel (Price const& price)
    : price_{ price }
    , size_{}
    , orders_{}
    {}

    // maybe std::move version too
    PriceLevel (Price const& price, Size const& size, OrderContainer orders)
    : price_{ price }
    , size_{ size }
    , orders_{ std::move(orders) }
    {}
};
