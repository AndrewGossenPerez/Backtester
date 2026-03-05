// excecution_handler.hpp, created by Andrew Gossen.

// Applies slippage and fees before filling an order to simulate a real market 

#pragma once
#include "backtesting/excecution.hpp"
#include "data/market_state.hpp"
#include "events/events.hpp"
#include "events/dispatcher.hpp"
#include "backtesting/excecution.hpp"
#include <iostream>

template <typename DispatchT>
class ExcecutionHandler {

    public:

    ExcecutionHandler(trd::MarketState& marketState,Portfolio& portfolio, DispatchT& dispatcher)
    : m_marketState(marketState),m_portfolio(portfolio), m_dispatcher(dispatcher) {}

    void on(const events::OrderEvent& event) { 

        // Apply slippage and factor in market fee 
        const trd::price px=slip(m_marketState.next.open, event.side); // Calculate price after slippage 
        trd::price fee=feeFor(event.qty, px); 

        //const trd::price cost=px*descaleQty(event.qty)+fee;
        //std::cout << "Excecution handler request for qty :" << descaleQty(event.qty) << " Real @ " << descaleQty(event.qty)*cost << "\n";
        
        if (event.qty <= 0) return;

        const double buyQty = descaleQty(event.qty);
        const double curPos = descaleQty(m_portfolio.pos); // negative position implies a short

        // Leverage checks done in portfolio 
        m_dispatcher.schedule(events::FillEvent{event.epoch,event.side,event.qty,px,fee,event.stop });

    }
    
    private:

    trd::MarketState& m_marketState;
    Portfolio& m_portfolio;
    DispatchT& m_dispatcher;

};
