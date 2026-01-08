#pragma once
#include "backtesting/excecution.hpp"
#include "data/market_state.hpp"
#include "events/events.hpp"
#include "events/dispatcher.hpp"

template <typename DispatchT>
class ExcecutionHandler {

    public:

    ExcecutionHandler(Excecution& exce, trd::MarketState& marketState, DispatchT& dispatcher)
    : m_exce(exce), m_marketState(marketState), m_dispatcher(dispatcher) {}

    void on(const events::OrderEvent& event) { 
        
        if (event.qty>m_marketState.next.volume) return;
        
        // Apply slippage and factor in market fee 
        const trd::price px = m_exce.slip(m_marketState.next.open, event.side); // Calculate price after slippage 
        const trd::price fee = m_exce.feeFor(event.qty, px); 

        m_dispatcher.schedule(
            events::FillEvent{event.epoch,event.side,event.qty,px,fee}
        );

    }
    
    void on(const events::StopFillEvent& e) {

        m_dispatcher.schedule(events::FillEvent{
            e.epoch,
            e.side,
            e.qty,
            static_cast<trd::price>(e.price),
            0
        });
    }

    private:

    Excecution& m_exce;
    trd::MarketState& m_marketState;
    DispatchT& m_dispatcher;

};
