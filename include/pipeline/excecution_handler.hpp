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
        const trd::price fee=feeFor(event.qty, px); 
        const trd::price cost=px*descaleQty(event.qty)+fee;
        
        if (event.side==trd::Side::Buy && m_portfolio.balance<cost){
            std::cout<<"NO LEVERAGE FOR FILL, COST : " << cost << " FEE : " << fee << "\n";
            return;
        }


        m_dispatcher.schedule(
            events::FillEvent{event.epoch,event.side,event.qty,px,fee}
        );

    }
    
    private:

    trd::MarketState& m_marketState;
    Portfolio& m_portfolio;
    DispatchT& m_dispatcher;

};
