#include <deque>
#include <iterator>
#include <list>
#include <vector>

#include "types.h"
#include "order.h"

struct ListOrderPolicy
{
    using OrderContainer = std::list<OrderPointer>;

    OrderContainer orders_;

    void add(OrderPointer order);

    void cancel(OrderPointer order);

    OrderPointer front();

    bool empty();

    OrderContainer::iterator begin();
    OrderContainer::iterator end();

    OrderContainer::const_iterator cbegin();
    OrderContainer::const_iterator cend();

    OrderContainer::iterator erase(OrderContainer::iterator);
};

struct DequeOrderPolicy
{
    using OrderContainer = std::deque<OrderPointer>;

    OrderContainer orders_;

    void add(OrderPointer order);

    void cancel(OrderPointer order);

    OrderPointer front();

    bool empty();

    OrderContainer::iterator begin();
    OrderContainer::iterator end();

    OrderContainer::const_iterator cbegin();
    OrderContainer::const_iterator cend();

    OrderContainer::iterator erase(OrderContainer::iterator);
};

struct VectorOrderPolicy
{
    using OrderContainer = std::vector<OrderPointer>;
    
    OrderContainer orders_;

    void add(OrderPointer order);

    void cancel(OrderPointer order);

    OrderPointer front();

    bool empty();

    OrderContainer::iterator begin();
    OrderContainer::iterator end();

    OrderContainer::const_iterator cbegin();
    OrderContainer::const_iterator cend();

    OrderContainer::iterator erase(OrderContainer::iterator);
};
