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
    head = tail = nullptr;
}

PLSimpleList PriceLevel::simpleList() {
    // todo
    PLSimpleList t;
    auto curr = head;
    while (curr) {
        t.push_back(
            std::make_tuple(curr->id, curr->qty)
        );
        curr = curr->right;
    }
    return t;
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
    //std::cerr << "PL " << price << ": " << "dict.size() = " << dict.size() << "; from addOrder" << std::endl;

    auto it = dict.find(id);
    if (it != dict.end()) return false;

    OrderNode* me = new OrderNode();

    me->id = id;
    me->qty = qty;
    me->right = nullptr;
    me->left = nullptr;

    if (head != nullptr) {
        me->left = tail;
        tail->right = me;
        tail = me;
    } else {
        head = tail = me;
    }

    //std::cout << id << std::endl;

    dict.emplace(id, me);
    // std::cout << "added " << me->id << ", " << me->qty << std::endl;

    return true;
}

BasicOrder PriceLevel::popOrder() {
    if (head == nullptr) throw std::runtime_error("No orders to pop");

    OrderNode* el = head;
    head = head->right;
    
    if (head) head->left = nullptr;
    else tail = nullptr;

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
    //std::cerr << "PL " << price << ": " << "dict.size() = " << dict.size() << "; from cancelOrder" << std::endl;
    if (dict.empty()) throw std::runtime_error("No order to cancel");
    auto it = dict.find(id);
    if (it == dict.end()) throw std::runtime_error("No order to cancel");
    OrderNode* node = it->second;

    //std::cout << node->id << std::endl;
    

    OrderNode* rnode = node->right;
    OrderNode* lnode = node->left;

    // unlink node cleanly from list
    if (lnode) lnode->right = rnode;
    if (rnode) rnode->left = lnode;

    if (head == node) head = rnode;
    if (tail == node) tail = lnode;

    // ensure new head/tail have null neighbors
    if (head) head->left = nullptr;
    if (tail) tail->right = nullptr;

    BasicOrder retval = BasicOrder(node->id, node->qty);

    dict.erase(id);
    delete node;

    //std::cerr << "dict.size() = " << dict.size() << std::endl;

    return retval;
}

bool PriceLevel::decrementByID(int id, int amt) {
    auto it = dict.find(id);
    if (it == dict.end()) return false;
    OrderNode* node = it->second;
    node->qty -= amt;
    return true;
}

PriceLevel::~PriceLevel() {
    OrderNode* curr = head;
    OrderNode* next = curr;
    while (curr) {
        next = curr->right;
        //dict.erase(curr->id);
        delete curr;
        curr = next;
    }
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


