import trading_engine as te
import matplotlib.pyplot as plt

# Trading engine backtest 
startingEquity = 100000
res = te.run(startingEquity)

# Data visulisation below 
BARSTEP = 1

plt.rcParams.update({
    "figure.dpi": 140,
    "axes.titlesize": 14,
    "axes.labelsize": 12,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
})

pts = res.equityPoints
pts = sorted(pts, key=lambda p: p.epoch)

x = list(range(len(pts)))
y = [p.equity for p in pts]

x_ds = x[::BARSTEP]
y_ds = y[::BARSTEP]

fig, ax = plt.subplots(figsize=(10, 5))
ax.step(x_ds, y_ds, where="post", linewidth=1.6)

ax.set_title("Equity Curve (per bar)")
ax.set_xlabel("Bar Index")
ax.set_ylabel("Equity ($)")
ax.ticklabel_format(style="plain", axis="both", useOffset=False)
ax.grid(True, which="major", alpha=0.30)
ax.minorticks_on()
ax.grid(True, which="minor", alpha=0.12)

# Y padding so the line isnt too close to the border 
ymin, ymax = min(y_ds), max(y_ds)
pad = (ymax - ymin) * 0.05 if ymax > ymin else 1.0
ax.set_ylim(ymin - pad, ymax + pad)

final_equity = y[-1]
pnl = y[-1]-startingEquity
ret=(pnl/startingEquity) * 100.0 # Calculate the return 

ax.text(
    0.02, 0.98,
    f"Starting Equity: ${startingEquity:,.0f}\nFinal Equity: ${final_equity:,.2f}\nFinal PnL: ${pnl:,.2f} ({ret:.2f}%)",
    transform=ax.transAxes,
    va="top",
    ha="left",
    bbox=dict(boxstyle="round", facecolor="grey", alpha=0.85, edgecolor="0.8")
)

plt.tight_layout()
plt.show()
