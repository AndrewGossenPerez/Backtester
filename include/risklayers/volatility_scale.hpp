// volatility_scale.hpp, created by Andrew Gossen.

#pragma once
#include <optional>
#include <algorithm>
#include <limits>
#include <iostream>
#include <cmath>
#include <cstdint>
#include <algorithm>

#include "data/bar.hpp"
#include "core/portfolio.hpp"
#include "pipeline/risk_handler.hpp"

// ------- CONFIG -----
inline constexpr double ATR_MULT = 1.4; // stop distance = ATR_MULT * ATR
inline constexpr double RISK_PCT = 1.0; // risk % of equity per trade
inline constexpr double MAX_CAPITAL_PCT = 200; // Clamps the maximum amount sold/bought as % of current equity
// --------------------

// Again, an early implementation of a risk model. 
// I intend to use some sort of position sizer using voltaility
// As the algo progresses more factors will be introduced 

template <typename DispatchT>
void VolatilityScaleStop(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {

    if (event.side == trd::Side::Hold) return;
    if (!riskData.barCapacity()) return;

    const double atr = riskData.calculateATR();

    const trd::price equity = riskData.m_portfolio.equity(riskData.m_marketState.current.close);

    float riskCapital= (equity * (RISK_PCT/100));
    float volatilityMeasure = (ATR_MULT * atr);
    float posSize = (riskCapital/volatilityMeasure);

    float stopLoss = (event.side==trd::Side::Sell) ? 
    (riskData.m_marketState.current.close) + volatilityMeasure : (riskData.m_marketState.current.close) - volatilityMeasure;

    float maxPos = (equity*(MAX_CAPITAL_PCT/100))/(riskData.m_marketState.current.close);

    if (posSize>maxPos) posSize=maxPos;

    trd::quantity qty = static_cast<trd::quantity>(QTY_SCALE * posSize);

    std::optional<stopData> stop = stopData{ // Update the current stop loss, currently only one stop loss is registered at a time ( and is overwritten per trade ) 
        true,
        event.side,
        stopLoss,
        qty
    };

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        event.side,
        qty,
        std::move(stop)
    });

}