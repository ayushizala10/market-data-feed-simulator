# Market Data Feed Simulator

A C++17 project that simulates a stream of stock market events and maintains the latest market state for multiple symbols.

## Features

- Generate simulated market events
- Track latest prices and total traded quantities
- Calculate price changes
- Display per-stock statistics
- Benchmark event processing
- Export market snapshots to CSV
- Validate basic event data

## Technologies

- C++17
- STL unordered_map and vector
- Random number generation
- Chrono for timing measurements
- CSV file output

## Build

```bash
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o main
```

## Run on Windows

```powershell
.\main.exe
```

## Data

All prices and events are simulated. This project does not connect to a live financial market.

## Limitations

The benchmark is a basic local microbenchmark. Results depend on the computer and system load and are not representative of production trading systems.