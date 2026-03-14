# A backtest engine

**Work-in-progress backtest engine** featuring a **C++ event-driven backtester** and **CSV market-data ingestion**, with a **Python front-end for visualisation and plotting**.

The goal is to build a flexible research environment for developing, testing, and evaluating trading strategies. Hopefully making profitable ones (Although realistically, probably not gonna happen). 

> Build system and additional optimisations are planned.

---

## Overview

- **Language:** C++17, Python 3.12.4
- **Core architecture:** event-driven backtesting engine using ring buffers
- **Data source:** historical CSV market data from Alpaca's API
- **Visualisation:** Python + Matplotlib
- **Use case:** strategy prototyping & performance analysis

---

## Example Strategy

**EMA crossover signaller + volatility position sizer** on **AAPL 5-minute bars**

### Parameter Comparison

| **EMA (24 / 120)** | **EMA (16 / 60)** |
|---|---|
| <img src="images/AAPL(24,120).png" width="420"><br><sub>A slower, smoother trend-following configuration.</sub> | <img src="images/AAPL(16,60).png" width="420"><br><sub>A faster configuration with more signal sensitivity.</sub> |

| **EMA (30 / 150)** | **EMA (40 / 200)** |
|---|---|
| <img src="images/AAPL(30,150).png" width="420"><br><sub>Higher lookbacks for stronger noise filtering.</sub> | <img src="images/AAPL(40,200).png" width="420"><br><sub>A slower configuration focused on multi-session drift.</sub> |

---

## Backtest Assumptions

> **Transaction cost model:** 1.3 bps slippage, 0.8 bps fee

Additional backtests can be found in the `images/` directory.

---

## Current Features

- [x] Historical CSV market-data ingestion
- [x] Event-driven backtesting engine
- [x] Python interface for plotting and visualisation
- [ ] Build system
- [ ] Further optimisation work
- [ ] Additional strategy modules

---

## Roadmap

- Add build system
- Improve engine performance and profiling
- Add more strategy implementations
- Add parameter heatmaps and robustness testing
