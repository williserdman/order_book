/*

Module for managing price levels in an order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

*/


#include <string>
#include <map>
#include <iostream>

struct OrderNode {
    int id;
    int qty;
    OrderNode* right = nullptr;
    OrderNode* left = nullptr;
};

struct BasicOrder {
    int id;
    int qty;

    BasicOrder(int id, int qty) {
        this->id = id;
        this->qty = qty;
    }
};

class PriceLevel {

private:
    OrderNode* head = nullptr;
    OrderNode* tail = nullptr;
    std::map<int, OrderNode*> dict;
    float price;
    std::string type;

public:
    PriceLevel(int price, std::string type) {
        this->price = price;
        this->type = type;
    }

    BasicOrder* simple_list() {
        // todo
    }

    std::string getType() const {
        return type;
    }

    int getPrice() const {
        return price;
    }

    bool empty() const {
        if (head == nullptr) {
            return true;
        }
        return false;
    }

    bool addOrder(int id, int qty) {
        if (dict.count(id)) return false;

        OrderNode* me = new OrderNode();

        me->id = id;
        me->qty = qty;

        if (head != nullptr) {
            me->left = tail;
            tail->right = me;
            tail = me;
        } else {
            head = tail = me;
        }

        dict[id] = me;
        // std::cout << "added " << me->id << ", " << me->qty << std::endl;

        return true;
    }

    BasicOrder popOrder() {
        if (head == nullptr) throw std::runtime_error("No orders to pop");

        OrderNode* el = head;
        head = head->right;
        
        if (head != nullptr) head->left = nullptr;

        BasicOrder retval = BasicOrder(el->id, el->qty);

        dict.erase(el->id);
        delete el;

        return retval;
    }

    BasicOrder peekOrder() {
        if (head == nullptr) throw std::runtime_error("No orders to peek");

        return BasicOrder(head->id, head->qty);
    }

    BasicOrder cancelOrder(int id) {
        if (!dict.count(id)) throw std::runtime_error("No order to cancel");

        OrderNode* node = dict[id];
        OrderNode* rnode = node->right;
        OrderNode* lnode = node->left;

        if (lnode) lnode->right = rnode;
        if (rnode) rnode->left = lnode;
        if (head == node) {
            head = node->right;
            head->left = nullptr;
        }
        if (tail == node) {
            tail = node->left;
            tail->left = nullptr;
        }

        BasicOrder retval = BasicOrder(node->id, node->qty);

        delete node;

        return retval;

    }

    bool decrementByID(int id, int amt) {
        if (dict.count(id)) {
            OrderNode* node = dict[id];
            node->qty -= amt;
            return true;
        }
        return false;
    }

};

int main() {
    PriceLevel pl = PriceLevel(100, "test");

    pl.addOrder(1, 100);
    pl.addOrder(2, 200);
    pl.addOrder(3, 300);

    BasicOrder mine = pl.popOrder();
    std::cout << mine.id << ", " << mine.qty << std::endl;

    mine = pl.cancelOrder(3);
    std::cout << mine.id << ", " << mine.qty << std::endl;

    mine = pl.popOrder();
    std::cout << mine.id << ", " << mine.qty << std::endl;


}


