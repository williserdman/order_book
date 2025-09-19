#include <iostream>
#include <OrderBook.h>
#include <cmath>

bool Order::operator < (const Order& other) const {
    if (price < other.price) {
        if (side=="BUY") return true;
        if (side=="SELL") return false;
    } else if (price == other.price) {
        return (timeCreated < other.timeCreated);
    }
    if (side=="BUY") return false;
    if (side=="SELL") return true;
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

Transaction::Transaction(int mid, int tid, int p, int q, int t) {
    makerID = mid;
    takerID = tid;
    price = p;
    quantity = q;
    time = t;
}

OrderBook::OrderBook(int poolSize): pool(poolSize) {};

bool OrderBook::timeStep() {
    time += 1;
    return true;
}

bool OrderBook::popBids() {
    if (bids.empty()) return false;

    pool.releaseOrder(bids.top());
    bids.pop();
    return true;
}

bool OrderBook::popAsks() {
    if (asks.empty()) return false;

    pool.releaseOrder(asks.top());
    asks.pop();
    return true;
}

bool OrderBook::partialBuy(int amt) {
    asks.top()->quantity -= amt;
    return true;
}
bool OrderBook::partialSell(int amt) {
    bids.top()->quantity -= amt;
    return true;
}

bool OrderBook::addToLedger(Transaction &t) {
    ledger.push_back(t);
    return true;
}

std::string OrderBook::getAsks() const {}
std::string OrderBook::getBids() const {}

bool OrderBook::addOrder(int id, int price, int quantity, std::string& type, std::string& side) {
    if (type == "MARKET") price = INFINITY;

    if (side == "BUY") {
        Order* best;
        bool accepts = topAsks(best);

        while (quantity > 0 && accepts) {
            Order m = *best;
            if (m.price <= price) {
                int q;
                if (m.quantity <= quantity) {
                    q = m.quantity;
                    popAsks();
                    //addTransaction(orderID, i, p, q);
                    accepts = topAsks(best);
                } else {
                    q = quantity;
                    partialBuy(quantity);
                    //addTransaction(orderID, i, p, q);
                }
                quantity -= q;
            } else {
                accepts = false;
            }
        }
        if (quantity > 0) {
            if (type != "MARKET") {
                addRestingBid(id, price, quantity, type, side);
            }
        }
    } else {
        Order* best;
        bool accepts = topBids(best);

        while (quantity> 0 && accepts) {
            Order m = *best;
            if (m.price >= price) { // if maker is offering more or equal to what we are asking
                int q;

                if (m.quantity <= quantity) { // if maker is offering fewer or equal shares we take them all
                    q = m.quantity;
                    popBids();
                    //addTransaction(orderID, i, p, q);
                    accepts = topBids(best);
                } else {
                    q = quantity;
                    partialSell(quantity);
                    //addTransaction(orderID, i, p, q);
                }
                quantity -= q;
            } else {
                accepts = false;
            }
        }
        if (quantity > 0) {
            if (type != "MARKET") {
                addRestingAsk(id, price, quantity, type, side);
            }
        }
    }
}

bool OrderBook::cancelOrder(int id) {
    if (canceledOrders.find(id) != canceledOrders.end()) {
        return false;
    }
    canceledOrders.insert(id);
    return true;
}

bool OrderBook::topBids(Order*& best) {
    if (bids.empty()) return false;
    best = bids.top();
    while (canceledOrders.find(best->id) != canceledOrders.end()) { // if it's in canceled orders
        popBids();
        if (bids.empty()) return false;
        best = bids.top();
    }
    return true;
}
bool OrderBook::topAsks(Order*& best) {
    if (asks.empty()) return false;
    best = asks.top();
    while (canceledOrders.find(best->id) != canceledOrders.end()) {
        popAsks();
        if (asks.empty()) return false;
        best = asks.top();
    }
    return true;
}

std::string OrderBook::getBook() const {}
std::string OrderBook::getLedger() const {}

OrderBook::~OrderBook() {
    while (!bids.empty()) bids.pop();
    while (!asks.empty()) asks.pop();

    canceledOrders.clear();

    // No need to manually call the destructor or delete pool since it's not a pointer.
    // The OrderPool destructor will be called automatically when the OrderBook is destroyed.
}
