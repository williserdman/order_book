
#include "OrderBookMatching.h"
#include <map>
#include <list>
#include <iostream>

const int ID_START_VAL = 10;
const int PROB_LIMIT = 50;
const int PROB_MARKET = 30;
const int PROB_CANCEL = 20;
const int MAX_PRICE = 200;
const int MAX_QTY = 50;

struct Order
{
    int id;
    std::string side;
    float price;
    int qty;
    std::string type;
    int cancel_id;
    bool alive;
    Order() {}
    Order(int id, std::string side, float price, std::string type, int qty, int cancel_id) {
        this->id = id;
        this->side = side;
        this->type = type;
        this->price = price;
        this->qty = qty;
        this->cancel_id = cancel_id;
        alive = true;
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
            try {
                ob.cancelOrder(o.cancel_id);
            } catch (const exception& e) {
                std::cout << e.what() << std::endl;
            }
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
            // std::cout << o.cancel_id << std::endl;
            try {
                ob.cancelOrder(o.cancel_id);
            } catch (const exception& e) {
                std::cout << e.what() << std::endl;
            }
        } else {
            ob.addOrder(o.id, o.side, o.price, o.qty, o.type);
        }
    }

    std::cout << ob.printBook() << std::endl;
    std::cout << ob.printLedger() << std::endl;
}

std::list<int> seenThings;

void matchTradesToBook(std::list<Order>* orders, OrderBookMatching* ob) {
    for (Order o : *orders) {
        //std::cout << "ORDER: " << "id: " << o.id << " price: " << o.price << " quantity: " << o.qty << " type: " << o.type << " side " << o.side << " cancel id: " << o.cancel_id << std::endl;
        //std::cout << "BOOK STATUS: " << ob->printBook() << "\n\n";
        
        if (o.type == "CANCEL") {
            // std::cout << o.cancel_id << std::endl;
            try {
                ob->cancelOrder(o.cancel_id);
                /* seenThings.push_back(o.cancel_id);
                //std::cout << "elements" << std::endl;
                for (auto el : seenThings) {
                    //std::cout << el << std::endl;
                } */
            } catch (const exception& e) {
                //std::cout << e.what() << std::endl;
            }
        } else {
            ob->addOrder(o.id, o.side, o.price, o.qty, o.type);
            //std::cout << ob->printBook() << std::endl;
            //std::cout << ob->printLedger() << std::endl;
        }
        //std::cout << ob->printBook() << "\n\n";
    }
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



void benchmarkPerformance(int orderCount) {
    std::list<Order> orders;
    int current_order_id = ID_START_VAL;
    for (int i = 0; i < orderCount; i++) {
        Order o;
        int r = rand() % 101; // between 0 and 100
        int r_helper = rand() % 2;
        std::string side = (r_helper == 0) ? "BUY" : "SELL";
        int qty = (((int) rand() % MAX_QTY) + 2);

        if (r < PROB_LIMIT) {
            int price = (rand() % MAX_PRICE) + 2;

            o = Order(
                i,
                side,
                price,
                "LIMIT",
                qty,
                0
            );
        } else if (r < (PROB_LIMIT + PROB_MARKET)) {
            o = Order(
                i,
                side,
                0,
                "MARKET",
                qty,
                0
            );
        } else {
            if (i > 0) {
                int cancel_id = rand() % (i-1);
                o = Order(
                    i,
                    "CANCEL",
                    0,
                    "CANCEL",
                    0,
                    cancel_id
                );
            }
        }
        if (o.alive) {
            orders.push_back(o);
            // std::cout << o.id << " " << o.type << " " << o.side << " " << o.price << " " << o.qty << " " << o.cancel_id << std::endl;
        }
    }

    OrderBookMatching ob = OrderBookMatching();
    std::cout << "\n\n" << "starting run" << "\n\n";
    auto start = std::chrono::high_resolution_clock::now();

    matchTradesToBook(&orders, &ob);

    auto end = std::chrono::high_resolution_clock::now();

    //std::cout << ob.printBook() << std::endl;
    //std::cout << ob.printLedger() << std::endl;

    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start);
    std::cerr << "Processed " << orderCount << " orders in " << duration.count() << " nanoseconds. This is an average time of " << duration.count() / orderCount << " nanoseconds per order." << std::endl;
}
    

int main() {
    std::cout << "running" << std::endl;
    /* testFIFOPartialFill();
    testMarketBuyAgainstSingleAsk();
    testMarketOrderExhaustBook();
    testLimitOrderRestWhenNoLiquidity();
    testMultiplePriceLevels();
    testFIFOWithPartialAndNewEntry();
    testCancelOrder(); */

    benchmarkPerformance(1000000); // one million orders
}


