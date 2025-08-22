"""
Module for managing price levels in an order book.

Author: Willis Erdman
Created: Aug 21, 2025
Version: 1.0

"""


class OrderNode:
    def __init__(self, id: int, qty: int):
        self.right = None
        self.left = None
        self.id = id
        self.qty = qty


class PriceLevel:

    def __init__(self, price: int, type: str):
        self.dict = {}
        self.head = None
        self.tail = None
        self.price = price
        self.type = type

    def simple_list(self) -> list:
        out = []
        curr = self.head
        while curr:
            out.append((curr.id, curr.qty))
            curr = curr.right
        return out

    def get_type(self) -> str:
        return self.type

    def get_price(self) -> int:
        return self.price

    def empty(self):
        return False if self.head else True

    def add_order(self, order_id: int, qty: int) -> bool:
        """

        Adds an order to the order book if it doesn't already exist
        -> False if it already exists
        -> True when added

        """

        if order_id in self.dict:
            return False

        me = OrderNode(order_id, qty)
        if self.head:
            me.left = self.tail
            self.tail.right = me
            self.tail = me
        else:
            self.head = self.tail = me

        self.dict[order_id] = me

        return True

    def pop_order(self) -> tuple[int, int] | bool:
        """
        Returns the oldest order at this price level
        -> (order_id, quantity)
        -> False if DNE
        """

        if self.head is None:
            return False

        el = self.head
        self.head = self.head.right

        rval = (el.id, el.qty)
        del self.dict[el.id]

        return rval

    def peek_order(self) -> tuple[int, int] | bool:
        """
        Returns the oldest order at this price level without removal
        -> (order_id, quantity)
        -> False if DNE
        """
        if self.head is None:
            return False

        return (self.head.id, self.head.qty)

    def cancel_order(self, id: int) -> tuple[int, int] | bool:
        """
        Cancels an order by order_id
        -> (order_id, quantity) if removed
        -> False if DNE
        """
        if id not in self.dict:
            return False

        node = self.dict[id]
        rnode = node.right
        lnode = node.left
        if lnode:
            lnode.right = rnode
        if rnode:
            rnode.left = lnode
        if self.head == node:
            self.head = node.right
        if self.tail == node:
            self.tail = node.left

        rval = (node.id, node.qty)
        del node

        return rval

    def decrement_by_id(self, id: int, amt: int) -> bool:
        if id in self.dict:
            node = self.dict[id]
            node.qty -= amt
            return True
        return False


if __name__ == "__main__":
    pl = PriceLevel()
    pl.add_order(1, 100)
    pl.add_order(2, 200)
    pl.add_order(3, 300)

    print(pl.pop_order())  # (1, 100) FIFO
    print(pl.cancel_order(3))  # (3, 300)
    print(pl.pop_order())  # (2, 200)
