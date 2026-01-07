
**A work-in-progress trading engine with a C++ backtester and a Python front-end data visualisation/plotting.**
--

*Example of a Buy-Hold strategy backtested against Bitcoin's market data and plotted with Matplotlib*
<img width="1392" height="682" alt="Screenshot 2026-01-07 at 11 00 34 PM" src="https://github.com/user-attachments/assets/9f97bc82-481f-4348-af3e-b2e4b725d27e" />


## Current Benchmarks 
Backtesting Throughput: ~1.8M bars/sec


*(median of 100 runs; Bitcoin OHLCV dataset, 7.6M+ bars)*

Data Ingestion Throughput: ~2.2M bars/sec


*(median of 100 runs; Bitcoin OHLCV dataset, 7.6M+ bars)*


## Features
- [x] Modular strategy framework  
- [x] Historical CSV market data ingestion
- [X] Backtesting engine (Event-driven architecture) 
- [X] Python Interface (WIP)
- [ ] Optimisation 
