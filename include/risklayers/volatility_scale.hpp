// volatility_scale.hpp, created by Andrew Gossen.

#pragma once

#include <cmath>
#include <optional>

#include "data/bar.hpp"
#include "core/portfolio.hpp"
#include "pipeline/risk_handler.hpp"

// Skeleton implementation of ATR-based position sizing.

// ------- CONFIG -----
inline constexpr double ATR_MULT = 1.5; // Stop distance = ATR_MULT * ATR
inline constexpr double RISK_PCT = 1.0; // Risk % of equity per trade
inline constexpr double MAX_CAPITAL_PCT = 100.0;    // Max notional exposure as % of equity
inline constexpr double MIN_ATR_PCT = 0.0 / 100.0;  // Optional ATR floor
inline constexpr double MIN_STOP_PCT = 0.0 / 100.0; // Optional stop distance floor
// --------------------

template <typename DispatchT>
void VolatilityScaleStop(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {

    if (event.side == trd::Side::Hold) return;
    if (!riskData.barCapacity()) return;

    const double close = riskData.m_marketState.current.close;
    if (!(close > 0.0) || !std::isfinite(close)) return;

    const double atr = riskData.calculateATR();
    if (!(atr > 0.0) || !std::isfinite(atr)) return;

    const double atrPct = atr / close;
    if (atrPct < MIN_ATR_PCT) return;

    const trd::price equity = riskData.m_portfolio.equity(close);
    if (!(equity > 0.0) || !std::isfinite(equity)) return;

    const double riskCapital = equity * (RISK_PCT / 100.0);

    double stopDistance = ATR_MULT * atr;
    stopDistance = std::max(stopDistance, close * MIN_STOP_PCT);

    if (!(stopDistance > 0.0) || !std::isfinite(stopDistance)) return;

    double targetExposure = riskCapital / stopDistance;

    const double maxExposure = (equity * (MAX_CAPITAL_PCT / 100.0)) / close;
    if (targetExposure > maxExposure) {
        targetExposure = maxExposure;
    }

    double flattenQty = 0.0;

    if (event.side == trd::Side::Sell && riskData.m_portfolio.pos > 0) {
        flattenQty = descaleQty(riskData.m_portfolio.pos);
    } else if (event.side == trd::Side::Buy && riskData.m_portfolio.pos < 0) {
        flattenQty = std::abs(descaleQty(riskData.m_portfolio.pos));
    }

    const double totalOrderQty = targetExposure + flattenQty;
    if (!(totalOrderQty > 0.0) || !std::isfinite(totalOrderQty)) return;

    const double stopLoss =
        (event.side == trd::Side::Sell)
            ? close + stopDistance
            : close - stopDistance;

    const trd::quantity orderQty =
        static_cast<trd::quantity>(QTY_SCALE * totalOrderQty);

    const trd::quantity stopQty =
        static_cast<trd::quantity>(QTY_SCALE * targetExposure);

    if (orderQty <= 0 || stopQty <= 0) return;

    std::optional<stopData> stop = stopData{
        true,
        event.side,
        stopLoss,
        stopQty
    };

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        event.side,
        orderQty,
        std::move(stop)
    });
}