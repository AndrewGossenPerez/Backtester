
**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**
A build system will be added later.

---

Very aggresive scalping approach (NFast 2, NSlow 8) - No risk manager
<img width="500" height="250" alt="Screenshot 2026-02-26 at 5 36 43 PM" src="https://github.com/user-attachments/assets/d5992d97-f9f4-4d71-8e9d-b7106f592cd5" />


<img width="500" height="250" alt="Screenshot 2026-02-26 at 5 37 02 PM" src="https://github.com/user-attachments/assets/cc63860b-5e0b-4ef0-9aec-379703333d15" />


<img width="500" height="300" alt="Screenshot 2026-02-26 at 5 37 19 PM" src="https://github.com/user-attachments/assets/6714669d-fe07-4986-8f54-b5b667ee68df" />



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
- [ ] Orderbooking ( Start of a truly low-latency system ) 
- [ ] Optimisation
