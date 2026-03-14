
**Work-in-progress trading engine featuring a C++ event-driven backtester + CSV market-data ingestion, with a Python front-end for visualisation and plotting.**
A build system will be added later. I intend to create a system that allows me to develop and backtest my own strategies, hopefully making profitable algos.

---

Some backtests can be found in images (1.3 BPS SLIPPAGE, 0.8 BPS FEE)


Example EMA Crossover Signaller & Volatility Position Sizer on AAPL 5min bars
<div align="center">

<figure style="display:inline-block; margin:10px;">
  <img src="images/AAPL(24,120).png" width="350">
  <figcaption><em>(24/120)</em></figcaption>
</figure>

<figure style="display:inline-block; margin:10px;">
  <img src="images/AAPL(16,60).png" width="390">
  <figcaption><em>(16/60)</em></figcaption>
</figure>

</div>

## Current features 
- [x] Historical CSV market-data ingestion
- [x] Event-driven backtesting engine
- [x] Python interface (WIP)
- [ ] Optimisations
