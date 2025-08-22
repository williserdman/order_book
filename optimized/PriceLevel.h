/*

Headers for managing price levels in an order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

*/


#include <string>
#include <map>

struct OrderNode {
    int id;
    int qty;
    OrderNode* right = nullptr;
    OrderNode* left = nullptr;
};

struct BasicOrder {
    int id;
    int qty;

    BasicOrder(int id, int qty);
};

class PriceLevel {

private:
    OrderNode* head = nullptr;
    OrderNode* tail = nullptr;
    std::map<int, OrderNode*> dict;
    float price;
    std::string type;

public:
    PriceLevel(float price, std::string type);

    BasicOrder simple_list();

    std::string getType() const;

    int getPrice() const;

    bool empty() const;

    bool addOrder(int id, int qty);

    BasicOrder popOrder();

    BasicOrder peekOrder();

    BasicOrder cancelOrder(int id);

    bool decrementByID(int id, int amt);

    ~PriceLevel();

};


