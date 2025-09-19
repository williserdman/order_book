#include <iostream>
#include <OrderBook.h>

bool Order::operator < (const Order& other) const {
   if (price < other.getPrice()) {
        return true;
    } else if (price == other.getPrice()) {
        return (timeCreated < other.getCreationTime());
    }
    return false; 
}

int Order::getPrice() const {
    return price;
}

int Order::getCreationTime() const {
    return timeCreated;
}

Order::Order() {
    id = 0;
    price = 0;
    quantity = 0;
    timeCreated = 0;
    type = "";
    side = "";
}

OrderPool::OrderPool(int num) {
    for (int i = 0; i < num; i++) {
        orderPool.push_back(new Order());
        myMap.emplace(orderPool[i], i);
    }
}

OrderPool::~OrderPool() {
    for (Order* o : orderPool) {
        delete o;
        o = nullptr;
    }
}

Order* OrderPool::allocateOrder() {
    if (head == orderPool.size()) {
        orderPool.push_back(new Order());
        return orderPool[head++];
    } else {
        return orderPool[head++];
    }
}

bool OrderPool::releaseOrder(Order* o) {
    auto it = myMap.find(o);
    if (it != myMap.end()) {
        int idx = it->second;
        --head;
        std::swap(orderPool[idx], orderPool[head]);
        return true;
    }
    return false;
}

OrderBook::OrderBook(int poolSize): pool(poolSize) {};

bool OrderBook::timeStep() {
    time += 1;
    return true;
}

bool OrderBook::popBids() {
    if (bids.empty()) return false;

    bids.pop();
    return true;
}

bool OrderBook::popAsks() {

}

bool OrderBook::partialBuy() {}
bool OrderBook::partialSell() {}

bool OrderBook::addToLedger(Transaction &t) {}

std::string OrderBook::getAsks() const {}
std::string OrderBook::getBids() const {}

bool OrderBook::addOrder(int& id, int& price, int& quantity, std::string& type, std::string& side) {}
bool OrderBook::cancelOrder(int& id) {}

Order OrderBook::topBids() const {}
Order OrderBook::topAsks() const {}

std::string OrderBook::getBook() const {}
std::string OrderBook::getLedger() const {}

OrderBook::~OrderBook() {}
