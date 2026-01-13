5vc/<img width="150" height="150" alt="TradingEngine" src="https://github.com/user-attachments/assets/0f72cd40-f343-4f2a-b395-2ad8be74c60e" />

**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**
A build system will be added later.

---


## Benchmarks

### Backtesting Engine (100 runs Coinflip strat FF risk)
- **Median throughput:** **2.66M bars/s**  
- **P90 throughput:** **2.48M bars/s**  

### CSV Ingestion
- **Throughput:** **5M bars/s** (With Epoch given)

*Dataset: ~7.6M-row Bitcoin OHLCV dataset, Coinflip strategy ~ fills every 2nd bar, FF risk layer.*

## Features
- [x] Modular strategy framework
- [x] Historical CSV market-data ingestion
- [x] Event-driven backtesting engine
- [x] Python interface (WIP)
- [x] Fixed Fractional Risk Handler with a stopping system
- [x] EMA (Simple first prototype)
- [ ] Optimisation
