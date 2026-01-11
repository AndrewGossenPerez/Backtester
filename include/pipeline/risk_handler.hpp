// risk_handler.hpp
#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>

#include "events/events.hpp"
#include "core/portfolio.hpp"
#include "data/config.hpp" // for QTY_SCALE
#include "data/market_state.hpp"
#include "backtesting/excecution.hpp"

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
    double slippageBuffer=0.05;
    // -----------------------------------


    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    DispatchT& m_dispatcher;

    // Function pointer to the current risk Model 
    using riskModel=void (*)(RiskData&,  const events::SignalEvent& event);
    riskModel current=nullptr;

};

// Risk Functions 

template <typename DispatchT>
void FollowThrough(RiskData<DispatchT>& riskData, const events::SignalEvent& event){

    if (event.side!=trd::Side::Hold){
        riskData.m_dispatcher.schedule(
            events::OrderEvent{event.epoch, event.side, QTY_SCALE} // Will only fill when vol >=1 
        );
    }

}

template <typename DispatchT>
void TrivialRisk(RiskData<DispatchT>& riskData, const events::SignalEvent& event){

    trd::Side side;
    trd::quantity qty;

    switch (event.side){
        case trd::Side::Sell: 
         side=trd::Side::Sell;
         qty=riskData.m_portfolio.pos;
         break;
        case trd::Side::Buy:
         side=trd::Side::Buy;
         qty=QTY_SCALE; // Buys one asset 
         break;
        default: return;
    }

    riskData.m_dispatcher.schedule(
        events::OrderEvent{event.epoch, side, qty}
    );

}

template <typename DispatchT>
void FixedFractionalRisk(RiskData<DispatchT>& riskData,const events::SignalEvent& event){

    // Solely Long-only for now, one position at a time 

    if (event.side == trd::Side::Sell){
        riskData.m_dispatcher.schedule(events::OrderEvent{ 
            event.epoch, trd::Side::Sell, riskData.m_portfolio.pos
         });
        return;
    };

    if (event.side==trd::Side::Hold) return;

    // One position at a time 
    if (riskData.m_portfolio.pos != 0) return;

    const auto& market = riskData.m_marketState;
    if (!market.current.epoch) return;

    const trd::Bar& current = market.current;

    const double equity = riskData.m_portfolio.equity(current.close);
    if (equity <= 0.0) return;

    const double allowableRisk = equity * riskData.riskFactor;
    if (allowableRisk <= 0.0) return;

    // Stop distance 
    double stopDist = 0.0;

    if (market.hasPrev) {
        const trd::Bar& prev = market.prev;
        const double range = prev.high - prev.low;
        if (range > 0.0) {
            stopDist = riskData.volatility * range;
        }
    }

    // Fallback stop distance
    if (stopDist <= 0.0) {
        stopDist = current.close * riskData.stopFactor;
    }

    // Hard minimum stop 
    const double minStopDist = current.close * 0.001; // 0.1%
    if (stopDist < minStopDist) {
        stopDist = minStopDist;
    }

    // Slippage / gap buffer (must be >= 0)
    const double effectiveStopDist = stopDist * (1.0 + riskData.slippageBuffer);

    // ---- Position sizing

    // Raw size from fixed fractional risk
    const double rawQty = allowableRisk / effectiveStopDist;
    if (rawQty <= 0.0) return;

    // Capital constraint 
    double costPerUnit = current.close * (1.0 + riskData.slippageBuffer); // add expected slippage
    double maxAffordableQty = riskData.m_portfolio.balance / costPerUnit;

    const double cappedQty = std::min(rawQty, maxAffordableQty);
    if (cappedQty <= 0.0) return;

    const trd::quantity scaledQty=cappedQty*QTY_SCALE;
    const long double actualRisk = descaleQty(scaledQty) * effectiveStopDist;
   
    if (actualRisk>allowableRisk) {
        return;
    }


    // Stop price
    const double entryPrice = current.close;
    const double stopPrice = entryPrice - stopDist;

    if (market.current.epoch && current.low<=stopPrice) {
        // Don't enter a position if the stop would be immediately hit 
        return;
    }

    // ---- Dispatch ( only long for now )
    riskData.m_dispatcher.schedule(events::OrderEvent{ event.epoch, trd::Side::Buy, scaledQty });
    riskData.m_dispatcher.schedule(events::StopPlanEvent{ event.epoch, trd::Side::Buy, stopPrice });

}
