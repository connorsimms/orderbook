#pragma once

using Price = std::int64_t;
using Size  = std::uint64_t;
using OrderId = std::uint64_t;

enum class Side { Buy, Sell };

enum class OrderType
{
    AllOrNone,
    FillAndKill,
    GoodForDay,
    GoodTillCancel,
    FillOrKill,
    Market,
};
