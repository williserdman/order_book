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
    PQOrder();
    PQOrder(int p, int q);
};

class OrderBook {

private:
    priority_queue<std::tuple<int, PriceLevel*> > bids;
    priority_queue<std::tuple<int, PriceLevel*> > asks;

    std::unordered_map<int, PriceLevel*> bmap;
    std::unordered_map<int, PriceLevel*> amap;

    std::unordered_map<int, PriceLevel*> IDToPL;

public:

    bool addOrder(int orderID, string side, float price, int qty); 
    priority_queue<tuple<int, PriceLevel*> >* getBids();
    priority_queue<tuple<int, PriceLevel*> > getBidsValues();
    priority_queue<tuple<int, PriceLevel*> > getAsksValues();
    void setBids(priority_queue<tuple<int, PriceLevel*> > b);
    void setAsks(priority_queue<tuple<int, PriceLevel*> > a);
    priority_queue<tuple<int, PriceLevel*> >* getAsks();
    std::unordered_map<int, PriceLevel*> getAMAP() const;
    std::unordered_map<int, PriceLevel*> getBMAP() const;
    void removeFromAMAP(float price);
    void removeFromBMAP(float price);
    void idFullySold(int id);

    PQOrder bestBid();

    PQOrder bestAsk();

    bool cancelOrder(int id);

    ~OrderBook();

};
