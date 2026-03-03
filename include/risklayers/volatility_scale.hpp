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
constexpr double ATR_MULT = 2.0; // stop distance = ATR_MULT * ATR
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

    std::optional<stopData> stop = stopData{
        true,
        event.side,
        1000000,
        QTY_SCALE
    };

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        event.side,
        QTY_SCALE,
        std::move(stop)
    });

}