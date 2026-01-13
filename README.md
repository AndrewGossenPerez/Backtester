<img width="150" height="150" alt="TradingEngine" src="https://github.com/user-attachments/assets/0f72cd40-f343-4f2a-b395-2ad8be74c60e" />

**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**
A build system will be added later.

---

* Current EMA + FF implementation on aapl market data 1 Nov 2022 to 14 Jan 2026. 


  <img width="346" height="611" alt="EMA+FF" src="https://github.com/user-attachments/assets/0f6b8430-dfa4-4ad3-b486-21f47475b6ec" />


## Benchmarks

### Backtesting Engine (100 runs Coinflip strat FF risk, consumer hardware)
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
- [ ] Fixed Fractional, less front exposure
- [ ] Optimisation
