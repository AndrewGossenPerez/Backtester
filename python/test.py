import trading_engine as te
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, ScalarFormatter

# ------------------------
# Equty and position over time 
# ------------------------
def plot_equity_and_position(epoch, equity, pos, TIME_SCALE=1, nmax=10_000_000):
    step = max(1, epoch.size // nmax)
    x = (epoch - epoch[0]) / (86400.0 * TIME_SCALE)

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(12,6), sharex=True)

    # --- Equity subplot ---
    ax1.plot(x[::step], equity[::step], color="#1f77b4", label="Equity")
    ax1.fill_between(x[::step], equity[0], equity[::step], color="#1f77b4", alpha=0.1)
    ax1.axhline(equity[0], color="gray", linestyle="--", linewidth=1, label="Start Equity")
    ax1.set_ylabel("Equity ($)")
    ax1.set_title("Equity Over Time")
    ax1.grid(True, alpha=0.3)
    ax1.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"${val:,.0f}"))
    ax1.legend(loc="upper left")

    # --- Position subplot ---
    ax2.plot(x[::step], pos[::step], color="#ff7f0e", label="Position")
    ax2.set_ylabel("Position")
    ax2.set_xlabel("Days elapsed")
    ax2.set_title("Position Over Time")
    ax2.yaxis.set_major_formatter(ScalarFormatter(useOffset=True))
    ax2.grid(True, alpha=0.3)
    ax2.legend(loc="upper left")

    # --- Add PnL summary box on equity plot ---
    start_eq = equity[0]
    final_eq = equity[-1]
    pnl = final_eq - start_eq

    # Max Drawdown
    roll_max = np.maximum.accumulate(equity)
    drawdowns = (roll_max - equity) / roll_max
    max_dd = np.max(drawdowns) if len(drawdowns) > 0 else 0

    textstr = '\n'.join((
        f"Starting Equity: ${start_eq:,.0f}",
        f"Final Equity: ${final_eq:,.0f}",
        f"Total PnL: ${pnl:,.0f} ({(pnl/start_eq)*100:.2f}%)",
        f"Max Drawdown: {max_dd*100:.2f}%"
    ))

    # place a text box in upper right in axes coords
    ax1.text(0.98, 0.95, textstr, transform=ax1.transAxes, fontsize=10,
             verticalalignment='top', horizontalalignment='right',
             bbox=dict(boxstyle='round,pad=0.5', facecolor='white', alpha=0.8))

    plt.tight_layout()
    return fig, ax1, ax2
# ------------------------
# Stock + trend lines and buy/sell markers 
# ------------------------

def plot_stock_trends(epoch, stock, fastN=None, slowN=None, trades=None, TIME_SCALE=1, nmax=10_000_000, pad_fraction=0.2):
    """
    pad_fraction: fraction of stock range to shift trend lines down
    """
    step = max(1, epoch.size // nmax)
    x = (epoch - epoch[0]) / (86400.0 * TIME_SCALE)

    fig, ax = plt.subplots(figsize=(12,4))
    ax.plot(x[::step], stock[::step], color="#9467bd", label="Stock")  # purple

    # Determine padding
    stock_min, stock_max = np.min(stock), np.max(stock)
    padding = pad_fraction * (stock_max - stock_min)

    # --- Trend lines as full-length arrays with NaNs for warmup ---
    if fastN is not None and slowN is not None:
        fast_full = np.full(len(epoch), np.nan)
        slow_full = np.full(len(epoch), np.nan)
        fast_full[-len(fastN):] = fastN
        slow_full[-len(slowN):] = slowN

        # Apply padding downward
        fast_full_padded = fast_full - padding
        slow_full_padded = slow_full - padding

        ax.plot(x, fast_full_padded, color="#2ca02c", linestyle="-", label="Fast EMA")  # green
        ax.plot(x, slow_full_padded, color="#d62728", linestyle="-", label="Slow EMA")  # red

        # Shading between padded trend lines
        ax.fill_between(x, fast_full_padded, slow_full_padded, where=fast_full_padded >= slow_full_padded, facecolor="#2ca02c", alpha=0.1, interpolate=True)
        ax.fill_between(x, fast_full_padded, slow_full_padded, where=fast_full_padded < slow_full_padded, facecolor="#d62728", alpha=0.1, interpolate=True)

    # --- Buy/Sell markers ---
    if trades is not None:
        buys_x, buys_y = [], []
        sells_x, sells_y = [], []
        for t in trades:
            idx = np.searchsorted(epoch, t['epoch'])
            if 0 <= idx < len(stock):
                if t['side'] == 0:  # Buy
                    buys_x.append(x[idx])
                    buys_y.append(stock[idx])
                elif t['side'] == 1:  # Sell
                    sells_x.append(x[idx])
                    sells_y.append(stock[idx])
        if buys_x:
            ax.scatter(buys_x, buys_y, color='green', marker='o', s=50, label='Buy')
        if sells_x:
            ax.scatter(sells_x, sells_y, color='red', marker='o', s=50, label='Sell')

    ax.set_xlabel("Days elapsed")
    ax.set_ylabel("Price ($)")
    ax.set_title("Stock Price & Trend Lines with Buy/Sell Signals")
    ax.grid(True, alpha=0.3)

    # Remove duplicate labels
    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    ax.legend(by_label.values(), by_label.keys())

    plt.tight_layout()
    return fig, ax

# ------------------------
# Main
# ------------------------

def main():
    TIME_SCALE = int(te.TIME_SCALE)
    starting_equity = 1_000
    print("Starting Backtest ...")
    d = te.run_arrays(starting_equity)
    print("Back test completed, plotting...")

    epoch = np.asarray(d["epoch"], dtype=np.float64)
    equity = np.asarray(d["equity"], dtype=np.float64)
    pos = np.asarray(d["pos"], dtype=np.float64)
    stock = np.asarray(d.get("stock", np.zeros_like(epoch)), dtype=np.float64)
    fastN = np.asarray(d.get("fastN", []), dtype=np.float64)
    slowN = np.asarray(d.get("slowN", []), dtype=np.float64)
    trades = d.get("trades", None)  # list of dicts with 'epoch' and 'side'

    # Interactive mode so both windows show simultaneously
    plt.ion()

    # Equity + Position window
    plot_equity_and_position(epoch, equity, pos, TIME_SCALE)

    # Stock + Trend lines + Buy/Sell markers window
    plot_stock_trends(epoch, stock, fastN, slowN, trades, TIME_SCALE)

    # Show all
    plt.show(block=True)

if __name__ == "__main__":
    main()