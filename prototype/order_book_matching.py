from price_level import PriceLevel
from order_book_skeleton import OrderBook
import heapq


class OrderBookMatching(OrderBook):
    def __init__(self):
        super().__init__()
        self.ledger = []

    def get_print_book(self):
        return {"bids": self.get_print_bids(), "asks": self.get_print_asks()}

    def get_print_asks(self) -> list:
        out = []
        for price, pl in self.get_asks():
            out.append((price, pl.simple_list()))
        out.sort()
        return out

    def get_print_bids(self) -> list:
        out = []
        for price, pl in self.get_bids():
            out.append((-price, pl.simple_list()))
        out.sort()
        return out

    def add_order(self, order_id, side, price, qty, type: str):
        # try to match, otherwise becomes maker, market becomes maker at last paid price
        # TODO: add market order
        if side == "BUY":
            m = super().best_ask()  # maker
            accepts = True
            while qty > 0 and m and accepts:
                maker_price, maker_qty = m
                if maker_price <= price:  # else becomes maker
                    # acceptable price
                    p = q = i = 0
                    if maker_qty <= qty:
                        p, q, i = self.pop_asks()
                        m = super().best_ask()
                    else:
                        p, q, i = self.partial_buy(qty)
                    self.add_transaction(order_id, i, p, q)
                    qty -= q
                else:
                    accepts = False
            if qty > 0:
                # becomes maker
                super().add_order(order_id, side, price, qty)
            # BUY COMPLETE
        else:
            # sell side
            m = super().best_ask()  # maker
            accepts = True
            while qty > 0 and m and accepts:
                inv_maker_price, maker_qty = m
                maker_price = -inv_maker_price
                if maker_price >= price:  # else becomes maker
                    # acceptable price
                    p = q = i = 0
                    if maker_qty <= qty:
                        p, q, i = self.pop_bids()
                        m = super().best_bid()
                    else:
                        p, q, i = self.partial_sell(qty)
                    self.add_transaction(order_id, i, p, q)
                    qty -= q
                else:
                    accepts = False
            if qty > 0:
                # becomes maker
                super().add_order(order_id, side, price, qty)
            # BUY COMPLETE

    def pop_bids(self) -> tuple[int, int, int]:  # -> (price, quantitiy, id)
        """
        Sells to the highest ask we have in book
        -> (price, quantity, id)
        """
        if len(self.get_bids()) < 1:
            return False

        price, pl = self.get_bids()[0]
        oid, qty = pl.pop_order()
        if pl.empty():
            heapq.heappop(self.get_bids())
            del self.get_bmap()[price]

        return (price, qty, oid)

    def pop_asks(self) -> tuple[int, int, int]:  # -> (price, quantitiy, id)
        """
        Buys the lowest ask we have in order book
        -> (price, quantity, id)
        """
        if len(self.get_asks()) < 1:
            return False

        # price, pl = self.bids[0]
        price, pl = self.get_asks()[0]
        oid, qty = pl.pop_order()
        if pl.empty():
            # delete this price level
            heapq.heappop(self.get_asks())
            del self.get_amap()[price]

        return (price, qty, oid)

    def partial_buy(self, amt: int) -> tuple[int, int, int]:
        """
        Partially buys the lowest ask we have in order book
        -> (price, quantity, id)
        """
        if len(self.get_asks()) < 1:
            return False

        price, pl = self.get_asks()[0]
        oid, qty = pl.peek_order()
        pl.decrement_by_id(oid, amt)

        return (price, amt, oid)

    def partial_sell(self, amt: int) -> tuple[int, int, int]:
        """
        Partially sells to the highest ask we have in order book
        -> (price, quantity, id)
        """

        if len(self.get_bids()) < 1:
            return False

        price, pl = self.get_bids()[0]
        oid, qty = pl.peek_order()
        pl.decrement_by_id(oid, amt)

        return (price, amt, oid)

    def add_transaction(
        self, taker_id: int, maker_id: int, price: int, qty: int
    ) -> None:
        """
        adds to ledger (taker_id, maker_id, price, qty)
        """
        # print(taker_id, maker_id, price, qty)
        self.ledger.append([taker_id, maker_id, price, qty])

    def get_ledger(self):
        return self.ledger


if __name__ == "__main__":
    expected_trades = [
        [3, 1, 101, 10],
        [3, 2, 101, 2],
        [4, 2, 101, 3],
    ]
    expected_book = {"bids": [(101, [(4, 1)])], "asks": []}  # price, [(id, qty)]

    ob = OrderBookMatching()

    ob.add_order(1, "SELL", 101, 10, "LIMIT")
    ob.add_order(2, "SELL", 101, 5, "LIMIT")
    ob.add_order(3, "BUY", 102, 12, "LIMIT")
    ob.add_order(4, "BUY", 101, 4, "LIMIT")

    print(ob.get_print_book())
    print(ob.get_ledger())
    assert ob.get_print_book() == expected_book
    assert ob.get_ledger() == expected_trades
