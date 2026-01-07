// excecution_handler.hpp, created by Andrew Gossen.

// ----
// The excecution handler takes an OrderEvent (Request to fill)
// Applies a slippage to reflect buy/sell discrepancy, and a fill fee
// Then schedules a fill event, to cause a buy/sell on the Portfolio itself 
// ---- 

#pragma once 
#include "backtesting/excecution.hpp"
#include "data/market_state.hpp"
#include "events/events.hpp"
#include "events/dispatcher.hpp"

template <typename dispatchT>
class ExcecutionHandler{

    public:

    ExcecutionHandler(Excecution& exce,trd::MarketState& marketState,dispatchT& dispatcher) 
    : m_exce(exce), m_marketState(marketState), m_dispatcher(dispatcher) {};

    void on(const events::OrderEvent& event){

         // Is an order event 
        trd::price px=m_exce.slip(m_marketState.next.open,event.side); // Apply slippage model
        trd::price fee=m_exce.fee;

        m_dispatcher.schedule( // Add on the fill event now to update portfolio details 
            events::FillEvent{event.epoch,event.side,event.qty,px,fee}
        );

    }

    private:

    Excecution& m_exce;
    trd::MarketState& m_marketState;
    dispatchT& m_dispatcher;
    
};