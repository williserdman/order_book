from order_book_matching import OrderBookMatching

ob = OrderBookMatching()


def processOrders(orders):
    """
    orders: list of dicts with keys {id, side, type, price, qty}
    returns: (trades, book)
      trades: list of [taker_id, maker_id, price, qty]
      book: {"bids": [(price, [(id, qty), ...])], "asks": [(price, [(id, qty), ...])]}
    """
    ob = OrderBookMatching()
    for jorder in orders:
        order = json.loads(jorder)
        # print(order)
        if order["type"] == "CANCEL":
            # cancel this order
            ob.cancel_order(order["cancel_id"])
        else:
            ob.add_order(
                order["id"], order["side"], order["price"], order["qty"], order["type"]
            )

    return ob.get_ledger()


import random
import json
import time
import numpy as np

NUM_ORDERS = 1000
MAX_PRICE = 200
MAX_QTY = 50
ORDER_ID_START = 1

PROB_LIMIT = 0.5
PROB_MARKET = 0.3
PROB_CANCEL = 0.2


def benchmark_performance(n_orders: int = 100_000):
    orders = []
    current_order_id = ORDER_ID_START
    for _ in range(NUM_ORDERS):
        r = random.random()

        if r < PROB_LIMIT:
            side = random.choice(["BUY", "SELL"])
            price = random.randint(1, MAX_PRICE)
            qty = random.randint(1, MAX_QTY)
            order = {
                "type": "LIMIT",
                "id": current_order_id,
                "side": side,
                "price": price,
                "qty": qty,
            }
            current_order_id += 1

        elif r < PROB_LIMIT + PROB_MARKET:
            side = random.choice(["BUY", "SELL"])
            qty = random.randint(1, MAX_QTY)
            order = {
                "type": "MARKET",
                "side": side,
                "qty": qty,
                "id": current_order_id,
                "price": 0,
            }
            current_order_id += 1

        else:  # CANCEL
            if len(orders) < 1:
                continue
            cancel_order = random.choice(orders)
            if "id" in json.loads(cancel_order):
                order = {"type": "CANCEL", "cancel_id": json.loads(cancel_order)["id"]}

        orders.append(json.dumps(order))

    # [print(o) for o in orders[:4]]

    # Benchmark with perf_counter
    latencies = []
    trades = []
    start = time.perf_counter()

    for msg in orders:
        t0 = time.perf_counter()
        trades.extend(processOrders([msg]))
        t1 = time.perf_counter()
        latencies.append((t1 - t0) * 1e6)  # microseconds

    end = time.perf_counter()
    total_time = end - start
    throughput = n_orders / total_time

    # Compute latency stats
    latencies.sort()

    def pct(p):
        return latencies[int(len(latencies) * p)]

    print(f"Processed {n_orders} messages in {total_time:.4f} sec")
    print(f"Throughput: {throughput:.2f} msgs/sec")
    print(f"Latency p50: {pct(0.50):.2f} µs")
    print(f"Latency p99: {pct(0.99):.2f} µs")
    print(f"Latency p99.9: {pct(0.999):.2f} µs")


if __name__ == "__main__":
    benchmark_performance(1_000_000_000)
