
# Analysis layer for now, protoype. Will likely swap to a different library later 

import trading_engine as te
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, ScalarFormatter
from matplotlib.widgets import Button


def plot_all(epoch, stock, equity, pos, fastN=None, slowN=None, atrs=None, trades=None, TIME_SCALE=1, nmax=10_000_000):  

    trend_padding=(equity/30)

    step = max(1, len(epoch) // nmax)
    x = (epoch - epoch[0]) / (86400.0 * TIME_SCALE)

    fig = plt.figure(figsize=(12, 6))
    gs = fig.add_gridspec(3, 1, height_ratios=[1.7, 1.2, 1])

    ax_stock = fig.add_subplot(gs[0])
    ax_equity = fig.add_subplot(gs[1], sharex=ax_stock)
    ax_pos = fig.add_subplot(gs[2], sharex=ax_stock)

    #Stock price 
    ax_stock.plot(x[::step], stock[::step], label="Stock", color="#9467bd")

    # Trend Lines 
    if fastN is not None and len(fastN) > 0:
        fast_full = np.full(len(stock), np.nan)
        fast_full[-len(fastN):] = fastN
        ax_stock.plot(x, fast_full-trend_padding, label="Fast Lookback", color="#2ca02c")

    if slowN is not None and len(slowN) > 0:
        slow_full = np.full(len(stock), np.nan)
        slow_full[-len(slowN):] = slowN
        ax_stock.plot(x, slow_full-trend_padding, label="Slow Lookback", color="#d62728")

    # Buy/sell markers
    buy_scatter = None
    sell_scatter = None

    if trades is not None:
        buys_x, buys_y = [], []
        sells_x, sells_y = [], []

        for t in trades:
            idx = np.searchsorted(epoch, t["epoch"])
            if 0 <= idx < len(stock):
                if t["side"] == 0:
                    buys_x.append(x[idx])
                    buys_y.append(stock[idx])
                elif t["side"] == 1:
                    sells_x.append(x[idx])
                    sells_y.append(stock[idx])

        if buys_x:
            buy_scatter = ax_stock.scatter(buys_x, buys_y, marker="^", s=70, label="Buy")
        if sells_x:
            sell_scatter = ax_stock.scatter(sells_x, sells_y, marker="v", s=70, label="Sell")

    ax_stock.set_title("Backtest results")
    ax_stock.set_ylabel("Price ($)")
    ax_stock.grid(True, alpha=0.3)


    # ATR on its own y-axis to preserve shape
    ax_atr = ax_stock.twinx()
    ax_atr.set_zorder(ax_stock.get_zorder() - 1)   # keep price on top
    ax_atr.patch.set_alpha(0.0)                    # transparent background

    # ATR amplified relative to stock 
    atr_line = None

        # --- ATR amplified relative to stock volatility
    if atrs is not None and len(atrs) > 0:
        atrs = np.asarray(atrs, dtype=np.float64)
        atr_full = np.full(len(stock), np.nan)
        atr_full[-len(atrs):] = atrs

        valid = np.isfinite(stock) & np.isfinite(atr_full)
        if np.any(valid):

            # Use std dev instead of total range (better scaling)
            stock_vol = np.nanstd(stock[valid])
            atr_vol   = np.nanstd(atr_full[valid])
            atr_vol = atr_vol if atr_vol > 0 else 1.0

            amp = (stock_vol / atr_vol) * 0.1

            atr_scaled = atr_full * amp

            # Keep it slightly above stock everywhere
            gap = stock_vol * 0.2
            shift = np.nanmax(stock[valid] - atr_scaled[valid]) + gap

            atr_line, = ax_stock.plot(
                x,
                atr_scaled + shift,
                label=f"ATR (scaled x{amp:.2f})",
                color="#f39c12",
                linewidth=2
            )

    ax_atr.set_yticks([])
    ax_atr.spines["right"].set_alpha(0.2)

    ax_atr.set_ylabel("ATR")
    ax_atr.grid(True, alpha=0.3)

    # Legend panel
    ax_stock.legend(loc="upper left", bbox_to_anchor=(1.01, 1))

    #Equity  
    ax_equity.plot(x[::step], equity[::step], label="Equity")
    ax_equity.axhline(equity[0], linestyle="--", linewidth=1, label="Start")
    ax_equity.set_ylabel("Equity ($)")
    ax_equity.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"${val:,.0f}"))
    ax_equity.grid(True, alpha=0.3)

    # Stats
    start_eq = equity[0]
    final_eq = equity[-1]
    pnl = final_eq - start_eq

    roll_max = np.maximum.accumulate(equity)
    drawdowns = (roll_max - equity) / roll_max
    max_dd = np.max(drawdowns) if len(drawdowns) > 0 else 0

    stats = (
        f"Start: ${start_eq:,.0f}\n"
        f"Final: ${final_eq:,.0f}\n"
        f"PnL: ${pnl:,.0f} ({(pnl/start_eq)*100:.2f}%)\n"
        f"Max DD: {max_dd*100:.2f}%"
    )

    # Stats moved outside so it doesn't overlap data
    ax_equity.text(
        1.01, 0.95,
        stats,
        transform=ax_equity.transAxes,
        verticalalignment="top",
        horizontalalignment="left",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.8)
    )

    #Position 
    ax_pos.plot(x[::step], pos[::step], label="Position")
    ax_pos.set_ylabel("Position")
    ax_pos.set_xlabel("Days Elapsed")
    ax_pos.yaxis.set_major_formatter(ScalarFormatter(useOffset=True))
    ax_pos.grid(True, alpha=0.3)

    # Remove duplicates
    for ax in [ax_equity, ax_pos, ax_atr]:
        handles, labels = ax.get_legend_handles_labels()
        if handles:
            ax.legend()

    plt.tight_layout()

    # Make room for right-side legend/stats and bottom button
    plt.subplots_adjust(hspace=0.15, right=0.80, bottom=0.12)

    #Toggle Button (hide/show buy/sell markers)
    ax_button = plt.axes([0.85, 0.02, 0.12, 0.05])
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

    # ATR Toggle 
    ax_atr_button = plt.axes([0.85, 0.08, 0.12, 0.05])  # above the buy/sell button
    atr_button = Button(ax_atr_button, "Hide ATR")

    def toggle_atr(event):
        if atr_line is None:
            return
        new_state = not atr_line.get_visible()
        atr_line.set_visible(new_state)
        atr_button.label.set_text("Hide ATR" if new_state else "Show ATR")
        fig.canvas.draw_idle()

    atr_button.on_clicked(toggle_atr)

    # Prevent garbage collection
    fig._atr_button = atr_button
    fig._atr_line = atr_line

    toggle_button.on_clicked(toggle_trades)

    # Prevent garbage collection of widgets/artists
    fig._toggle_button = toggle_button
    fig._buy_scatter = buy_scatter
    fig._sell_scatter = sell_scatter

    return fig


def main():

    TIME_SCALE = int(te.TIME_SCALE)
    starting_equity = 1_000

    print("Starting Backtest...")
    d = te.run_arrays(starting_equity)
    print("Backtest complete. Plotting...")

    epoch = np.asarray(d["epoch"], dtype=np.float64)
    equity = np.asarray(d["equity"], dtype=np.float64)
    pos = np.asarray(d["pos"], dtype=np.float64)
    stock = np.asarray(d.get("stock", np.zeros_like(epoch)), dtype=np.float64)
    fastN = np.asarray(d.get("fastN", []), dtype=np.float64)
    slowN = np.asarray(d.get("slowN", []), dtype=np.float64)
    trades = d.get("trades", None)
    atrs = np.asarray(d.get("atrs", []), dtype=np.float64)

    fig = plot_all(epoch, stock, equity, pos, fastN, slowN, atrs, trades, TIME_SCALE)
    plt.show()

if __name__ == "__main__":
    main()