#pragma once
#include "backtesting/excecution.hpp"
#include "data/market_state.hpp"
#include "events/events.hpp"
#include "events/dispatcher.hpp"
#include <iostream>

template <typename DispatchT>
class ExcecutionHandler {

    public:

    ExcecutionHandler(Excecution& exce, trd::MarketState& marketState, DispatchT& dispatcher)
    : m_exce(exce), m_marketState(marketState), m_dispatcher(dispatcher) {}

    void on(const events::OrderEvent& event) { 
        
        if (descaleQty(event.qty)>m_marketState.next.volume) return;
        
        // Apply slippage and factor in market fee 
        const trd::price px = m_exce.slip(m_marketState.next.open, event.side); // Calculate price after slippage 
        const trd::price fee = m_exce.feeFor(event.qty, px); 

        m_dispatcher.schedule(
            events::FillEvent{event.epoch,event.side,event.qty,px,fee}
        );

    }
    
    private:

    Excecution& m_exce;
    trd::MarketState& m_marketState;
    DispatchT& m_dispatcher;

};
