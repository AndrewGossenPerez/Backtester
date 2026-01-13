// stop_manager.hpp, created by Andrew Gossen

#pragma once
#include <optional>
#include "data/market_state.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"


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

            if (data.side==trd::Side::Buy && current.low<=data.stopPrice){
                // Stop price hit, sell the position associated with this stop 

                //std::cout << " STOP HIT AT: " << data.stopPrice << " FOR QTY: " << data.qty << " ON EPOCH: " << data.epoch << "\n";
                //std::cout << "SCHEDULING A SELL FOR QTY : " << descaleQty(data.qty) << " AT EPOCH: " << current.epoch << "\n";
                
                m_dispatcher.schedule(
                    events::OrderEvent({data.epoch,trd::Side::Sell,data.qty})
                );
                
            }

        }
    
    }

    private:

    DispatchT& m_dispatcher;
    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    RingBuffer<stopData,25> m_stops;

};