/*

Module for managing price levels in an order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

*/

// * dereference 
// . access
// -> is *.

#include "PriceLevel.h"
#include <iostream>



BasicOrder::BasicOrder(int id, int qty) {
    this->id = id;
    this->qty = qty;
}

PriceLevel::PriceLevel(float price, std::string type) {
    this->price = price;
    this->type = type;
}

BasicOrder PriceLevel::simple_list() {
    // todo
    return BasicOrder(0, 0);
}

std::string PriceLevel::getType() const {
    return type;
}

int PriceLevel::getPrice() const {
    return price;
}

bool PriceLevel::empty() const {
    if (head == nullptr) {
        return true;
    }
    return false;
}

bool PriceLevel::addOrder(int id, int qty) {
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

BasicOrder PriceLevel::popOrder() {
    if (head == nullptr) throw std::runtime_error("No orders to pop");

    OrderNode* el = head;
    head = head->right;
    
    if (head != nullptr) head->left = nullptr;

    BasicOrder retval = BasicOrder(el->id, el->qty);

    dict.erase(el->id);
    delete el;

    return retval;
}

BasicOrder PriceLevel::peekOrder() {
    if (head == nullptr) throw std::runtime_error("No orders to peek");

    return BasicOrder(head->id, head->qty);
}

BasicOrder PriceLevel::cancelOrder(int id) {
    if (!dict.count(id)) throw std::runtime_error("No order to cancel");

    OrderNode* node = dict[id];
    OrderNode* rnode = node->right;
    OrderNode* lnode = node->left;

    if (lnode) lnode->right = rnode;
    if (rnode) rnode->left = lnode;
    if (head == node) {
        head = node->right;
        if (head) head->left = nullptr;
    }
    if (tail == node) {
        tail = node->left;
        if (tail) tail->left = nullptr;
    }

    BasicOrder retval = BasicOrder(node->id, node->qty);

    delete node;

    return retval;

}

bool PriceLevel::decrementByID(int id, int amt) {
    if (dict.count(id)) {
        OrderNode* node = dict[id];
        node->qty -= amt;
        return true;
    }
    return false;
}

/* int main() {
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


} */


