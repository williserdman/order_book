/*

Headers for resting order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

*/

#include "PriceLevel.h"
#include <queue>
#include <tuple>

using namespace std;

struct PQOrder {
    int price;
    int qty;
    PQOrder(int p, int q);
};

class OrderBook {

private:
    priority_queue<std::tuple<int, PriceLevel*> > bids;
    priority_queue<std::tuple<int, PriceLevel*> > asks;

    map<int, PriceLevel*> bmap;
    map<int, PriceLevel*> amap;

    map<int, PriceLevel*> IDToPL;

public:

    bool addOrder(int orderID, string side, int price, int qty); 
    priority_queue<tuple<int, PriceLevel*> > getBids() const;
    priority_queue<tuple<int, PriceLevel*> > getAsks() const;
    map<int, PriceLevel*> getAMAP() const;
    map<int, PriceLevel*> getBMAP() const;

    PQOrder bestBid();

    PQOrder bestAsk();

    bool cancelOrder(int id);

    ~OrderBook();

};
