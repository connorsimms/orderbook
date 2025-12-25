#pragma once

#include "types.h"
#include "order.h"

struct TradeData
{
    OrderId orderId_;
    Price price_;
    Size size_;
};

class Trade
{
public:
    Trade(const TradeData& bid, const TradeData& ask)
    : bid_{ bid }
    , ask_{ ask }
    {}

    const TradeData getBid() const { return bid_; }
    const TradeData getAsk() const { return ask_; }

private:
    TradeData bid_;
    TradeData ask_;
};
