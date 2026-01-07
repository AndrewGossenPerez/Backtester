
# This is currently just a prototype set up to validate the backtesting engine

import trading_engine as te
import matplotlib.pyplot as plt

# -- Actual Trading Engine 
print("Starting Backtest ...")
startingEquity = 100_000
res = te.run(startingEquity)

# --- Settings 
BARSTEP = 1
inTime = True  # If set to true then instead of per bar, equity will be versus time ( in days ) 

# -- Visulisation below 
plt.rcParams.update({
    "figure.dpi": 140,
    "axes.titlesize": 14,
    "axes.labelsize": 12,
    "xtick.labelsize": 10,
    "ytick.labelsize": 10,
})

pts = res.equityPoints
if not pts:
    raise RuntimeError("No equityPoints returned")

startingEpoch = pts[0].epoch

# -- X axis
if inTime:
    title = "Equity Curve vs Time (days elapsed)"
    # Epoch is in 
    x = [(p.epoch - startingEpoch) / (86400) for p in pts]
    xlabel = "Days elapsed"
else:
    title = "Equity per bar"
    x = list(range(len(pts)))
    xlabel = "Bar Index"

# -- Y axis
y = [p.equity for p in pts]

# -- Downsample
x_ds = x[::BARSTEP]
y_ds = y[::BARSTEP]

fig, ax = plt.subplots(figsize=(10, 5))
ax.step(x_ds, y_ds, where="post", linewidth=1.6)

ax.set_title(title)
ax.set_xlabel(xlabel)
ax.set_ylabel("Equity ($)")
ax.ticklabel_format(style="plain", axis="both", useOffset=False)

ax.grid(True, which="major", alpha=0.30)
ax.minorticks_on()
ax.grid(True, which="minor", alpha=0.12)

# Y padding
ymin, ymax = min(y_ds), max(y_ds)
pad = (ymax - ymin) * 0.05 if ymax > ymin else 1.0
ax.set_ylim(ymin - pad, ymax + pad)

finalEquity = y[-1]
pnl = finalEquity - startingEquity
ret = (pnl / startingEquity) * 100.0

ax.text(
    0.02, 0.98,
    f"Starting: ${startingEquity:,.0f}\nFinal: ${finalEquity:,.2f}\nPnL: ${pnl:,.2f} ({ret:.2f}%)",
    transform=ax.transAxes,
    va="top",
    ha="left",
    bbox=dict(boxstyle="round", facecolor="white", alpha=0.85, edgecolor="0.8"),
)

#-- Show the final result
plt.tight_layout()
plt.show()
