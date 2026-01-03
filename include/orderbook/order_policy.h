#pragma once

#include <deque>
#include <iterator>
#include <list>
#include <unordered_map>
#include <vector>

#include "orderbook/order.h"
#include "orderbook/types.h"

struct ListOrderPolicy
{
  using OrderContainer = std::list<OrderPointer>;

  OrderContainer orders_;
  std::unordered_map<OrderId, OrderContainer::iterator> orderPosition_;

  ListOrderPolicy() : orders_{}, orderPosition_{} {}

  void insert(OrderPointer order)
  {
    orderPosition_[order->getOrderId()] = orders_.insert(orders_.end(), order);
  }

  OrderContainer::iterator erase(OrderContainer::iterator it)
  {
    auto order = *it;
    auto next = orders_.erase(it);
    orderPosition_.erase(order->getOrderId());
    return next;
  }

  OrderContainer::iterator erase(OrderPointer order)
  {
    auto next = orders_.erase(orderPosition_[order->getOrderId()]);
    orderPosition_.erase(order->getOrderId());
    return next;
  }

  auto size() const { return orders_.size(); }

  OrderPointer front() const { return orders_.front(); }

  bool empty() const { return orders_.empty(); }

  OrderContainer::iterator begin() { return orders_.begin(); }

  OrderContainer::iterator end() { return orders_.end(); }

  OrderContainer::const_iterator begin() const { return orders_.begin(); }

  OrderContainer::const_iterator end() const { return orders_.end(); }
};

struct DequeOrderPolicy
{
  using OrderContainer = std::deque<OrderPointer>;

  OrderContainer orders_;

  DequeOrderPolicy() : orders_{} {}

  void insert(OrderPointer order) { return orders_.push_back(order); }

  OrderContainer::iterator erase(OrderContainer::iterator it)
  {
    return orders_.erase(it);
  }

  OrderContainer::iterator erase(OrderPointer order)
  {
    return orders_.erase(std::remove(orders_.begin(), orders_.end(), order),
                         orders_.end());
  }

  auto size() const { return orders_.size(); }

  OrderPointer front() { return orders_.front(); }

  bool empty() { return orders_.empty(); }

  OrderContainer::iterator begin() { return orders_.begin(); }

  OrderContainer::iterator end() { return orders_.end(); }

  OrderContainer::const_iterator begin() const { return orders_.begin(); }

  OrderContainer::const_iterator end() const { return orders_.end(); }
};

struct VectorOrderPolicy
{
  using OrderContainer = std::vector<OrderPointer>;

  OrderContainer orders_;

  VectorOrderPolicy() : orders_{} {}

  void insert(OrderPointer order) { orders_.push_back(order); }

  OrderContainer::iterator erase(OrderContainer::iterator it)
  {
    return orders_.erase(it);
  }

  OrderContainer::iterator erase(OrderPointer order)
  {
    return orders_.erase(std::remove(orders_.begin(), orders_.end(), order),
                         orders_.end());
  }

  auto size() const { return orders_.size(); }

  OrderPointer front() { return orders_.front(); }

  bool empty() { return orders_.empty(); }

  OrderContainer::iterator begin() { return orders_.begin(); }

  OrderContainer::iterator end() { return orders_.end(); }

  OrderContainer::const_iterator begin() const { return orders_.begin(); }

  OrderContainer::const_iterator end() const { return orders_.end(); }
};
