// stop_manager.hpp, created by Andrew Gossen

// ----- 
// Closes open positions if a stop price is hit
// -----

#pragma once
#include <optional>
#include "data/market_state.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"

// --------- CONFIG  -------------
double trailingFactor=0.55; // 50%
// ------------------------------

template <typename DispatchT>
class StopHandler {
    
    public:

    StopHandler(DispatchT& dispatcher, Portfolio& portfolio,trd::MarketState& marketState)
    : m_dispatcher(dispatcher),m_portfolio(portfolio), m_marketState(marketState) {}

    void on(const events::FillEvent& ev){

        //if (ev.side==trd::Side::Buy) std::cout << " BOUGHT FOR QTY: " << descaleQty(ev.qty) << " PX : " << ev.px << " EPOCH: " << ev.epoch << "\n";
        if (ev.stop.has_value()){
            m_stops.overwrite(std::move(ev.stop.value()));
        }

    }

    void on(const events::MarketEvent){ // Long only implementation

        const trd::Bar &current=m_marketState.current;

        // Sell any positions with the stop price hit 
        
        for (std::size_t i=0;i<m_stops.size();i++){

            stopData data;
            m_stops.pop(data);


            // Trailing stop 
            if (data.side == trd::Side::Buy) {
                double newStop = current.close - data.trailDist * trailingFactor;
                if (newStop > data.stopPrice) data.stopPrice = newStop;  // Only move stop up
            }

            // Stop hit at this point 
            if (data.side == trd::Side::Buy && current.low <= data.stopPrice) {

                m_dispatcher.schedule(
                    events::OrderEvent{
                        data.epoch,
                        trd::Side::Sell,
                        data.qty
                    }
                );

                continue; 
            }

        }
    
    }

    private:

    DispatchT& m_dispatcher;
    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    RingBuffer<stopData,128> m_stops;

};