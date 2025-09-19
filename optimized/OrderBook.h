#include <string>
#include <queue> // Add this include for std::priority_queue
#include <set>


struct Order {
    int id;
    int price;
    int quantity;
    int timeCreated;
    std::string type;
    std::string side;

    Order();
    bool operator < (const Order& other) const;
    int getPrice() const;
    int getCreationTime() const;
};

class OrderPool {
    private:
        std::vector<Order*> orderPool;
        std::unordered_map<Order*, int> myMap;
        int head;
    
    public:
        OrderPool(int num);
        Order* allocateOrder();
        bool releaseOrder(Order*);
        ~OrderPool();
};

struct Transaction {
    int makerID;
    int takerID;
    int price;
    int quantity;
    int time;
};


class OrderBook {
    private:
        std::priority_queue<Order> bids;
        std::priority_queue<Order> asks;

        std::vector<Transaction> ledger;

        std::set<int> canceledOrders;

        OrderPool pool;

        long time = 0;
        
        bool popBids();
        bool popAsks();

        bool partialBuy();
        bool partialSell();

        bool addToLedger(Transaction &t);

        std::string getAsks() const;
        std::string getBids() const;

        bool timeStep();

    public:
        OrderBook(int poolSize);

        bool addOrder(int& id, int& price, int& quantity, std::string& type, std::string& side);
        bool cancelOrder(int& id);
        
        Order topBids() const;
        Order topAsks() const;

        std::string getBook() const;
        std::string getLedger() const;

        ~OrderBook();
};