<img src="images/AAPL(24,120).png" width="500">

# A Backtester

Work-in-progress backtester featuring a C++ event-driven backtesting engine,  
CSV market-data ingestion, and a Python front-end for visualisation and plotting.

### Installation
## Build

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


> Note: The Python example (for now) uses a predefined backtest configuration in [`binding.cpp`](src/binding/binding.cpp).  
> To modify the strategy, dataset, or parameters, edit `run_backtest` and rebuild.

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

## Example Strategy

**EMA crossover signaller with volatility position sizing**  
tested on **AAPL 5-minute bars**

## Recent Benchmarks

Tested on **consumer hardware**, on a single thread.
(p50,n=1000) on a 936k AAPL OHLCV dataset

| Component | Throughput |
|---|---:|
| CSV market-data ingestion | **~5.2M OHLCV bars/sec** |
| Event-driven backtest engine | **~1.55M bars/sec** |
| Strategy configuration | **EMA signaller + volatility position sizer** |


---

## Future Work

Planned improvements:

- Build system 
- Optimisations 
- Strategy parameter heatmaps

