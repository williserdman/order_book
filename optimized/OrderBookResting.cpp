/*

Resting order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

*/

#include "PriceLevel.h"
#include <queue>
#include <tuple>
#include <iostream>

using namespace std;

struct PQOrder {
    int price;
    int qty;
    PQOrder(int p, int q) {
        price = p;
        qty = q;
    }
};

class OrderBook {

private:
    priority_queue<std::tuple<int, PriceLevel*> > bids;
    priority_queue<std::tuple<int, PriceLevel*> > asks;

    map<int, PriceLevel*> bmap;
    map<int, PriceLevel*> amap;

    map<int, PriceLevel*> IDToPL;

public:

    bool addOrder(int orderID, string side, int price, int qty) {
        if (side == "BUY") {
            if (!bmap.count(price)) {
                bmap[price] = new PriceLevel(price, "BUY");
                bids.push(
                    make_tuple(price, bmap[price])
                );
            }
            bmap[price]->addOrder(orderID, qty);
            IDToPL[orderID] = bmap[price];
            return true;
        } else if (side == "SELL")
        {
            if (!amap.count(price)) {
                amap[price] = new PriceLevel(price, "SELL");
                asks.push(
                    make_tuple(-price, amap[price])
                );
            }
            amap[price]->addOrder(orderID, qty);
            IDToPL[orderID] = amap[price];
            return true;
        }

        return false;
    }
    
    priority_queue<tuple<int, PriceLevel*> > getBids() const {
        return bids;
    }
    priority_queue<tuple<int, PriceLevel*> > getAsks() const {
        return asks;
    }
    map<int, PriceLevel*> getAMAP() const {
        return amap;
    }
    map<int, PriceLevel*> getBMAP() const {
        return bmap;
    }

    PQOrder bestBid() {
        if (bids.size() < 1) throw runtime_error("No resting bids");

        tuple<int, PriceLevel*> x = bids.top();

        BasicOrder bo = std::get<1>(x)->peekOrder();
        
        return PQOrder(std::get<0>(x), bo.qty);
    }

    PQOrder bestAsk() {
       if (asks.size() < 1) throw runtime_error("No resting asks");

       tuple<int, PriceLevel*> x = asks.top();

       BasicOrder bo = std::get<1>(x)->peekOrder();

       return PQOrder(-std::get<0>(x), bo.qty);
    }

    bool cancelOrder(int id) {
        if (!IDToPL.count(id)) throw runtime_error("Cancel order invalid");

        PriceLevel* pl = IDToPL[id];
        pl->cancelOrder(id);
        IDToPL.erase(id);


        try {
            pl->peekOrder();
        }
        catch(const exception& e) {
            int price = pl->getPrice();
            string type = pl->getType();
            if (type == "BUY") {
                priority_queue<tuple<int, PriceLevel*> > newBids;
                while (!bids.empty()) {
                    auto [pq_price, pq_pl] = bids.top();
                    bids.pop();
                    if (pq_price != price) {
                        newBids.push(make_tuple(pq_price, pq_pl));
                    } else {
                        bmap.erase(price);
                        delete pq_pl;
                    }
                }
                bids = newBids;
            } else {
                while (!asks.empty()) {
                    priority_queue<tuple<int, PriceLevel*> > newBids;
                    auto [inv_price, pl] = bids.top();
                    bids.pop();
                    if (-inv_price != price) {
                        newBids.push(make_tuple(
                            inv_price, pl
                        ));
                    } else {
                        bmap.erase(price);
                        delete pl;
                    }
                    bids = newBids;
                }
            }
            return true;
        }
        return false;

    }


};

int main() {
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


}
