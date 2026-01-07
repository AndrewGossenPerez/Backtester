
#pragma once 
#include "events/dispatcher.hpp"
#include "events/events.hpp"
#include "core/portfolio.hpp"
#include "events/dispatcher.hpp"

template <typename dispatchT>
struct RiskHandler{

    RiskHandler(Portfolio& portfolio,dispatchT& dispatcher) : m_portfolio(portfolio), m_dispatcher(dispatcher) {};

    void on(const events::SignalEvent& event){ // For now a simple position-gate risk layer 

        if (event.side==trd::Side::Buy && m_portfolio.pos==0){
            // If we have no assets, buy one 
            m_dispatcher.schedule(events::OrderEvent{event.epoch,event.side,1});
        } else if (event.side==trd::Side::Sell && m_portfolio.pos > 0 ){
            // If we have assets, sell all 
            m_dispatcher.schedule(events::OrderEvent{event.epoch,event.side,m_portfolio.pos});
        }

    }

    private:

    Portfolio& m_portfolio;
    dispatchT& m_dispatcher;

};

