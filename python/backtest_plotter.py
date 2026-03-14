import trading_engine as te
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import FuncFormatter, ScalarFormatter
from matplotlib.widgets import Button


def _safe_ratio(a, b, default=0.0):
    return a / b if b != 0 else default


def _compute_metrics(equity, pos, x, bars_per_day=78.0):
    equity = np.asarray(equity, dtype=np.float64)
    pos = np.asarray(pos, dtype=np.float64)

    start_eq = float(equity[0]) if equity.size else 0.0
    final_eq = float(equity[-1]) if equity.size else 0.0
    pnl = final_eq - start_eq
    total_return = _safe_ratio(pnl, start_eq, 0.0)

    roll_max = np.maximum.accumulate(equity) if equity.size else np.array([], dtype=np.float64)
    drawdowns = np.where(roll_max > 0, (equity - roll_max) / roll_max, 0.0) if equity.size else np.array([], dtype=np.float64)
    max_dd = float(np.min(drawdowns)) if drawdowns.size else 0.0

    returns = np.diff(equity) / equity[:-1] if equity.size > 1 else np.array([], dtype=np.float64)
    ann_factor = np.sqrt(252.0 * bars_per_day)

    if returns.size > 1 and np.std(returns) > 0:
        sharpe = float(np.mean(returns) / np.std(returns) * ann_factor)
    else:
        sharpe = 0.0

    downside = returns[returns < 0]
    if downside.size > 0 and np.std(downside) > 0:
        sortino = float(np.mean(returns) / np.std(downside) * ann_factor)
    else:
        sortino = 0.0

    days_elapsed = float(x[-1] - x[0]) if x.size > 1 else 0.0
    abs_pos = np.abs(pos)
    avg_abs_pos = float(np.mean(abs_pos)) if abs_pos.size else 0.0
    max_abs_pos = float(np.max(abs_pos)) if abs_pos.size else 0.0

    pos_changes = np.diff(pos) if pos.size > 1 else np.array([], dtype=np.float64)
    turnover = float(np.sum(np.abs(pos_changes))) if pos_changes.size else 0.0
    trades_est = int(np.count_nonzero(pos_changes)) if pos_changes.size else 0
    trades_per_day = _safe_ratio(trades_est, days_elapsed, 0.0) if days_elapsed > 0 else 0.0

    return {
        "start_eq": start_eq,
        "final_eq": final_eq,
        "pnl": pnl,
        "total_return": total_return,
        "drawdowns": drawdowns,
        "max_dd": max_dd,
        "returns": returns,
        "sharpe": sharpe,
        "sortino": sortino,
        "avg_abs_pos": avg_abs_pos,
        "max_abs_pos": max_abs_pos,
        "turnover": turnover,
        "trades_est": trades_est,
        "trades_per_day": trades_per_day,
    }


