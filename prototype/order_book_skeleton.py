from price_level import PriceLevel
import heapq


class OrderBook:
    def __init__(self):
        self.bids: list[tuple[int, PriceLevel]] = []  # (-price, pl)
        self.bmap: dict[int, PriceLevel] = {}  # price -> pl
        self.asks: list[tuple[int, PriceLevel]] = []  # (price, pl)
        self.amap: dict[int, PriceLevel] = {}  # price -> pl
        self.id_to_pl: dict[int, PriceLevel] = {}  # id -> pl

    def add_order(self, order_id: int, side: str, price: int, qty: int) -> bool:
        # print("adding order")
        if side == "BUY":
            if price not in self.bmap:
                self.bmap[price] = PriceLevel(price, "BUY")
                heapq.heappush(
                    self.bids, (-price, self.bmap[price])
                )  # each price level object can have multiple ids
            self.bmap[price].add_order(order_id, qty)
            self.id_to_pl[order_id] = self.bmap[price]
            return True
        elif side == "SELL":
            if price not in self.amap:
                self.amap[price] = PriceLevel(price, "SELL")
                heapq.heappush(self.asks, (price, self.amap[price]))
            self.amap[price].add_order(order_id, qty)
            self.id_to_pl[order_id] = self.amap[price]
            return True

        else:
            return False

    def get_bids(self):
        return self.bids

    def get_amap(self):
        return self.amap

    def get_bmap(self):
        return self.bmap

    def get_asks(self):
        return self.asks

    def best_ask(self) -> tuple[int, int] | bool:  # lowest val
        """
        Peeks at the best ask we have in order book
        -> (id, quantity)
        -> False if DNE
        """
        if len(self.asks) < 1:
            return False

        price, pl = self.asks[0]
        t = pl.peek_order()
        if t:
            oid, qty = t
            return (price, qty)
        else:
            return False

    def best_bid(self) -> tuple[int, int] | bool:  # highest val
        """
        Peeks at the best bid we have in order book
        -> (price, quantity)
        -> False if DNE
        """
        if len(self.bids) < 1:
            return False

        invprice, pl = self.bids[0]
        # (-price, PriceLevel)

        t = pl.peek_order()
        if t:
            oid, qty = t
            return (-invprice, qty)
        else:
            return False

    def cancel_order(self, order_id) -> bool:
        if order_id in self.id_to_pl:
            pl = self.id_to_pl[order_id]
            pl.cancel_order(order_id)
            if not pl.peek_order():
                price = pl.get_price()
                type = pl.get_type()
                if type == "BUY":
                    for i in range(len(self.bids)):
                        if self.bids[i][0] == -price:
                            self.bids.pop(i)
                            break
                else:
                    for i in range(len(self.asks)):
                        if self.asks[i][0] == price:
                            self.asks.pop(i)
                            break

            return True
        else:
            return False


if __name__ == "__main__":
    # Initialize order book
    ob = OrderBook()

    # Add some orders
    ob.add_order(1, "BUY", 100, 10)  # bid 100 x 10
    ob.add_order(2, "BUY", 101, 5)  # bid 101 x 5
    ob.add_order(3, "SELL", 102, 20)  # ask 102 x 20
    ob.add_order(4, "SELL", 103, 15)  # ask 103 x 15

    # Queries
    print("begin")
    print(ob.best_bid())  # (101, 5)   highest BUY price is 101
    print(ob.best_ask())  # (102, 20)  lowest SELL price is 102

    # Cancel an order
    # print(ob.bids)
    ob.cancel_order(2)  # cancel order id 2 (bid @ 101)
    # print(ob.bids)

    print(ob.best_bid())  # (100, 10)  next highest bid
    print(ob.best_ask())  # (102, 20)  ask unchanged
