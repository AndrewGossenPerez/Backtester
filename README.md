<img width="250" height="250" alt="TradingEngine" src="https://github.com/user-attachments/assets/0f72cd40-f343-4f2a-b395-2ad8be74c60e" />

**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**

---

*Buy-and-hold strategy backtested on Bitcoin OHLCV data and plotted with Matplotlib.*
<img width="1392" height="682" alt="Screenshot 2026-01-07 at 11 00 34 PM" src="https://github.com/user-attachments/assets/9f97bc82-481f-4348-af3e-b2e4b725d27e" />

## Benchmarks
- **Backtesting throughput:** ~**1.8M bars/s**
- **Data ingestion throughput:** ~**2.2M bars/s**

*Median of 100 runs on a Bitcoin OHLCV dataset (~7.6M bars).*

## Features
- [x] Modular strategy framework
- [x] Historical CSV market-data ingestion
- [x] Event-driven backtesting engine
- [x] Python interface (WIP)
- [ ] Optimisation
