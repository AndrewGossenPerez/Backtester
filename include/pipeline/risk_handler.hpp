// risk_handler.hpp
#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>

#include "events/events.hpp"
#include "core/portfolio.hpp"
#include "data/config.hpp" // for QTY_SCALE
#include "data/market_state.hpp"

template <typename DispatchT>
struct RiskData {

    // The backtest creates one of this struct, which is passed onto each 
    // risk model function 
    
    RiskData(Portfolio& portfolio, trd::MarketState& marketState,DispatchT& dispatcher)
    : m_portfolio(portfolio), m_marketState(marketState), m_dispatcher(dispatcher) {}

    // --- Config used by risk models --- 

    double riskFactor=0.01;
    double stopFactor=0.02;
    double volatility=1.0;

    // -----------------------------------


    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    DispatchT& m_dispatcher;

    // Function pointer to the current risk Model 
    using riskModel=void (*)(RiskData&,  const events::SignalEvent& event);
    riskModel current=nullptr;

};

template <typename DispatchT>
void FixedFractionalRisk(RiskData<DispatchT>& riskData, const events::SignalEvent& event){

    // Risk the same % of equity each time 
    if (event.side == trd::Side::Sell) return; // Currently just long only, will add shorts later 
    if (riskData.m_portfolio.pos != 0) { return; }

    const trd::Bar& prev = riskData.m_marketState.prev;
    const trd::Bar& current = riskData.m_marketState.current;

    const double equity = riskData.m_portfolio.equity(current.close);
    const double allowableRisk = equity * riskData.riskFactor;
    if (allowableRisk <= 0.0) { std::cout << " \n Hit allowable risk "; return; } 

    const double entryPrice = current.close;
    const double k = riskData.volatility;

    double stopDist = 0.0;
    if (riskData.m_marketState.hasPrev) {
        stopDist = k * (prev.high - prev.low);
    } // If there is a prev 
    // Fallback if there isn't a  prev bar or if the range is degenerate
    if (stopDist <= 0.0) stopDist = entryPrice * riskData.stopFactor;

    // Minimum stop distance to prevent gigantic quantity
    const double minStopDist = entryPrice * 0.001; // 0.1%
    if (stopDist < minStopDist) stopDist = minStopDist;

    double stopPrice = entryPrice;

    switch (event.side) {
        case trd::Side::Buy:
            stopPrice = entryPrice - stopDist;
            break;
        case trd::Side::Sell:
            stopPrice = entryPrice + stopDist;
            break;
        default: return;
    }

    double qty = allowableRisk / stopDist;
    if (qty <= 0.0) { std::cout << "0 qty "; return;}

    // No-leverage 
    qty = std::min(qty, riskData.m_portfolio.balance / entryPrice);
    if (qty <= 0.0) { std::cout << "0 qty "; return;}

    
    trd::quantity scaledQty =static_cast<trd::quantity>(std::floor(qty * QTY_SCALE));
    if (scaledQty < 1) scaledQty = 1;

    // Recompute unscaled units after the min-lot adjustment
    const long double unscaledQty = static_cast<long double>(scaledQty) / static_cast<long double>(QTY_SCALE);
    // Final affordability check (handles rounding/min lot)
    if (unscaledQty * entryPrice > riskData.m_portfolio.balance) { std::cout << "Not enough bal "; return;}

    riskData.m_dispatcher.schedule(
        events::OrderEvent{ event.epoch, event.side, scaledQty}
    );

    riskData.m_dispatcher.schedule(
        events::StopPlanEvent{ event.epoch, event.side, stopPrice}
    );

}
