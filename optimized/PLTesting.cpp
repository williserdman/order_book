#include <iostream>
#include <stdexcept>
#include "PriceLevel.h"   // your header

void runTests() {
    PriceLevel pl(100.0f, "BID");

    std::cout << "=== Adding orders ===" << std::endl;
    pl.addOrder(1, 10);
    pl.addOrder(2, 20);
    pl.addOrder(3, 30);

    std::cout << "Initial simpleList: ";
    for (auto& [id, qty] : pl.simpleList()) {
        std::cout << "(" << id << "," << qty << ") ";
    }
    std::cout << "\n\n";

    std::cout << "=== Cancel once ===" << std::endl;
    try {
        auto order = pl.cancelOrder(2);
        std::cout << "Cancelled order id=" << order.id << " qty=" << order.qty << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error on cancel 2: " << e.what() << "\n";
    }

    std::cout << "List after first cancel: ";
    for (auto& [id, qty] : pl.simpleList()) {
        std::cout << "(" << id << "," << qty << ") ";
    }
    std::cout << "\n\n";

    std::cout << "=== Cancel twice ===" << std::endl;
    try {
        auto order = pl.cancelOrder(2);
        std::cout << "Cancelled order id=" << order.id << " qty=" << order.qty << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error on second cancel 2: " << e.what() << "\n";
    }

    std::cout << "=== Pop head ===" << std::endl;
    try {
        auto order = pl.popOrder();
        std::cout << "Popped order id=" << order.id << " qty=" << order.qty << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error on pop: " << e.what() << "\n";
    }

    std::cout << "Remaining simpleList: ";
    for (auto& [id, qty] : pl.simpleList()) {
        std::cout << "(" << id << "," << qty << ") ";
    }
    std::cout << "\n\n";

    std::cout << "=== Cancel non-existent ===" << std::endl;
    try {
        pl.cancelOrder(999);
    } catch (const std::exception& e) {
        std::cerr << "Error cancelling id=999: " << e.what() << "\n";
    }

    std::cout << "=== Done ===" << std::endl;
}
/* 
int main() {
    runTests();
    return 0;
} */