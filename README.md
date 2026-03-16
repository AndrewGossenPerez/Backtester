# Trading Engine

**Work-in-progress trading engine** featuring a **C++ event-driven backtesting engine**,  
CSV market-data ingestion, and a **Python front-end for visualisation and plotting**.

The goal of this project is to build a handrolled flexible research environment for developing and backtesting trading strategies.



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
   Processes the `FillEvent`, updating portfolio state, positions, and equity.

## Backtests currently include **transaction costs**:

- Slippage: **1.3 bps**
- Fee: **0.8 bps**

---

## Example Strategy

**EMA crossover signaller with volatility position sizing**  
tested on **AAPL 5-minute bars**

### Parameter Comparison

| EMA (24 / 120) | EMA (16 / 60) |
|---|---|
| <img src="images/AAPL(24,120).png" width="420"> | <img src="images/AAPL(16,60).png" width="420"> |

| EMA (30 / 150) | EMA (40 / 200) |
|---|---|
| <img src="images/AAPL(30,150).png" width="420"> | <img src="images/AAPL(40,200).png" width="420"> |

---

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

