# plot_equity_pos.py
# Fast plotting using te.run_arrays() (NumPy arrays) from your pybind module.

import trading_engine as te
import numpy as np
import matplotlib.pyplot as plt

def main():
    
    TIME_SCALE = int(te.TIME_SCALE)

    print("Starting Backtest ...")
    starting_equity = 1_000_000
    d = te.run_arrays(starting_equity)
    print("Back test completed, plotting...")

    epoch = np.asarray(d["epoch"], dtype=np.int64)
    equity = np.asarray(d["equity"], dtype=np.float64)
    pos = np.asarray(d["pos"], dtype=np.int64) 

    if epoch.size == 0:
        raise RuntimeError("No data returned")

    # --- Settings
    in_time = True # True: x-axis is days elapsed, False: bar index
    nmax = 10000000 # max points to plot
    step = max(1, epoch.size // nmax)

    # --- Downsample
    epoch_ds = epoch[::step]
    equity_ds = equity[::step]
    pos_ds = pos[::step]

    # X axis
    if in_time:
        title = "Equity & Position vs Time (days elapsed)"
        x = (epoch_ds - epoch_ds[0]) / (86400.0 * TIME_SCALE)
        xlabel = "Days elapsed since first bar"
    else:
        title = "Equity & Position per bar"
        x = np.arange(0, epoch.size, step, dtype=np.int64)
        xlabel = "Bar Index"

    # --- Plot styling
    plt.rcParams.update({
        "figure.dpi": 140,
        "axes.titlesize": 14,
        "axes.labelsize": 12,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
    })

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 5), sharex=True)

    # Equity
    ax1.plot(x, equity_ds, linewidth=1.2)
    ax1.set_title(title)
    ax1.set_ylabel("Equity ($)")
    ax1.grid(True, which="major", alpha=0.30)

    # Equity stats box (use full-res final equity)
    final_equity = float(equity[-1])
    pnl = final_equity - starting_equity
    ret = (pnl / starting_equity) * 100.0

    ax1.text(
        0.0, -0.1,
        f"Starting: ${starting_equity:,.0f}\nFinal: ${final_equity:,.2f}\nPnL: ${pnl:,.2f} ({ret:.2f}%)",
        transform=ax1.transAxes,
        va="top",
        ha="left",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.85, edgecolor="0.8"),
    )

    # Position
    ax2.plot(x, pos_ds, linewidth=1.0)
    ax2.set_xlabel(xlabel)
    ax2.set_ylabel("Position")
    ax2.grid(True, which="major", alpha=0.30)

    # Nice y padding for pos
    pmin,pmax = float(np.min(pos_ds)), float(np.max(pos_ds))
    ppad = (pmax - pmin) * 0.05 if pmax > pmin else 1.0
    if pmax > pmin:
        ax2.set_ylim(pmin, pmax)  # no padding
    else:
        ax2.set_ylim(pmin - 0.5, pmax + 0.5)  # tiny range if all values equal

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()