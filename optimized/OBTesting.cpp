
#include "OrderBookMatching.h"
#include <map>
#include <list>
#include <iostream>

struct Order
{
    int id;
    std::string side;
    float price;
    int qty;
    std::string type;
    int cancel_id;
    Order(int id, std::string side, float price, std::string type, int qty, int cancel_id) {
        this->id = id;
        this->side = side;
        this->type = type;
        this->price = price;
        this->qty = qty;
        this->cancel_id = cancel_id;
    }
};

struct BookFreeze {
    Ledger ledger;
    OBPrint book;
    BookFreeze(Ledger l, OBPrint b) {
        ledger = l;
        book = b;
    }
    bool equals(BookFreeze other) {
        if (ledger == other.ledger && book == other.book) return true;
        return false;
    }
};


BookFreeze matchTradesAndReturnBook(std::vector<Order> orders) {
    OrderBookMatching ob = OrderBookMatching();
    for (int i=0; i < orders.size(); i++) {
        Order o = orders[i];
        if (o.type == "CANCEL") {
            ob.cancelOrder(o.cancel_id);
        } else {
            ob.addOrder(o.id, o.side, o.price, o.qty, o.type);
        }
    }

    return BookFreeze(ob.getLedger(), ob.getPrintBook());
}

void matchTradesAndPrintBook(std::list<Order> orders) {
    OrderBookMatching ob = OrderBookMatching();
    for (Order o : orders) {
        if (o.type == "CANCEL") {
            ob.cancelOrder(o.cancel_id);
        } else {
            ob.addOrder(o.id, o.side, o.price, o.qty, o.type);
        }
    }

    std::cout << ob.printBook() << std::endl;
    std::cout << ob.printLedger() << std::endl;
}

void testFIFOPartialFill() {
    std::list<Order> orders;

    orders.push_back(
        Order(1, "SELL", 101.0f, "LIMIT", 10, 0)
    );
    orders.push_back(
        Order(2, "SELL", 101.0f, "LIMIT", 5, 0)
    );
    orders.push_back(
        Order(3, "BUY", 102.0f, "LIMIT", 12, 0)
    );
    orders.push_back(
        Order(4, "BUY", 101.0f, "LIMIT", 4, 0)
    );

    matchTradesAndPrintBook(orders);
}

void testMarketBuyAgainstSingleAsk() {
    std::list<Order> orders;

    orders.push_back(
        Order(10, "SELL", 105.0f, "LIMIT", 5, 0)
    );
    orders.push_back(
        Order(11, "BUY", 0.f, "MARKET", 3, 0)
    );

    matchTradesAndPrintBook(orders);
}

void testMarketOrderExhaustBook() {
    std::list<Order> orders;

    orders.push_back(
        Order(20, "SELL", 103.0f, "LIMIT", 2, 0)
    );
    orders.push_back(
        Order(21, "BUY", 0.f, "MARKET", 5, 0)
    );

    matchTradesAndPrintBook(orders); 
}

void testLimitOrderRestWhenNoLiquidity() {
    std::list<Order> orders;

    orders.push_back(
        Order(30, "BUY", 100.0f, "LIMIT", 4, 0)
    );

    matchTradesAndPrintBook(orders); 
}

void testMultiplePriceLevels() {
    std::list<Order> orders;

    orders.push_back(
        Order(40, "SELL", 102.0f, "LIMIT", 3, 0)
    );
    orders.push_back(
        Order(41, "SELL", 101.f, "LIMIT", 2, 0)
    );
    orders.push_back(
        Order(42, "BUY", 103.f, "LIMIT", 5, 0)
    );

    matchTradesAndPrintBook(orders); 
}

void testFIFOWithPartialAndNewEntry() {
    std::list<Order> orders;

    orders.push_back(
        Order(50, "SELL", 100.0f, "LIMIT", 5, 0)
    );
    orders.push_back(
        Order(51, "SELL", 100.f, "LIMIT", 5, 0)
    );
    orders.push_back(
        Order(52, "BUY", 100.f, "LIMIT", 6, 0)
    );
    orders.push_back(
        Order(53, "BUY", 100.f, "LIMIT", 2, 0)
    );

    matchTradesAndPrintBook(orders); 
}

void testCancelOrder() {
    {
    std::list<Order> orders;

    orders.push_back(
        Order(60, "SELL", 100.0f, "LIMIT", 5, 0)
    );
    orders.push_back(
        Order(61, "CANCEL", 105.f, "CANCEL", 5, 60)
    );
    orders.push_back(
        Order(62, "BUY", 105.f, "LIMIT", 3, 0)
    );

    matchTradesAndPrintBook(orders); 
}
}


#include <chrono>
#include <random>
#include <algorithm>
    

int main() {
    std::cout << "running" << std::endl;
    testFIFOPartialFill();
    testMarketBuyAgainstSingleAsk();
    testMarketOrderExhaustBook();
    testLimitOrderRestWhenNoLiquidity();
    testMultiplePriceLevels();
    testFIFOWithPartialAndNewEntry();
    testCancelOrder();
}