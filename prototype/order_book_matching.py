from price_level import PriceLevel
from order_book_skeleton import OrderBook
import heapq

DEFAULT_BID = 100
DEFAULT_ASK = 100


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
        lp = None
        if type == "MARKET":
            price = float("inf")

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
                if type != "MARKET":
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
                if type != "MARKET":
                    super().add_order(order_id, side, price, qty)
            # SELL COMPLETE

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


### VALIDATION ###


def match_trades_and_return_book(orders):
    """
    orders: list of dicts with keys {id, side, type, price, qty}
    returns: (trades, book)
      trades: list of [taker_id, maker_id, price, qty]
      book: {"bids": [(price, [(id, qty), ...])], "asks": [(price, [(id, qty), ...])]}
    """
    ob = OrderBookMatching()
    for order in orders:
        if order["type"] == "CANCEL":
            # cancel this order
            ob.cancel_order(order["cancel_id"])
        else:
            ob.add_order(
                order["id"], order["side"], order["price"], order["qty"], order["type"]
            )

    return (ob.get_ledger(), ob.get_print_book())


def test_fifo_partial_fill():
    orders = [
        {"id": 1, "side": "SELL", "type": "LIMIT", "price": 101, "qty": 10},
        {"id": 2, "side": "SELL", "type": "LIMIT", "price": 101, "qty": 5},
        {"id": 3, "side": "BUY", "type": "LIMIT", "price": 102, "qty": 12},
        {"id": 4, "side": "BUY", "type": "LIMIT", "price": 101, "qty": 4},
    ]
    expected_trades = [
        [3, 1, 101, 10],
        [3, 2, 101, 2],
        [4, 2, 101, 3],
    ]
    expected_book = {"bids": [(101, [(4, 1)])], "asks": []}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


def test_market_buy_against_single_ask():
    orders = [
        {"id": 10, "side": "SELL", "type": "LIMIT", "price": 105, "qty": 5},
        {"id": 11, "side": "BUY", "type": "MARKET", "price": 0, "qty": 3},
    ]
    expected_trades = [
        [11, 10, 105, 3],
    ]
    expected_book = {"bids": [], "asks": [(105, [(10, 2)])]}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


def test_market_order_exhausts_book():
    orders = [
        {"id": 20, "side": "SELL", "type": "LIMIT", "price": 103, "qty": 2},
        {"id": 21, "side": "BUY", "type": "MARKET", "price": 0, "qty": 5},
    ]
    expected_trades = [
        [21, 20, 103, 2],
    ]
    expected_book = {"bids": [], "asks": []}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


def test_limit_order_rests_when_no_liquidity():
    orders = [{"id": 30, "side": "BUY", "type": "LIMIT", "price": 100, "qty": 4}]
    expected_trades = []
    expected_book = {"bids": [(100, [(30, 4)])], "asks": []}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


def test_multiple_price_levels():
    orders = [
        {"id": 40, "side": "SELL", "type": "LIMIT", "price": 102, "qty": 3},
        {"id": 41, "side": "SELL", "type": "LIMIT", "price": 101, "qty": 2},
        {"id": 42, "side": "BUY", "type": "LIMIT", "price": 103, "qty": 5},
    ]
    expected_trades = [
        [42, 41, 101, 2],
        [42, 40, 102, 3],
    ]
    expected_book = {"bids": [], "asks": []}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


def test_fifo_with_partial_and_new_entry():
    orders = [
        {"id": 50, "side": "SELL", "type": "LIMIT", "price": 100, "qty": 5},
        {"id": 51, "side": "SELL", "type": "LIMIT", "price": 100, "qty": 5},
        {"id": 52, "side": "BUY", "type": "LIMIT", "price": 100, "qty": 6},
        {"id": 53, "side": "BUY", "type": "LIMIT", "price": 100, "qty": 2},
    ]
    expected_trades = [
        [52, 50, 100, 5],
        [52, 51, 100, 1],
        [53, 51, 100, 2],
    ]
    expected_book = {"bids": [], "asks": [(100, [(51, 2)])]}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


def test_cancel_order():
    orders = [
        {"id": 60, "side": "SELL", "type": "LIMIT", "price": 105, "qty": 5},
        {
            "id": 61,
            "side": "CANCEL",
            "type": "CANCEL",
            "price": 105,
            "qty": 0,
            "cancel_id": 60,
        },
        {"id": 62, "side": "BUY", "type": "LIMIT", "price": 105, "qty": 3},
    ]
    expected_trades = []  # because order 60 was canceled
    expected_book = {"bids": [(105, [(62, 3)])], "asks": []}
    trades, book = match_trades_and_return_book(orders)
    assert trades == expected_trades
    assert book == expected_book


if __name__ == "__main__":
    """expected_trades = [
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
    assert ob.get_ledger() == expected_trades"""

    # import pytest
    print("begin testing")

    test_fifo_partial_fill()
    test_market_buy_against_single_ask()
    test_market_order_exhausts_book()
    test_limit_order_rests_when_no_liquidity()
    test_multiple_price_levels()
    test_fifo_with_partial_and_new_entry()
    test_cancel_order()

    print("all tests passed")
