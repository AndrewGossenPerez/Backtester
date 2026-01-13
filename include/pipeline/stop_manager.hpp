// stop_manager.hpp, created by Andrew Gossen

#pragma once
#include <optional>
#include "data/market_state.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"

struct stopData{
    trd::timestamp epoch;
    trd::Side side;
    trd::price stopPrice;
};

template <typename DispatchT>
class StopHandler {
    
    public:

    StopHandler(DispatchT& dispatcher, Portfolio& portfolio,trd::MarketState& marketState)
    : m_dispatcher(dispatcher),m_portfolio(portfolio), m_marketState(marketState) {}

    void on(const events::StopPlanEvent& event){
        activeStop={event.epoch,event.side,event.stopPrice};
    };

    void on(const events::MarketEvent){ // Long only implementation

        if (!activeStop.has_value()) return;

        const trd::Bar &current=m_marketState.current;
        
        if (current.low<=activeStop->stopPrice){
            // Stop price hit, sell all positions 
            m_dispatcher.schedule(
                events::OrderEvent({current.epoch,trd::Side::Sell,m_portfolio.pos})
            );
        }


    }

    private:

    std::optional<stopData> activeStop;
    DispatchT& m_dispatcher;
    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;

};
