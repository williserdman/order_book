

#include "OrderBookMatching.h"
#include <iostream>


OBPrint OrderBookMatching::getPrintBook() {
    OBPrint myMap;
    myMap["bids"] = getPrintBids();
    myMap["asks"] = getPrintAsks();
    return myMap;
}

std::string OrderBookMatching::printBook() {
    OBPrint x = getPrintBook();

    std::string s = "";
    

    for (const auto& pair : x) {
        s += pair.first + "\n";
        // std::cout << pair.second.size() << std::endl;
        for (int i = 0; i < (int) pair.second.size(); i++) {
            // std::cout << 1 << std::endl;
            // std::cout << std::to_string(std::get<0>(pair.second[i])) << std::endl;
            s += std::to_string(std::get<0>(pair.second[i])) + ": ";
            PLSimpleList t = std::get<1>(pair.second[i]);
            for (int j = 0; j < (int) t.size(); j++) {
                s += "(";
                s += std::to_string(std::get<0>(t[j]));
                s += ",";
                s += std::to_string(std::get<1>(t[j]));
                s += ")";
                if (j != t.size()-1) {
                    s += ", ";
                } else {
                    s += "\n";
                }
            }
        }
    }
    // std::cout << s << std::endl;
    return s;
}
InnerBookPrintList OrderBookMatching::getPrintAsks() {
    InnerBookPrintList out;
    priority_queue<tuple<int, PriceLevel*> > nBids;
    priority_queue<tuple<int, PriceLevel*> > curr = OrderBook::getAsks();
    while (!curr.empty()) {
        auto el = curr.top();
        curr.pop();
        nBids.push(el);
        // std::cout << -std::get<0>(el) << std::endl;

        out.push_back(
            std::make_tuple(-std::get<0>(el), std::get<1>(el)->simpleList())
        );
    }
    return out;
}
InnerBookPrintList OrderBookMatching::getPrintBids() {
    InnerBookPrintList out;
    priority_queue<tuple<int, PriceLevel*> > nBids;
    priority_queue<tuple<int, PriceLevel*> > curr = OrderBook::getBids();
    while (!curr.empty()) {
        auto el = curr.top();
        curr.pop();
        nBids.push(el);

        out.push_back(
            std::make_tuple(std::get<0>(el), std::get<1>(el)->simpleList())
        );
    }
    return out;
}


void OrderBookMatching::addOrder(int orderID, std::string side, float price, int qty, std::string type) {
    if (type == "MARKET") price = INFINITY;

    if (side == "BUY") {
        bool accepts = true;
        PQOrder m;

        try {
            m = OrderBook::bestAsk();
        } catch (const std::exception&) {
            accepts = false;
        }

        while (qty > 0 && accepts) {
            if (m.price <= price) {
                float p;
                int q;
                int i;

                if (m.qty <= qty) {
                    auto [p, q, i] = OrderBookMatching::popAsks();
                    try {
                        m = OrderBook::bestAsk();
                    } catch (const std::exception&) {
                        accepts = false;
                    }
                } else {
                    auto [p, q, i] = OrderBookMatching::partialBuy(qty);
                }
                qty -= q;
            } else {
                accepts = false;
            }
        }
        if (qty > 0) {
            // std::cout << "added" << std::endl;
            if (type != "MARKET") {
                OrderBook::addOrder(orderID, side, price, qty);
            }
        }
    } else {
        bool accepts = true;
        PQOrder m;

        try {
            m = OrderBook::bestBid();
        } catch (const std::exception&) {
            accepts = false;
        }

        while (qty > 0 && accepts) {
            if (m.price >= price) {
                float p;
                int q;
                int i;

                if (m.qty <= qty) {
                    auto [p, q, i] = OrderBookMatching::popBids();
                    try {
                        m = OrderBook::bestBid();
                    } catch (const std::exception&) {
                        accepts = false;
                    }
                } else {
                    auto [p, q, i] = OrderBookMatching::partialSell(qty);
                }
                qty -= q;
            } else {
                accepts = false;
            }
        }
        if (qty > 0) {
            // std::cout << "added" << std::endl;
            if (type != "MARKET") {
                OrderBook::addOrder(orderID, side, price, qty);
            }
        }
    }
}

MMTransaction OrderBookMatching::popAsks() {
    if (OrderBook::getAsks().empty()) throw std::runtime_error("No asks willing to buy");

    auto [price, PL] = OrderBook::getAsks().top();
    BasicOrder bo = PL->popOrder();

    if (PL->empty()) {
        OrderBook::getAsks().pop();
        delete PL;
    }

    return std::make_tuple(-price, bo.qty, bo.id);
}
MMTransaction OrderBookMatching::popBids() {
    if (OrderBook::getBids().empty()) throw std::runtime_error("No bids to sell to");

    auto [price, PL] = OrderBook::getBids().top();
    BasicOrder bo = PL->popOrder();

    if (PL->empty()) {
        OrderBook::getBids().pop();
        delete PL;
    }

    return std::make_tuple(price, bo.qty, bo.id);
}
MMTransaction OrderBookMatching::partialBuy(int amt) {
    if (OrderBook::getAsks().empty()) throw std::runtime_error("No bids to sell to");

    auto [price, PL] = OrderBook::getAsks().top();
    BasicOrder bo = PL->peekOrder();

    PL->decrementByID(bo.id, amt);

    return std::make_tuple(-price, bo.qty, bo.id);
}
MMTransaction OrderBookMatching::partialSell(int amt) {
    if (OrderBook::getBids().empty()) throw std::runtime_error("No bids to sell to");

    auto [price, PL] = OrderBook::getBids().top();
    BasicOrder bo = PL->peekOrder();

    PL->decrementByID(bo.id, amt);

    return std::make_tuple(price, bo.qty, bo.id);
}



void OrderBookMatching::addTransaction(int takerID, int makerID, float price, int qty) {
    getLedger().push_back(std::make_tuple(takerID, makerID, price, qty));
}

Ledger OrderBookMatching::getLedger() const {
    return ledger;
}

int main() {
    //std::cout << "entry" << std::endl;
    std::vector<std::tuple<int, int, float, int> > expectedTrades = {
        std::make_tuple(3, 1, 101., 10),
        std::make_tuple(3, 2, 101., 2),
        std::make_tuple(4, 2, 101., 3)
    };
    //std::cout << "created" << std::endl;

    OrderBookMatching ob = OrderBookMatching();
    
    //std::cout << "obc" << std::endl;

    ob.addOrder(1, "SELL", 101, 10, "LIMIT");
    ob.addOrder(2, "SELL", 101, 5, "LIMIT");

    std::cout << ob.printBook() << std::endl;

    ob.addOrder(3, "BUY", 102, 12, "LIMIT");
    ob.addOrder(4, "BUY", 101, 4, "LIMIT");

    //std::cout << "ob ops" << std::endl;

    std::cout << ob.printBook() << std::endl;
}