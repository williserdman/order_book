

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
        for (int i = 0; i < (int) pair.second.size(); i++) {
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
    return s;
}
InnerBookPrintList OrderBookMatching::getPrintAsks() {
    InnerBookPrintList out;
    priority_queue<tuple<int, PriceLevel*> > nBids;
    priority_queue<tuple<int, PriceLevel*> > curr = OrderBook::getAsksValues();
    while (!curr.empty()) {
        auto el = curr.top();
        curr.pop();
        nBids.push(el);

        out.push_back(
            std::make_tuple(-std::get<0>(el), std::get<1>(el)->simpleList())
        );
    }
    return out;
}
InnerBookPrintList OrderBookMatching::getPrintBids() {
    InnerBookPrintList out;
    priority_queue<tuple<int, PriceLevel*> > nBids;
    priority_queue<tuple<int, PriceLevel*> > curr = OrderBook::getBidsValues();
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
                    std::tie(p, q, i) = OrderBookMatching::popAsks();
                    addTransaction(orderID, i, p, q);
                    try {
                        m = OrderBook::bestAsk();
                    } catch (const std::exception&) {
                        accepts = false;
                    }
                } else {
                    std::tie(p, q, i) = OrderBookMatching::partialBuy(qty);
                    addTransaction(orderID, i, p, q);
                }
                qty -= q;
            } else {
                accepts = false;
            }
        }
        if (qty > 0) {
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
                    std::tie(p, q, i) = OrderBookMatching::popBids();
                    addTransaction(orderID, i, p, q);
                    try {
                        m = OrderBook::bestBid();
                    } catch (const std::exception&) {
                        accepts = false;
                    }
                } else {
                    std::tie(p, q, i) = OrderBookMatching::partialSell(qty);
                    addTransaction(orderID, i, p, q);
                }
                qty -= q;
            } else {
                accepts = false;
            }
        }
        if (qty > 0) {
            if (type != "MARKET") {

                // std::cout << OrderBookMatching::printBook() << std::endl;
                OrderBook::addOrder(orderID, side, price, qty);
            }
        }
    }
}

MMTransaction OrderBookMatching::popAsks() {
    if (OrderBook::getAsks()->empty()) throw std::runtime_error("No asks willing to buy");

    auto [price, PL] = OrderBook::getAsks()->top();
    BasicOrder bo = PL->popOrder();
    //std::cout << "popping ask" << std::endl;

    if (PL->empty()) {
        OrderBook::getAsks()->pop();
        delete PL;
        //std::cout << "del pl" << std::endl;
        OrderBook::removeFromDicts(price);
    }

    return std::make_tuple(-price, bo.qty, bo.id);
}
MMTransaction OrderBookMatching::popBids() {
    if (OrderBook::getBids()->empty()) throw std::runtime_error("No bids to sell to");

    auto [price, PL] = OrderBook::getBids()->top();
    BasicOrder bo = PL->popOrder();

    if (PL->empty()) {
        OrderBook::getBids()->pop();
        delete PL;
    }

    return std::make_tuple(price, bo.qty, bo.id);
}
MMTransaction OrderBookMatching::partialBuy(int amt) {
    if (OrderBook::getAsks()->empty()) throw std::runtime_error("No bids to sell to");

    auto [price, PL] = OrderBook::getAsks()->top();
    BasicOrder bo = PL->peekOrder();

    PL->decrementByID(bo.id, amt);

    return std::make_tuple(-price, bo.qty, bo.id);
}
MMTransaction OrderBookMatching::partialSell(int amt) {
    if (OrderBook::getBids()->empty()) throw std::runtime_error("No bids to sell to");

    auto [price, PL] = OrderBook::getBids()->top();
    BasicOrder bo = PL->peekOrder();

    PL->decrementByID(bo.id, amt);

    return std::make_tuple(price, bo.qty, bo.id);
}



void OrderBookMatching::addTransaction(int takerID, int makerID, float price, int qty) {
    ledger.push_back(std::make_tuple(takerID, makerID, price, qty));
}

Ledger OrderBookMatching::getLedger() const {
    return ledger;
}

std::string OrderBookMatching::printLedger() const {
    std::string build = "";
    for (int i = 0; i < ledger.size(); i++) {
        std::tuple<int, int, float, int> order = ledger[i];
        build += std::to_string(std::get<0>(order)) + " ";
        build += std::to_string(std::get<1>(order)) + " ";
        build += std::to_string(std::get<2>(order)) + " ";
        build += std::to_string(std::get<3>(order)) + "\n";
    }
    return build;
}

/* int main() {
    std::vector<std::tuple<int, int, float, int> > expectedTrades = {
        std::make_tuple(3, 1, 101., 10),
        std::make_tuple(3, 2, 101., 2),
        std::make_tuple(4, 2, 101., 3)
    };

    OrderBookMatching ob = OrderBookMatching();
    

    ob.addOrder(1, "SELL", 101, 10, "LIMIT");
    ob.addOrder(2, "SELL", 101, 5, "LIMIT");

    std::cout << ob.printBook() << std::endl;

    ob.addOrder(3, "BUY", 102, 12, "LIMIT");

    std::cout << ob.printBook() << std::endl;

    ob.addOrder(4, "BUY", 101, 4, "LIMIT");

    std::cout << ob.printBook() << std::endl;
} */