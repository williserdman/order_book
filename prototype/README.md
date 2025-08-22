# Order Matching Engine (Python Prototype)

## Overview

This repository contains a prototype limit order book and matching engine implemented in Python. The system is designed for low-latency order handling and performance benchmarking. It provides a reference implementation for message-driven trading infrastructure.

## Features

-   In-memory limit order book with FIFO matching at each price level
-   Support for limit, market, and cancel orders
-   JSON-based message stream interface
-   Benchmark harness with `time.perf_counter` for latency measurement
-   Separation of parsing, matching, and trade logging for future concurrency extensions

## Project Structure

```
.
├── prototype
│   ├── order_book_matching.py # Core order book data structures and matching logic
│   ├── order_book_skeleton.py # resting order book
│   ├── price_level.py
│   ├── README.md
│   └── throughput_testing.py # benchmarking harness for matching version of orderbook
└── README.md
```

## Performance

Latency benchmarks are collected using random order streams.

Ran on 2021 Macbook Pro M1 Pro

**Without JSON parsing**

-   p50 ≈ 1.0 µs
-   p99 ≈ 1.5 µs
-   p99.9 ≈ 8–9 µs

**With `json.loads` parsing**

-   p50 ≈ 2.17 µs
-   p99 ≈ 2.67 µs
-   p99.9 ≈ 11.63 µs

Results highlight both the efficiency of the order book operations and the overhead introduced by JSON deserialization in Python.

## Design

-   **Order Book:**  
    Two heaps are maintained: a max-heap for bids and a min-heap for asks. Each price level holds a FIFO queue of orders. Matching executes greedily and supports partial fills.

-   **Message Processing:**  
    Incoming messages are JSON objects of type `LIMIT`, `MARKET`, or `CANCEL`. Messages are parsed, applied to the order book, and may generate trades.

-   **Benchmarking:**  
    Random order streams are generated and processed. Latencies are recorded per message and aggregated into percentile distributions.

## Extensions

-   Replace JSON with a binary protocol to reduce parsing overhead
-   Add lock-free queues for concurrent parsing, matching, and logging
-   Benchmark at higher message rates to evaluate throughput limits
-   Implement visualization of order book depth and trade activity
