// volatility_scale.hpp, created by Andrew Gossen.

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

// ------- CONFIG -----
constexpr double ATR_MULT = 1.2; // stop distance = ATR_MULT * ATR
constexpr double RISK_PCT = 0.01; // risk 1% of equity per trade
constexpr double MAX_SHARES_CAP = 10.0;
constexpr double MIN_SHARES_CAP = 1.0;
// --------------------

// Again, an early implementation of a risk model. 
// I intend to use some sort of position sizer using voltaility
// As the algo progresses more factors will be introduced 

template <typename DispatchT>
void VolatilityScaleStop(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {

    if (event.side == trd::Side::Hold) return;
    if (!riskData.barCapacity()) return;

    const double atr = riskData.calculateATR();
    std::cout << "ATR for epoch " << riskData.m_marketState.current.epoch << " is: " << atr << "\n";

    const double stopDist = ATR_MULT * atr;     
    if (!(stopDist > 0.0)) return;

    const double entryPrice = static_cast<double>(riskData.m_marketState.current.close);

    const double equity = static_cast<double>(riskData.m_portfolio.equity(entryPrice));
    if (!(equity > 0.0)) return;

    const double riskDollars = equity * RISK_PCT;
    double shares = riskDollars / stopDist;

    shares = std::clamp(shares, MIN_SHARES_CAP, MAX_SHARES_CAP);

    const long long qtyInternal = std::llround(shares * static_cast<double>(QTY_SCALE));

    const long long minQtyInternal = static_cast<long long>(std::llround(MIN_SHARES_CAP * QTY_SCALE));
    const long long maxQtyInternal = static_cast<long long>(std::llround(MAX_SHARES_CAP * QTY_SCALE));

    const trd::quantity qty = static_cast<trd::quantity>(std::clamp(qtyInternal, minQtyInternal, maxQtyInternal));

    const trd::price stopPrice = (event.side == trd::Side::Buy) ? static_cast<trd::price>(entryPrice - stopDist) : static_cast<trd::price>(entryPrice + stopDist);

    std::optional<stopData> stop = stopData{
        true,
        event.side,
        stopPrice,
        qty
    };

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        event.side,
        qty,
        std::move(stop)
    });

}