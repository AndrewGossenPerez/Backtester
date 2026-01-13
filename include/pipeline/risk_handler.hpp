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

