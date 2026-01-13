// strategy_handler.hpp, created by Andrew Gossen.

// ---- 
// Applies the given strategy for a bar
// Will return a SignalEvent with the side as Buy/Sell/HOld  
// according to whatever the strategy decides 
// --- 

#pragma once 
#include "data/market_state.hpp"
#include "backtesting/strategies.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"

template <typename dispatchT>
class StrategyHandler{

    public:

    StrategyHandler(Strategy& strat,dispatchT& dispatcher) : m_strat(strat),m_dispatcher(dispatcher) {};

    void on(const events::MarketEvent& event){

        m_strat.onMarketData(event);
        Signal signal=m_strat.onBar(event.bar); // Apply strategy on the bar to generate a singal on current bar

        if (signal.side!=trd::Side::Hold){
            // Timestamp the signal so we don't trade on the same bar 
            m_dispatcher.schedule( // Push the signal event to be handled by the risk handler 
                events::SignalEvent{event.next.epoch,signal.side,signal.marketChange}
            );
        }

    }

    void on(const events::FillEvent& event){ // For buy-hold 
        m_strat.onFill(event);
    }

    private:

    Strategy& m_strat;
    dispatchT& m_dispatcher;

};