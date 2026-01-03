#pragma once

#include "orderbook/order_policy.h"
#include "orderbook/types.h"
#include <memory>

template <typename OrderContainer> struct PriceLevel
{
  Price price_;
  Size size_;
  OrderContainer orders_;

  PriceLevel(Price const &price) : price_{price}, size_{}, orders_{} {}
};
