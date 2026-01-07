
A work-in-progress trading engine with a C++ backtester and a Python front-end data visualisation/plotting.

<img width="200" height="200" alt="Deltoid2" src="https://github.com/user-attachments/assets/c6585f45-40cb-49f2-9ad0-850d47ff203c" />


**Current example of a Buy-Hold strategy backtested and plotted with Matplotlib**


<img width="277.4" height="134.6" alt="image" src="https://github.com/user-attachments/assets/fe4f738b-a0ae-4eb8-aa36-42a9b90403fd" />

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
