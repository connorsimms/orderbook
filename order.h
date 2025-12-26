#pragma once

#include "types.h"

class Order 
{
public:
    Order(OrderType orderType, OrderId orderId, Side side, Price price, Size volume)
        : orderType_{ orderType }
        , orderId_{ orderId }
        , side_{ side }
        , price_{ price }
        , volume_{ volume }
        , remaining_{ volume }
    {}

    OrderType getOrderType() const { return orderType_; }
    OrderId getOrderId() const { return orderId_; }
    Side getSide() const { return side_; }
    Price getPrice() const { return price_; }
    Size getInitialSize() const { return volume_; }
    Size getRemainingSize() const { return remaining_; }
    Size filledAmount() const { return volume_ - remaining_; }

    bool isFilled() const { return remaining_ == 0; }

    void fill(Size size) { remaining_ -= size; }

private:
    OrderType orderType_;
    OrderId orderId_;
    Side side_;
    Price price_;
    Size volume_;
    Size remaining_;
};