def plot_all(
    epoch,
    stock,
    equity,
    pos,
    fastN=None,
    slowN=None,
    atrs=None,
    trades=None,
    TIME_SCALE=1,
    nmax=10_000_000,
):
    epoch = np.asarray(epoch, dtype=np.float64)
    stock = np.asarray(stock, dtype=np.float64)
    equity = np.asarray(equity, dtype=np.float64)
    pos = np.asarray(pos, dtype=np.float64)

    if epoch.size == 0:
        raise ValueError("epoch is empty")

    step = max(1, len(epoch) // nmax)
    x = np.arange(len(epoch), dtype=np.float64) / 78.0  # 78 x 5-min bars per trading day

    fig = plt.figure(figsize=(12, 8))
    gs = fig.add_gridspec(
        5, 1,
        height_ratios=[1.8, 0.9, 1.2, 0.9, 1.0],
        hspace=0.55
    )

    ax_stock = fig.add_subplot(gs[0])
    ax_atr = fig.add_subplot(gs[1], sharex=ax_stock)
    ax_equity = fig.add_subplot(gs[2], sharex=ax_stock)
    ax_drawdown = fig.add_subplot(gs[3], sharex=ax_stock)
    ax_pos = fig.add_subplot(gs[4], sharex=ax_stock)

    # ---------------- PRICE PANEL ----------------
    ax_stock.plot(
        x[::step],
        stock[::step],
        label="Stock",
        color="#9467bd",
        alpha=0.85,
        linewidth=1.5,
    )

    buy_scatter = None
    sell_scatter = None

    if trades is not None:
        buys_x, buys_y, sells_x, sells_y = [], [], [], []

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
            buy_scatter = ax_stock.scatter(
                buys_x, buys_y,
                marker="^",
                s=28,
                alpha=0.70,
                label="Buy",
                zorder=4,
            )

        if sells_x:
            sell_scatter = ax_stock.scatter(
                sells_x, sells_y,
                marker="v",
                s=28,
                alpha=0.70,
                label="Sell",
                zorder=4,
            )

    price_span = np.nanmax(stock) - np.nanmin(stock)
    trend_offset = 0.002 * price_span if np.isfinite(price_span) and price_span > 0 else 0.0

    fast_full = None
    slow_full = None

    if fastN is not None and len(fastN) > 0:
        fastN = np.asarray(fastN, dtype=np.float64)
        fast_full = np.full(len(stock), np.nan)
        fast_full[-len(fastN):] = fastN
        fast_full -= trend_offset
        ax_stock.plot(x, fast_full, label="Fast Lookback", color="#2ca02c", linewidth=1.6)

    if slowN is not None and len(slowN) > 0:
        slowN = np.asarray(slowN, dtype=np.float64)
        slow_full = np.full(len(stock), np.nan)
        slow_full[-len(slowN):] = slowN
        slow_full -= trend_offset
        ax_stock.plot(x, slow_full, label="Slow Lookback", color="#d62728", linewidth=1.6)

    if fast_full is not None and slow_full is not None:
        valid = np.isfinite(fast_full) & np.isfinite(slow_full)

        ax_stock.fill_between(
            x, fast_full, slow_full,
            where=valid & (fast_full >= slow_full),
            interpolate=True,
            alpha=0.16,
            color="#2ca02c",
        )

        ax_stock.fill_between(
            x, fast_full, slow_full,
            where=valid & (slow_full > fast_full),
            interpolate=True,
            alpha=0.16,
            color="#d62728",
        )

    ax_stock.set_title("Backtest results")
    ax_stock.set_ylabel("Price ($)")
    ax_stock.grid(True, alpha=0.35)
    ax_stock.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))

    # ---------------- ATR PANEL ----------------
    atr_line = None
    if atrs is not None and len(atrs) > 0:
        atrs = np.asarray(atrs, dtype=np.float64)
        atr_full = np.full(len(stock), np.nan)
        atr_full[-len(atrs):] = atrs
        atr_line, = ax_atr.plot(
            x, atr_full,
            label="ATR",
            color="#f39c12",
            linewidth=1.5,
        )

    ax_atr.set_ylabel("ATR")
    ax_atr.grid(True, alpha=0.30)
    ax_atr.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))

    # ---------------- METRICS ----------------
    metrics = _compute_metrics(equity, pos, x)
    drawdowns = metrics["drawdowns"]

    # ---------------- EQUITY PANEL ----------------
    ax_equity.plot(
        x[::step],
        equity[::step],
        color="#1f77b4",
        label="Equity",
        linewidth=1.5,
    )

    ax_equity.fill_between(
        x[::step],
        equity[0],
        equity[::step],
        color="#1f77b4",
        alpha=0.10,
    )

    ax_equity.axhline(
        equity[0],
        color="gray",
        linestyle="--",
        linewidth=1.0,
        label="Start Equity",
    )

    ax_equity.set_ylabel("Equity ($)")
    ax_equity.grid(True, alpha=0.30)
    ax_equity.yaxis.set_major_formatter(FuncFormatter(lambda val, _: f"${val:,.0f}"))
    ax_equity.legend(loc="upper left", bbox_to_anchor=(1.0, 1.58))

    stats = (
        f"Start: ${metrics['start_eq']:,.0f}\n"
        f"Final: ${metrics['final_eq']:,.0f}\n"
        f"PnL: ${metrics['pnl']:,.0f} ({metrics['total_return']*100:.2f}%)\n"
        f"Max DD: {abs(metrics['max_dd'])*100:.2f}%\n"
        f"Sharpe: {metrics['sharpe']:.2f}\n"
        f"Sortino: {metrics['sortino']:.2f}\n"
        f"Trades/day: {metrics['trades_per_day']:.2f}"
    )

    ax_equity.text(
        1.015, 0.85, stats,
        transform=ax_equity.transAxes,
        va="top", ha="left",
        bbox=dict(boxstyle="round", facecolor="white", alpha=0.88, edgecolor="0.7"),
    )

    # ---------------- DRAWDOWN PANEL ----------------
    dd_pct = drawdowns * 100.0 if drawdowns.size else np.array([], dtype=np.float64)

    if dd_pct.size > 0:
        ax_drawdown.plot(
            x[::step],
            dd_pct[::step],
            color="#d62728",
            label="Drawdown",
            linewidth=1.5,
        )

        ax_drawdown.fill_between(
            x[::step],
            dd_pct[::step],
            0,
            color="#d62728",
            alpha=0.20,
        )

        dd_idx = int(np.argmin(drawdowns))
        ax_drawdown.scatter(
            x[dd_idx],
            dd_pct[dd_idx],
            color="black",
            s=42,
            zorder=5,
            label="Max DD",
        )

        dd_min = float(np.min(dd_pct))
        ax_drawdown.set_ylim(dd_min * 1.02, 0.2)

    ax_drawdown.axhline(0, color="black", linewidth=1.0)
    ax_drawdown.set_ylabel("DD (%)")
    ax_drawdown.grid(True, alpha=0.30)
    ax_drawdown.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))

    # ---------------- POSITION PANEL ----------------
    ax_pos.step(
        x[::step],
        pos[::step],
        where="post",
        label="Position",
        linewidth=1.5,
    )

    ax_pos.axhline(0, color="gray", linewidth=1.0, alpha=0.8)
    ax_pos.set_ylabel("Position")
    ax_pos.set_xlabel("Days Elapsed")
    ax_pos.grid(True, alpha=0.30)
    ax_pos.legend(loc="upper left", bbox_to_anchor=(1.01, 1.0))
    ax_pos.yaxis.set_major_formatter(ScalarFormatter(useOffset=True))

    # ---------------- LAYOUT ----------------
    plt.subplots_adjust(right=0.80, bottom=0.12)

    # ---------------- TOGGLE BUY/SELL BUTTON ----------------
    ax_button = plt.axes([0.82, 0.02, 0.16, 0.05])
    toggle_button = Button(ax_button, "Hide buy/sells")

    if buy_scatter is not None:
        buy_scatter.set_visible(True)
    if sell_scatter is not None:
        sell_scatter.set_visible(True)

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

    epoch = np.asarray(d["epoch"], dtype=np.float64)
    equity = np.asarray(d["equity"], dtype=np.float64)
    pos = np.asarray(d["pos"], dtype=np.float64)
    stock = np.asarray(d.get("stock", np.zeros_like(epoch)), dtype=np.float64)

    fastN = np.asarray(d.get("fastN", []), dtype=np.float64)
    slowN = np.asarray(d.get("slowN", []), dtype=np.float64)
    atrs = np.asarray(d.get("atrs", []), dtype=np.float64)
    trades = d.get("trades", None)

    fig = plot_all(
        epoch=epoch,
        stock=stock,
        equity=equity,
        pos=pos,
        fastN=fastN,
        slowN=slowN,
        atrs=atrs,
        trades=trades,
        TIME_SCALE=TIME_SCALE,
    )
    plt.show()


if __name__ == "__main__":
    main()