/*

Resting order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

*/

#include "OrderBookResting.h"
#include <iostream>

// DEV STUFF
using Clock = std::chrono::steady_clock;
using ns = std::chrono::nanoseconds;

using namespace std;

/* struct PQOrder {
    int price;
    int qty;
    PQOrder(int p, int q) {
        price = p;
        qty = q;
    }
}; */
PQOrder::PQOrder() {}
PQOrder::PQOrder(int p, int q) {
    price = p;
    qty = q;
}

bool OrderBook::addOrder(int orderID, string side, float price, int qty) {
    // std::cout << "adding order" << std::endl;
    //std::cout << "adding order: " << orderID << " price: " << price << " qty: " << qty << " side: " << side << std::endl;
    auto t0 = Clock::now();
    auto t1 = Clock::now();
    auto t2 = Clock::now();
    if (side == "BUY") {
        //std::cerr << orderID << " added to " << " buy side " << std::endl;
        //std::cout << "adding to buy order book" << std::endl;
        t0 = Clock::now();
        if (bmap.find(price) == bmap.end()) {
            //std::cout << "creating new" << std::endl;
            bmap.emplace(price, new PriceLevel(price, "BUY"));
            //std::cout << " price level just added: " << bmap[price]->getPrice() << std::endl;

            auto lvl_before = bmap[price];
            //std::cout << "addr before push: " << static_cast<void*>(lvl_before) << " price before: " << lvl_before->getPrice() << std::endl;

            bids.push(std::make_tuple(price, lvl_before));

            auto lvl_after = bmap[price];
            //std::cout << "addr after push: " << static_cast<void*>(lvl_after) << " price after: " << (lvl_after ? lvl_after->getPrice() : -1) << std::endl;
            /*             bids.push(
                make_tuple(price, bmap[price])
            ); */
        }
        t1 = Clock::now();
        //std::cout << "special adding to " << bmap[price]->getPrice() << std::endl;
        bmap[price]->addOrder(orderID, qty);
        t2 = Clock::now();
        IDToPL.emplace(orderID, bmap[price]);
        std::cout << "possible malloc operations: " << std::chrono::duration_cast<ns>(t1 - t0).count() << " other reqs " << std::chrono::duration_cast<ns>(t2 - t1).count() << std::endl;
        return true;
    } else if (side == "SELL") {
        //std::cerr << orderID << " added to " << " sell side " << std::endl;
        // std::cout << "adding to sell book" << std::endl;
        t0 = Clock::now();
        if (amap.find(price) == amap.end()) {
            //std::cout << " creating new " << std::endl;
            amap.emplace(price, new PriceLevel(price, "SELL"));
            //std::cout << " price level just added: " << amap[price]->getPrice() << std::endl;

            auto lvl_before = amap[price];
            //std::cout << "addr before push: " << static_cast<void*>(lvl_before) << " price before: " << lvl_before->getPrice() << std::endl;

            //bids.push(std::make_tuple(price, lvl_before));
            asks.push(
                std::make_tuple(-price, amap[price])
            );

            auto lvl_after = amap[price];
            //std::cout << "addr after push: " << static_cast<void*>(lvl_after) << " price after: " << (lvl_after ? lvl_after->getPrice() : -1) << std::endl;
            
        }
        t1 = Clock::now();
        //std::cout << "special adding to " << amap[price]->getPrice() << std::endl;
        amap[price]->addOrder(orderID, qty);
        t2 = Clock::now();
        IDToPL.emplace(orderID, amap[price]);
        std::cout << "possible malloc operations: " << std::chrono::duration_cast<ns>(t1 - t0).count() << " other reqs " << std::chrono::duration_cast<ns>(t2 - t1).count() << std::endl;
        return true;
    }

    return false;
}

priority_queue<tuple<int, PriceLevel*> >* OrderBook::getBids() {
    return &bids;
}
priority_queue<tuple<int, PriceLevel*> > OrderBook::getBidsValues() {
    return bids;
}
priority_queue<tuple<int, PriceLevel*> > OrderBook::getAsksValues() {
    return asks;
}
priority_queue<tuple<int, PriceLevel*> >* OrderBook::getAsks() {
    return &asks;
}

