
**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**
A build system will be added later. I intend to create a system that allows me to develop and backtest my own strategies, hopefully making profitable algos.

---

Naked EMA Signaller w/o risk management (NSlow=8,NFast=21) NVDA


<img width="500" height="250" alt="image" src="https://github.com/user-attachments/assets/7beba6d1-a9ae-4ea6-9746-09eb9a2fdbbc" />



Buy/Hold NVDA 

<img width="500" height="250" alt="Screenshot 2026-03-02 at 6 16 50 PM" src="https://github.com/user-attachments/assets/f155d8d5-0249-49a7-a145-58da1885b14b" />



## Current Benchmarks

### Backtesting Engine (100 runs Coinflip strat no risk manager, consumer hardware)
- **Median throughput:** **2.66M bars/s**  
- **P90 throughput:** **2.48M bars/s**  

### CSV Ingestion
- **Throughput:** **5.1M bars/s** (With Epoch given)

*Dataset: ~7.6M-row Bitcoin OHLCV dataset, Coinflip strategy ~ fills every 2nd bar, FF risk layer.*

## Features
- [x] Modular strategy framework
- [x] Historical CSV market-data ingestion
- [x] Event-driven backtesting engine
- [x] Python interface (WIP)
- [x] Fixed Fractional Risk Handler with a stopping system
- [x] EMA (Simple first prototype)
- [ ] Volatility scaling
- [ ] Shorting
