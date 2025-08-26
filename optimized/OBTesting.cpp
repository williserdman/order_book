
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

int main() {
    std::cout << "running" << std::endl;
    testFIFOPartialFill();
}