void OrderBook::setBids(priority_queue<tuple<int, PriceLevel*> > b) {
    bids = b;
}

void OrderBook::setAsks(priority_queue<tuple<int, PriceLevel*> > a) {
    asks = a;
}
std::unordered_map<int, PriceLevel*> OrderBook::getAMAP() const {
    return amap;
}
std::unordered_map<int, PriceLevel*> OrderBook::getBMAP() const {
    return bmap;
}

PQOrder OrderBook::bestBid() {
    if (bids.size() < 1) throw runtime_error("No resting bids");

    tuple<int, PriceLevel*> x = bids.top();

    BasicOrder bo = std::get<1>(x)->peekOrder();
    
    return PQOrder(std::get<0>(x), bo.qty);
}

PQOrder OrderBook::bestAsk() {
    if (asks.size() < 1) throw runtime_error("No resting asks");

    tuple<int, PriceLevel*> x = asks.top();

    BasicOrder bo = std::get<1>(x)->peekOrder();

    return PQOrder(-std::get<0>(x), bo.qty);
}

bool areFloatsEqual(float a, float b, float epsilon = std::numeric_limits<float>::epsilon()) {
    return std::fabs(a - b) <= epsilon;
}

bool OrderBook::cancelOrder(int id) {
    if (!IDToPL.count(id)) throw runtime_error("Cancel order invalid");
    //std::cout << "cancelling " << id << " from OBR" << std::endl;
    PriceLevel* pl = IDToPL[id];
    pl->cancelOrder(id);
    IDToPL.erase(id);

    if (pl->empty()) {
        int price = pl->getPrice();
        string type = pl->getType();
        if (type == "BUY") {
            priority_queue<tuple<int, PriceLevel*> > newBids;
            while (!bids.empty()) {
                auto [pq_price, pq_pl] = bids.top();
                bids.pop();
                if (!areFloatsEqual(pq_price, price)) {
                    newBids.push(make_tuple(pq_price, pq_pl));
                } else {
                    bmap.erase(price);
                    delete pq_pl;
                }
            }
            
            bids = newBids;
        } else {
            priority_queue<tuple<int, PriceLevel*> > newAsks;
            while (!asks.empty()) {
                auto [inv_price, pl] = asks.top();
                asks.pop();
                if (!areFloatsEqual(-inv_price, price)) {
                    newAsks.push(make_tuple(
                        inv_price, pl
                    ));
                } else {
                    amap.erase(price);
                    delete pl;
                }
            }
            asks = newAsks;
        }
        return true;
    }
    return false;

}

void OrderBook::removeFromBMAP(float price) {
    //std::cout << "erasing " << price << std::endl;
    bmap.erase(price);
    // erase the price level pointer that's in bmap 
}

void OrderBook::removeFromAMAP(float price) {
    amap.erase(price);
}

void OrderBook::idFullySold(int id) {
    IDToPL.erase(id);
}

OrderBook::~OrderBook() {
    //std::cout << "this got called" << std::endl;
    while (!bids.empty()) {
        auto [pq_price, pq_pl] = bids.top();
        delete pq_pl;
        bids.pop();
    }
    while (!asks.empty()) {
        auto [pq_price, pq_pl] = asks.top();
        delete pq_pl;   
        asks.pop();
    }
    bmap.clear();
    amap.clear();
    IDToPL.clear();
}


/* int main() {
    OrderBook ob = OrderBook();

    ob.addOrder(1, "BUY", 100, 10);
    ob.addOrder(2, "BUY", 101, 5);
    ob.addOrder(3, "SELL", 102, 20);
    ob.addOrder(4, "SELL", 103, 15);

    cout << "begin" << endl;

    PQOrder mine = ob.bestBid();
    cout << mine.price << ", " << mine.qty << endl;

    mine = ob.bestAsk();
    cout << mine.price << ", " << mine.qty << endl;

    ob.cancelOrder(2);

    mine = ob.bestBid();
    cout << mine.price << ", " << mine.qty << endl;
    mine = ob.bestAsk();
    cout << mine.price << ", " << mine.qty << endl;
} */

