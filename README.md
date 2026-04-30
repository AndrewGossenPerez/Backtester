<img src="images/AAPL(24,120).png" width="500">

# A Backtester

Work-in-progress backtester featuring a C++ event-driven backtesting engine,  
CSV market-data ingestion, and a Python front-end for visualisation and plotting.

## Installation

```bash
git clone --recursive https://github.com/AndrewGossenPerez/Backtester.git
cd Backtester
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run

```bash
./build/trading_main
```

## Python example

```bash
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements.txt
PYTHONPATH="$PWD/build" python python/backtest_plotter.py
```

> Note: The Python example uses a predefined backtest configuration in [`binding.cpp`](src/binding/binding.cpp).

## Core components:

- C++ event-driven backtesting engine
- Inbuilt Stop Loss system
- CSV market-data ingestion
- Python visualisation layer
- Strategy prototyping framework

## Event Architecture:

Each market bar propagates through the following event pipeline:

1. **Strategy Handler**  
   Receives a `MarketEvent` (new bar) and generates a `SignalEvent` (Buy/Sell/Hold).

2. **Risk Handler**  
   Processes the `SignalEvent` and generates an `OrderEvent` according to the risk model.

3. **Execution Handler**  
   Simulates market execution and converts the `OrderEvent` into an authoritative `FillEvent`.

4. **Portfolio Handler**  
   Processes the `FillEvent`, which in turn updates position 

## Backtests currently include **transaction costs**:

- Slippage: **2.8 bps**
- Fee: **1.0 bps**
> These can be modified in [`config.hpp`](include/data/config.hpp).  
---

## Recent Benchmarks

### Dataset
- **7,371,037 OHLCV bars**
- Single-threaded execution
- 1000 runs (with warmup)

---

### Backtesting Engine

| Metric | Value |
|---|---:|
| Median runtime | **0.256 s** |
| Mean runtime | **0.270 ± 0.039 s** |
| p90 runtime | **0.313 s** |
| Min / Max | **0.237 / 0.747 s** |
| Median bars/sec | **28.8M** |
| Mean bars/sec | **27.3M** |
| p90 bars/sec | **23.6M** |
| Median fills | **4,742** |
| Median fills/sec | **18,532** |

---

### CSV Ingestion

| Metric | Value |
|---|---:|
| Median runtime | **0.398 s** |
| Mean runtime | **0.421 ± 0.076 s** |
| p90 runtime | **0.498 s** |
| Median bars/sec | **18.5M** |
| Mean bars/sec | **17.5M** |
---

### Notes

- Median used to reduce variance from system noise  
- Warmup runs performed before measurement  
- Includes full event pipeline (strategy → risk → execution → portfolio)  
---

## Future Work

Planned improvements:

- Build system 
- Optimisations 
- Strategy parameter heatmaps

