// volatility_scale2.hpp
#pragma once

#include <optional>
#include <algorithm>
#include <limits>
#include <iostream>
#include <cmath>
#include <cstdint>

#include "data/bar.hpp"
#include "core/portfolio.hpp"
#include "pipeline/risk_handler.hpp"

namespace risklayers::volscale2 {

struct Config {
    static constexpr double atrMult        = 2.5;    // stop distance = atrMult * ATR
    static constexpr double riskPct        = 0.30;   // % of equity risked per trade
    static constexpr double maxCapitalPct  = 100.0;  // max notional as % of equity
    static constexpr double minAtrFloorPct = 0.001;  // minimum stop distance as % of price
    static constexpr bool   enableLog      = true;
};

inline bool isFinitePositive(double x) {
    return std::isfinite(x) && x > 0.0;
}

inline double riskBudget(double equity) {
    return equity * (Config::riskPct / 100.0);
}

inline double stopDistance(double price, double atr) {
    const double atrDist   = Config::atrMult * atr;
    const double floorDist = price * Config::minAtrFloorPct;
    return std::max(atrDist, floorDist);
}

inline double stopPrice(trd::Side side, double entryPrice, double stopDist) {
    return (side == trd::Side::Sell)
        ? entryPrice + stopDist
        : entryPrice - stopDist;
}

inline double maxUnitsFromCapital(double equity, double price) {
    const double maxNotional = equity * (Config::maxCapitalPct / 100.0);
    return maxNotional / price;
}

inline void logDecision(
    double equity,
    double price,
    double atr,
    double stopDist,
    double stopPx,
    double rawUnits,
    double cappedUnits,
    trd::Side side,
    trd::quantity qty
) {
    if constexpr (!Config::enableLog) return;

    std::cout << "\n-- VolatilityScaleStop v2 --\n"
              << "Equity        : " << equity << "\n"
              << "Price         : " << price << "\n"
              << "ATR           : " << atr << "\n"
              << "Stop distance : " << stopDist << "\n"
              << "Stop price    : " << stopPx << "\n"
              << "Raw units     : " << rawUnits << "\n"
              << "Capped units  : " << cappedUnits << "\n"
              << "Final qty     : " << qty << "\n"
              << "Side          : " << (side == trd::Side::Sell ? "Sell" : "Buy")
              << "\n";
}

template <typename DispatchT>
void VolatilityScaleStop(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {
    if (event.side == trd::Side::Hold) return;
    if (!riskData.barCapacity()) return;

    const double price = static_cast<double>(riskData.m_marketState.current.close);
    if (!isFinitePositive(price)) return;

    const double atr = static_cast<double>(riskData.calculateATR());
    if (!isFinitePositive(atr)) return;

    const double equity = static_cast<double>(riskData.m_portfolio.equity(price));
    if (!isFinitePositive(equity)) return;

    const double dollarsAtRisk = riskBudget(equity);
    if (!isFinitePositive(dollarsAtRisk)) return;

    const double stopDist = stopDistance(price, atr);
    if (!isFinitePositive(stopDist)) return;

    const double rawUnits = dollarsAtRisk / stopDist;
    if (!isFinitePositive(rawUnits)) return;

    const double maxUnits = maxUnitsFromCapital(equity, price);
    if (!isFinitePositive(maxUnits)) return;

    const double finalUnits = std::min(rawUnits, maxUnits);
    if (!isFinitePositive(finalUnits)) return;

    const auto qty = static_cast<trd::quantity>(
        std::max(0.0, std::floor(finalUnits * static_cast<double>(QTY_SCALE)))
    );
    if (qty <= 0) return;

    const double stopPx = stopPrice(event.side, price, stopDist);
    if (!isFinitePositive(stopPx)) return;

    std::optional<stopData> stop = stopData{
        true,
        event.side,
        static_cast<trd::price>(stopPx),
        qty
    };

    logDecision(
        equity,
        price,
        atr,
        stopDist,
        stopPx,
        rawUnits,
        finalUnits,
        event.side,
        qty
    );

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        event.side,
        qty,
        std::move(stop)
    });
}

} // namespace risklayers::volscale2