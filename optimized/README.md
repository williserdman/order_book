## C++ Limit Order Book

last update 09/02/2025

📖 Overview

This project is a prototype order matching engine in C++ designed to simulate the core mechanisms of modern exchanges. It maintains a limit order book, supports market, limit, and cancel orders, and enforces price–time priority.

The current focus is correctness and profiling rather than production-level performance. Benchmarks reveal allocator overhead as the dominant bottleneck, and the roadmap outlines the path toward high-performance systems engineering.

⸻

⚙️ Features

-   In-memory limit order book with price-level priority.
-   Order types: market, limit, and cancel.
-   Order matching: FIFO within each price level.
-   ID-to-order tracking via hash map for efficient cancels.
-   Benchmarking harness:
-   Throughput (orders/sec).
-   Latency distributions (p50, p99, p99.9).
-   Nanosecond-level timing of critical operations.

⸻

🧪 Current Performance

-   Naive implementation: ~500k orders/sec.
-   Average per-order latency: ~2000 ns.
-   Profiling results:
-   addOrder (~500 ns): dominated by heap allocation inside price-level containers.
-   emplace (~250 ns): cost of unordered_map insertion + potential rehashing.
-   creating new price levels ~1000 ns
-   Remaining overhead ~250 ns: lookups and matching logic.

⸻

🛠️ Roadmap Toward High Performance

1. Memory Pooling (Reduce malloc/free)

    - Problem: Each new order triggers dynamic allocation.
    - Solution: Implement a fixed-size pool of Order objects with O(1) allocate/free.
    - Goal: Cut addOrder latency from ~500 ns → ~10 ns.

2. Cache-Friendly Hash Maps

    - Problem: std::unordered_map incurs pointer chasing + rehashing.
    - Solution: Replace with Abseil’s flat_hash_map or Robin Hood hashing.
    - Goal: Reduce ID mapping (emplace) cost from ~250 ns → ~50 ns.

    - IDEA: we could also skip hashing entirely if we have an array that covers possible price varyations, this would be 100 \* 100 = 10,000 pointers long, if we cover every whole cent 0.00 to 99.99. trade memory for speed

3. Concurrency & Lock-Free Structures

    - Problem: Current engine is single-threaded with logging and order processing intertwined.
    - Solution:
    - Dedicated threads for message parsing, order book updates, and logging.
    - Lock-free queues (e.g., ring buffer) for inter-thread communication.
    - Goal: Maximize throughput while minimizing contention.

4. Advanced Optimizations
    - Batching: Process incoming orders in small batches to reduce synchronization overhead.
    - Cache alignment: Align frequently accessed structures to cache line boundaries.
    - NUMA awareness: For multi-socket systems, ensure locality of order pools and queues.

⸻

📂 Project Structure

````.
├── OBMatching
├── OBSimple
├── OBTesting
├── OBTesting.cpp
├── OrderBookMatching.cpp
├── OrderBookMatching.h
├── OrderBookResting.cpp
├── OrderBookResting.h
├── PLTesting
├── PLTesting.cpp
├── PriceLevel
├── PriceLevel.cpp
├── PriceLevel.h
└── README.md```


⸻

📈 Future Work
	•	Integrate exchange-style features (partial fills, iceberg orders).
	•	Add persistent logging of trades (async disk writer).
	•	Implement real-time monitoring of throughput/latency metrics.

⸻

📜 Status

This project is a correctness-first prototype. The next iteration will target order-of-magnitude speedups through memory pooling, cache-friendly data structures, and lock-free concurrency.

⸻

Would you like me to also design a graphical roadmap diagram (like a flowchart or timeline) for your README so it visually shows “Prototype → Memory Pools → Flat Hash Maps → Lock-Free → Production”? That could make it pop for GitHub/portfolio.
note to self

g++ -std=c++23 OrderBookResting.cpp PriceLevel.cpp -o OBSimple

to get tuples to work on macos

g++ -O3 -std=c++23 -march=native optimized/\*.cpp -o optimized/OBTesting
````
