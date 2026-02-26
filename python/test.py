import trading_engine as te
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, ScalarFormatter

def main():
    TIME_SCALE = int(te.TIME_SCALE)

    print("Starting Backtest ...")
    starting_equity = 1_000
    d = te.run_arrays(starting_equity)
    print("Back test completed, plotting...")

    epoch = np.asarray(d["epoch"], dtype=np.float64)
    equity = np.asarray(d["equity"], dtype=np.float64)
    pos = np.asarray(d["pos"], dtype=np.float64)

    if epoch.size == 0:
        raise RuntimeError("No data returned")

    # --- Settings
    in_time = True  # x-axis is days elapsed
    nmax = 10_000_000  # max points to plot
    step = max(1, epoch.size // nmax)

    # --- Downsample
    epoch_ds = epoch[::step]
    equity_ds = equity[::step]
    pos_ds = pos[::step]

    # X axis
    if in_time:
        title = "Equity & Position vs Time"
        x = (epoch_ds - epoch_ds[0]) / (86400.0 * TIME_SCALE)
        xlabel = "Days elapsed since first bar"
    else:
        title = "Equity & Position per bar"
        x = np.arange(0, epoch.size, step, dtype=np.int64)
        xlabel = "Bar Index"

    # --- Plot styling
    plt.rcParams.update({
        "figure.dpi": 140,
        "axes.titlesize": 16,
        "axes.labelsize": 12,
        "xtick.labelsize": 10,
        "ytick.labelsize": 10,
        "lines.linewidth": 1.5
    })

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 6), sharex=True)

    # --- Equity plot
    ax1.plot(x, equity_ds, color="#1f77b4", label="Equity")
    ax1.fill_between(x, starting_equity, equity_ds, color="#1f77b4", alpha=0.1)
    ax1.axhline(starting_equity, color="gray", linestyle="--", linewidth=1, label="Start Equity")

    # --- Fast & Slow MAs
    if "fastN" in d and "slowN" in d:
        fast = np.asarray(d["fastN"], dtype=np.float64)
        slow = np.asarray(d["slowN"], dtype=np.float64)

        # Downsample
        fast_ds = fast[::step]
        slow_ds = slow[::step]

        # Align lengths
        min_len = min(len(x), len(fast_ds), len(slow_ds))
        x_plot = x[:min_len]
        fast_plot = fast_ds[:min_len]
        slow_plot = slow_ds[:min_len]

        # Plot lines
        ax1.plot(x_plot, fast_plot, color="#2ca02c", linestyle="-", label="Fast MA")
        ax1.plot(x_plot, slow_plot, color="#d62728", linestyle="-", label="Slow MA")

        # Fill shading between fast/slow MA
        ax1.fill_between(
            x_plot, fast_plot, slow_plot,
            where=fast_plot >= slow_plot,
            facecolor="#2ca02c", alpha=0.1,
            interpolate=True
        )
        ax1.fill_between(
            x_plot, fast_plot, slow_plot,
            where=fast_plot < slow_plot,
            facecolor="#d62728", alpha=0.1,
            interpolate=True
        )

    ax1.set_title(title)
    ax1.set_ylabel("Equity ($)")
    ax1.grid(True, which="major", alpha=0.3)
    ax1.yaxis.set_major_formatter(FuncFormatter(lambda x_val, _: f"${x_val:,.0f}"))

    # Equity stats box
    final_equity = float(equity[-1])
    pnl = final_equity - starting_equity
    ret = (pnl / starting_equity) * 100.0

    ax1.text(
        1.0, -0.05,
        f"Start: ${starting_equity:,.0f}\nFinal: ${final_equity:,.0f}\nPnL: ${pnl:,.0f} ({ret:.2f}%)",
        transform=ax1.transAxes,
        va="bottom",
        ha="right",
        bbox=dict(boxstyle="round,pad=0.4", facecolor="white", alpha=0.85, edgecolor="0.8"),
    )

    ax1.legend(loc="upper left")

    # --- Position plot
    ax2.plot(x, pos_ds, color="#ff7f0e", label="Position")
    ax2.set_xlabel(xlabel)
    ax2.set_ylabel("Position")
    ax2.grid(True, which="major", alpha=0.3)
    ax2.yaxis.set_major_formatter(ScalarFormatter(useOffset=True))

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    main()