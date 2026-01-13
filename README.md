<img width="150" height="150" alt="TradingEngine" src="https://github.com/user-attachments/assets/0f72cd40-f343-4f2a-b395-2ad8be74c60e" />

**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**
A build system will be added later.

---

*Buy-and-hold strategy backtested on Bitcoin OHLCV data and plotted with Matplotlib to sanity check the engine*
<img width="350" height="150" alt="Screenshot 2026-01-07 at 11 00 34 PM" src="https://github.com/user-attachments/assets/9f97bc82-481f-4348-af3e-b2e4b725d27e" />


*Prototype of an EMA strategy with Fixed Fractional risk management, a minimum market % change to trigger a signal is 2.5% to avoid whipsaw*


<img width="350" height="150" alt="BTC" src="https://github.com/user-attachments/assets/63d2d288-8bfd-4fdf-807a-93526a93a785" />

## Benchmarks

### Backtesting Engine (100 runs)
- **Median throughput:** **2.66M bars/s**  
- **P90 throughput:** **2.48M bars/s**  

### CSV Ingestion
- **Throughput:** **5M bars/s** (With Epoch given)

*Dataset: ~7.6M-row Bitcoin OHLCV dataset, Coinflip strategy.*

## Features
- [x] Modular strategy framework
- [x] Historical CSV market-data ingestion
- [x] Event-driven backtesting engine
- [x] Python interface (WIP)
- [x] Fixed Fractional Risk Handler with a stopping system
- [x] EMA (Simple first prototype)
- [ ] Optimisation
