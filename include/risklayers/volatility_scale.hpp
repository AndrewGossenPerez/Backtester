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

// Note: Any actual strategies will not be posted to the github, this is a skeleton 

// ------- CONFIG -----
inline constexpr double ATR_MULT = 1.75; // stop distance = ATR_MULT * ATR
inline constexpr double RISK_PCT = 1.35; // risk % of equity per trade
inline constexpr double MAX_CAPITAL_PCT = 100; // Clamps the maximum amount sold/bought as % of current equity
//inline constexpr double MIN_STOP_PCT = 0.0 / 100; // Clamps the minimum volatility measure to 0.2% of the close, 
//inline constexpr double MIN_ATR_PCT = 0.1 / 100;
// --------------------

// Again, an early implementation of a risk model. 
// I intend to use some sort of position sizer using voltaility
// As the algo progresses more factors will be introduced 

template <typename DispatchT>
void VolatilityScaleStop(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {

    if (event.side == trd::Side::Hold) return;
    if (!riskData.barCapacity()) return;

    const double atr = riskData.calculateATR();

    double atrPct = atr / riskData.m_marketState.current.close;
    //if (atrPct < MIN_ATR_PCT) return; // ATR Threshold to avoid deadzones 

    const trd::price equity = riskData.m_portfolio.equity(riskData.m_marketState.current.close);

    double riskCapital= (equity * (RISK_PCT/100));
    double volatilityMeasure = (ATR_MULT * atr);
    //volatilityMeasure = std::max(volatilityMeasure, riskData.m_marketState.current.close * MIN_STOP_PCT); // Clamped to a minimum stop pct to avoid blowup if the atr is small

    if (!(atr > 0.0) || !std::isfinite(atr)) return;
    if (!(volatilityMeasure > 0.0) || !std::isfinite(volatilityMeasure)) return; // Another blowup protection

    double posSize = (riskCapital/volatilityMeasure);

    double stopLoss = (event.side==trd::Side::Sell) ? 
    (riskData.m_marketState.current.close) + volatilityMeasure : (riskData.m_marketState.current.close) - volatilityMeasure;

    double maxPos = (equity*(MAX_CAPITAL_PCT/100))/(riskData.m_marketState.current.close);
    if (posSize>maxPos) posSize=maxPos;

    // Flatten
    double flatten=0;

    if (event.side==trd::Side::Sell && riskData.m_portfolio.pos>0){
       // std::cout <<"Flattening : " <<  riskData.m_portfolio.pos << "\n";
       flatten=descaleQty(riskData.m_portfolio.pos);
        posSize+=flatten;
    } else if(event.side==trd::Side::Buy && riskData.m_portfolio.pos<0){
        flatten=std::abs(descaleQty(riskData.m_portfolio.pos));
        posSize+=flatten;
    }

    trd::quantity stopQty = static_cast<trd::quantity>(QTY_SCALE * (posSize-flatten));
    trd::quantity qty = static_cast<trd::quantity>(QTY_SCALE * posSize);
    if (qty <= 0 || stopQty <= 0 ) return;

    std::optional<stopData> stop = stopData{ // Update the current stop loss, currently only one stop loss is registered at a time ( and is overwritten per trade ) 
        true,
        event.side,
        stopLoss,
        stopQty
    };

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        event.side,
        qty,
        std::move(stop)
    });

}