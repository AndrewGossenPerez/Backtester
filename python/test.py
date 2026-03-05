import trading_engine as te
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter,ScalarFormatter
from matplotlib.widgets import Button

def plot_all(epoch, stock, equity, pos, fastN=None, slowN=None, atrs=None, trades=None, TIME_SCALE=1, nmax=10_000_000):

    epoch = np.asarray(epoch,  dtype=np.float64)
    stock = np.asarray(stock,  dtype=np.float64)
    equity = np.asarray(equity, dtype=np.float64)
    pos = np.asarray(pos,    dtype=np.float64)

    step = max(1, len(epoch) // nmax)
    x = (epoch - epoch[0]) / (86400.0 * TIME_SCALE)


    fig = plt.figure(figsize=(12, 6))

    # Spacing between panels 
    gs = fig.add_gridspec(
        5, 1,
        height_ratios=[1.7, 1.0, 1.0, 1.2, 1.0],
        hspace=0.55
    )

    ax_stock = fig.add_subplot(gs[0])
    ax_ema = fig.add_subplot(gs[1], sharex=ax_stock)
    ax_atr = fig.add_subplot(gs[2], sharex=ax_stock)
    ax_equity = fig.add_subplot(gs[3], sharex=ax_stock)
    ax_pos = fig.add_subplot(gs[4], sharex=ax_stock)

    # Stock Panel
    ax_stock.plot(x[::step], stock[::step], label="Stock", color="#9467bd")

    buy_scatter = None
    sell_scatter = None
    if trades is not None:
        buys_x, buys_y, sells_x, sells_y = [], [], [], []
        for t in trades:
            idx = np.searchsorted(epoch, t["epoch"])
            if 0 <= idx < len(stock):
                if t["side"] == 0:
                    buys_x.append(x[idx]);  buys_y.append(stock[idx])
                elif t["side"] == 1:
                    sells_x.append(x[idx]); sells_y.append(stock[idx])

        if buys_x:
            buy_scatter = ax_stock.scatter(buys_x, buys_y, marker="^", s=70, label="Buy")
        if sells_x:
            sell_scatter = ax_stock.scatter(sells_x, sells_y, marker="v", s=70, label="Sell")

    ax_stock.set_title("Backtest results")
    ax_stock.set_ylabel("Price ($)")
    ax_stock.grid(True, alpha=0.3)
    ax_stock.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))

    # Trend lines 
    if fastN is not None and len(fastN) > 0:
        fastN = np.asarray(fastN, dtype=np.float64)
        fast_full = np.full(len(stock), np.nan)
        fast_full[-len(fastN):] = fastN
        ax_ema.plot(x, fast_full, label="Fast Lookback", color="#2ca02c")

    if slowN is not None and len(slowN) > 0:
        slowN = np.asarray(slowN, dtype=np.float64)
        slow_full = np.full(len(stock), np.nan)
        slow_full[-len(slowN):] = slowN
        ax_ema.plot(x, slow_full, label="Slow Lookback", color="#d62728")

    ax_ema.set_ylabel("EMA")
    ax_ema.grid(True, alpha=0.3)
    ax_ema.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))

    # ATR
    atr_line = None
    if atrs is not None and len(atrs) > 0:
        atrs = np.asarray(atrs, dtype=np.float64)
        atr_full = np.full(len(stock), np.nan)
        atr_full[-len(atrs):] = atrs
        atr_line, = ax_atr.plot(x, atr_full, label="ATR", color="#f39c12", linewidth=2)

    ax_atr.set_ylabel("ATR")
    ax_atr.grid(True, alpha=0.3)
    ax_atr.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))

    # Equity 

    step = max(1, epoch.size // nmax)
    x = (epoch - epoch[0]) / (86400.0 * TIME_SCALE)

    ax_equity.plot(x[::step], equity[::step], color="#1f77b4", label="Equity")
    ax_equity.fill_between(x[::step], equity[0], equity[::step], color="#1f77b4", alpha=0.1)
    ax_equity.axhline(equity[0], color="gray", linestyle="--", linewidth=1, label="Start Equity")
    ax_equity.set_ylabel("Equity ($)")
    ax_equity.grid(True, alpha=0.3)
    ax_equity.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"${val:,.0f}"))
    ax_equity.legend(loc="upper left", bbox_to_anchor=(1.0, 1.45))
    
    # Equity stats 
    #print("Ending equity : ") 
    #print(x[400])

    start_eq = float(equity[0])
    final_eq = float(equity[-1])
    pnl = final_eq - start_eq

    roll_max = np.maximum.accumulate(equity)
    drawdowns = np.where(roll_max > 0, (roll_max - equity) / roll_max, 0.0)
    max_dd = float(np.max(drawdowns)) if drawdowns.size else 0.0

    stats = (
        f"Start: ${start_eq:,.0f}\n"
        f"Final: ${final_eq:,.0f}\n"
        f"PnL: ${pnl:,.0f} ({(pnl/start_eq)*100:.2f}%)\n"
        f"Max DD: {max_dd*100:.2f}%"
    )

    ax_equity.text(
        1.015, 0.55, stats,
        transform=ax_equity.transAxes,
        va="top", ha="left",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.85, edgecolor="0.7")
    )

    # Pos 

    ax_pos.plot(x[::step], pos[::step], label="Position")
    ax_pos.set_ylabel("Position")
    ax_pos.set_xlabel("Days Elapsed")
    ax_pos.grid(True, alpha=0.3)
    ax_pos.legend(loc="upper left", bbox_to_anchor=(1.01, 0.9))
    ax_pos.yaxis.set_major_formatter(ScalarFormatter(useOffset=True))

    # Hide x tick labels for upper panels
    for ax in (ax_stock, ax_ema, ax_atr, ax_equity):
        plt.setp(ax.get_xticklabels(), visible=False)

    # room for right-side legends
    plt.subplots_adjust(right=0.80, bottom=0.12)

    # Toggle buy/sell markers
    ax_button = plt.axes([0.82, 0.02, 0.16, 0.05])
    toggle_button = Button(ax_button, "Show buy/sells")

    def toggle_trades(event):
        any_visible = False
        if buy_scatter is not None and buy_scatter.get_visible():
            any_visible = True
        if sell_scatter is not None and sell_scatter.get_visible():
            any_visible = True

        new_state = not any_visible

        if buy_scatter is not None:
            buy_scatter.set_visible(new_state)
        if sell_scatter is not None:
            sell_scatter.set_visible(new_state)

        toggle_button.label.set_text("Hide buy/sells" if new_state else "Show buy/sells")
        fig.canvas.draw_idle()

    toggle_button.on_clicked(toggle_trades)

    fig._toggle_button = toggle_button
    fig._buy_scatter = buy_scatter
    fig._sell_scatter = sell_scatter
    fig._atr_line = atr_line

    return fig

def main():

    TIME_SCALE = int(te.TIME_SCALE)
    starting_equity = 1_000

    print("Starting Backtest...")
    d = te.run_arrays(starting_equity)
    print("Backtest complete. Plotting...")

    epoch  = np.asarray(d["epoch"], dtype=np.float64)
    equity = np.asarray(d["equity"], dtype=np.float64)
    pos = np.asarray(d["pos"], dtype=np.float64)
    stock = np.asarray(d.get("stock", np.zeros_like(epoch)), dtype=np.float64)

    fastN = np.asarray(d.get("fastN", []), dtype=np.float64)
    slowN = np.asarray(d.get("slowN", []), dtype=np.float64)
    atrs = np.asarray(d.get("atrs", []), dtype=np.float64)
    trades = d.get("trades", None)

    fig = plot_all(epoch, stock, equity, pos, fastN, slowN, atrs, trades, TIME_SCALE)
    plt.show()

if __name__ == "__main__":
    main()