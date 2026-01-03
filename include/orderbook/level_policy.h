#pragma once

#include <concepts>
#include <list>
#include <map>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "orderbook/order.h"
#include "orderbook/price_level.h"
#include "orderbook/trade.h"
#include "orderbook/types.h"

/**
 * @brief Orderbook policy for storing PriceLevel%s in std::map
 *
 * @tparam Compare          the comparator for map ordering
 * @tparam OrderContainer   the type of container storing OrderPointer%s
 */
template <typename Compare, typename OrderContainer> class MapLevelPolicy
{
public:
  MapLevelPolicy() : levels_{}, comp_{} {}

  bool empty() const { return levels_.empty(); }

  Price getBest() const
  {
    if (empty())
    {
      throw std::runtime_error("Level is empty");
    }
    else
    {
      return levels_.begin()->first;
    }
  }

  /**
   * @brief Checks if aggressing order can be completely filled
   *
   * @details Double for-loop is used because of possibly AllOrNone resting
   *          orders that may not be matchable to aggressor.
   *          AllOrNone orders that are too big are skipped.
   */
  bool canFullyFill(Price const &aggressorPrice, Size volumeNeeded) const
  {
    for (const auto &[restingPrice, level] : levels_)
    {
      if (comp_(aggressorPrice, level.price_))
        break;

      for (const auto &resting : level.orders_)
      {
        if (resting->getOrderType() == OrderType::AllOrNone)
        {
          if (resting->getRemainingSize() > volumeNeeded)
            continue;
        }

        volumeNeeded -= std::min(volumeNeeded, resting->getRemainingSize());
        if (volumeNeeded == 0)
          return true;
      }
    }
    return false;
  }

  /**
   * @brief Matches aggressing order against as many resting orders as possible
   */
  Trades match(OrderId const &orderId, Side const &side, Price const &price,
               Size &volumeRemaining, const auto &onRemove)
  {
    Trades matches;

    for (auto lvl = levels_.begin();
         lvl != levels_.end() && volumeRemaining > 0;)
    {
      if (price != MARKET_PRICE && comp_(price, lvl->first))
        break;

      auto &[restingPrice, level] = *lvl;
      auto &orders = level.orders_;

      for (auto ord = orders.begin();
           ord != orders.end() && volumeRemaining > 0;)
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

        TradeData incomingData{orderId, restingPrice, tradeSize};
        TradeData restingData{resting->getOrderId(), restingPrice, tradeSize};

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

      if (orders.empty())
      {
        lvl = levels_.erase(lvl);
      }
      else
      {
        ++lvl;
      }
    }
    return matches;
  }

  /**
   * @brief Adds order to price level
   */
  void add(OrderPointer order)
  {
    auto [it, inserted] =
        levels_.try_emplace(order->getPrice(), order->getPrice());
    auto &[price, level] = *it;
    level.size_ += order->getRemainingSize();
    level.orders_.insert(order);
  }

  /**
   * @brief Cancels order in price level
   */
  void cancel(OrderPointer order)
  {
    auto it = levels_.find(order->getPrice());
    if (it == levels_.end())
      return;

    it->second.orders_.erase(order);
    it->second.size_ -= order->getRemainingSize();

    if (it->second.orders_.empty())
    {
      levels_.erase(it);
    }
  }

  std::map<Price, PriceLevel<OrderContainer>, Compare>::iterator begin()
  {
    return levels_.begin();
  }

  std::map<Price, PriceLevel<OrderContainer>, Compare>::iterator end()
  {
    return levels_.end();
  }

  std::map<Price, PriceLevel<OrderContainer>, Compare>::const_iterator
  begin() const
  {
    return levels_.begin();
  }

  std::map<Price, PriceLevel<OrderContainer>, Compare>::const_iterator
  end() const
  {
    return levels_.end();
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
template <typename Compare, typename OrderContainer> class VectorLevelPolicy
{
public:
  VectorLevelPolicy() : levels_{}, comp_{} {}

  bool empty() const { return levels_.empty(); }

  Price getBest() const
  {
    if (empty())
    {
      throw std::runtime_error("Level is empty");
    }
    else
    {
      return levels_.back().price_;
    }
  }

  bool canFullyFill(Price const &aggressorPrice, Size volumeNeeded) const
  {
    for (auto level = levels_.crbegin(); level != levels_.crend(); ++level)
    {
      if (comp_(aggressorPrice, level->price_))
        break;

      for (const auto &resting : level->orders_)
      {
        if (resting->getOrderType() == OrderType::AllOrNone)
        {
          if (resting->getRemainingSize() > volumeNeeded)
            continue;
        }

        volumeNeeded -= std::min(volumeNeeded, resting->getRemainingSize());
        if (volumeNeeded == 0)
          return true;
      }
    }
    return false;
  }

  Trades match(OrderId const &orderId, Side const &side, Price const &price,
               Size &volumeRemaining, const auto &onRemove)
  {
    Trades matches;

    for (auto level = levels_.rbegin();
         level != levels_.rend() && volumeRemaining > 0;)
    {
      if (price != MARKET_PRICE && comp_(price, level->price_))
        break;

      auto &orders = level->orders_;

      for (auto ord = orders.begin();
           ord != orders.end() && volumeRemaining > 0;)
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

        TradeData incomingData{orderId, level->price_, tradeSize};
        TradeData restingData{resting->getOrderId(), level->price_, tradeSize};

        if (side == Side::Buy)
        {
          matches.emplace_back(incomingData, restingData);
        }
        else
        {
          matches.emplace_back(restingData, incomingData);
        }

        volumeRemaining -= tradeSize;
        level->size_ -= tradeSize;
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

      if (orders.empty())
      {
        level =
            std::make_reverse_iterator(levels_.erase(std::next(level).base()));
      }
      else
      {
        ++level;
      }
    }
    return matches;
  }

  void add(OrderPointer order)
  {
    Price orderPrice = order->getPrice();

    auto lvl = std::lower_bound(
        levels_.begin(), levels_.end(), orderPrice,
        [&](const PriceLevel<OrderContainer> &level, Price price)
        { return comp_(price, level.price_); });

    if (lvl != levels_.end() && lvl->price_ == orderPrice)
    {
      lvl->size_ += order->getRemainingSize();
      lvl->orders_.insert(order);
    }
    else
    {
      lvl = levels_.emplace(lvl, orderPrice);
      lvl->orders_.insert(order);
      lvl->size_ += order->getRemainingSize();
    }
  }

  void cancel(OrderPointer order)
  {
    Price orderPrice = order->getPrice();

    auto lvl = std::find_if(levels_.begin(), levels_.end(),
                            [&](const PriceLevel<OrderContainer> &level)
                            { return orderPrice == level.price_; });

    if (lvl != levels_.end() && lvl->price_ == orderPrice)
    {
      lvl->orders_.erase(order);
      lvl->size_ -= order->getRemainingSize();

      if (lvl->orders_.empty())
      {
        levels_.erase(lvl);
      }
    }
  }

  std::vector<PriceLevel<OrderContainer>>::iterator begin()
  {
    return levels_.begin();
  }

  std::vector<PriceLevel<OrderContainer>>::iterator end()
  {
    return levels_.end();
  }

  std::vector<PriceLevel<OrderContainer>>::const_iterator begin() const
  {
    return levels_.begin();
  }

  std::vector<PriceLevel<OrderContainer>>::const_iterator end() const
  {
    return levels_.end();
  }

private:
  std::vector<PriceLevel<OrderContainer>> levels_;
  Compare comp_;
};

template <typename Compare, typename OrderContainer> class ListLevelPolicy
{
public:
  ListLevelPolicy() : levels_{}, comp_{} {}

  bool empty() const { return levels_.empty(); }

  Price getBest() const
  {
    if (empty())
    {
      throw std::runtime_error("Level is empty");
    }
    else
    {
      return levels_.front().price_;
    }
  }

  bool canFullyFill(Price const &aggressorPrice, Size volumeNeeded) const
  {
    for (auto level = levels_.cbegin(); level != levels_.cend(); ++level)
    {
      if (comp_(aggressorPrice, level->price_))
        break;

      for (const auto &resting : level->orders_)
      {
        if (resting->getOrderType() == OrderType::AllOrNone)
        {
          if (resting->getRemainingSize() > volumeNeeded)
            continue;
        }

        volumeNeeded -= std::min(volumeNeeded, resting->getRemainingSize());
        if (volumeNeeded == 0)
          return true;
      }
    }
    return false;
  }

  Trades match(OrderId const &orderId, Side const &side, Price const &price,
               Size &volumeRemaining, const auto &onRemove)
  {
    Trades matches;

    for (auto level = levels_.begin();
         level != levels_.end() && volumeRemaining > 0;)
    {
      if (price != MARKET_PRICE && comp_(price, level->price_))
        break;

      auto &orders = level->orders_;

      for (auto ord = orders.begin();
           ord != orders.end() && volumeRemaining > 0;)
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

        TradeData incomingData{orderId, level->price_, tradeSize};
        TradeData restingData{resting->getOrderId(), level->price_, tradeSize};

        if (side == Side::Buy)
        {
          matches.emplace_back(incomingData, restingData);
        }
        else
        {
          matches.emplace_back(restingData, incomingData);
        }

        volumeRemaining -= tradeSize;
        level->size_ -= tradeSize;
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

      if (orders.empty())
      {
        level = levels_.erase(level);
      }
      else
      {
        ++level;
      }
    }
    return matches;
  }

  void add(OrderPointer order)
  {
    Price orderPrice = order->getPrice();

    auto it = std::lower_bound(
        levels_.begin(), levels_.end(), orderPrice,
        [&](const PriceLevel<OrderContainer> &level, Price price)
        { return comp_(level.price_, price); });

    if (it != levels_.end() && it->price_ == orderPrice)
    {
      it->size_ += order->getRemainingSize();
      it->orders_.insert(order);
    }
    else
    {
      it = levels_.emplace(it, orderPrice);
      it->orders_.insert(order);
      it->size_ += order->getRemainingSize();
    }
  }

  void cancel(OrderPointer order)
  {
    Price orderPrice = order->getPrice();

    auto it = std::lower_bound(
        levels_.begin(), levels_.end(), orderPrice,
        [&](const PriceLevel<OrderContainer> &level, Price price)
        { return comp_(level.price_, price); });

    if (it != levels_.end() && it->price_ == orderPrice)
    {
      it->size_ -= order->getRemainingSize();
      it->orders_.erase(order);

      if (it->orders_.empty())
      {
        levels_.erase(it);
      }
    }
  }

  std::list<PriceLevel<OrderContainer>>::iterator begin()
  {
    return levels_.begin();
  }

  std::list<PriceLevel<OrderContainer>>::iterator end()
  {
    return levels_.end();
  }

  std::list<PriceLevel<OrderContainer>>::const_iterator begin() const
  {
    return levels_.begin();
  }

  std::list<PriceLevel<OrderContainer>>::const_iterator end() const
  {
    return levels_.end();
  }

private:
  std::list<PriceLevel<OrderContainer>> levels_;
  Compare comp_;
};
