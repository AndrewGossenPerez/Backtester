// stop_manager.hpp, created by Andrew Gossen

#pragma once
#include <optional>
#include <iostream>
#include "data/market_state.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"

// Currently only one stop loss is active at a time 

template <typename DispatchT>
class StopHandler {

    public:

    StopHandler(DispatchT& dispatcher, Portfolio& portfolio, trd::MarketState& marketState)
        : m_dispatcher(dispatcher), m_portfolio(portfolio), m_marketState(marketState) {}

    // Whenever we get a fill that contains a stop, we overwrite the single active stop
    void on(const events::FillEvent& ev) {
        if (!ev.stop.has_value()) return;

        stopData s = ev.stop.value();
        s.active = true;

        m_stops.overwrite(s); // Overwrites current stop loss data 
    }

    void on(const events::MarketEvent& /*ev*/) {
        const trd::Bar& current = m_marketState.current;

        if (m_stops.empty()) return;

        // Only one stop exists (latest)
        stopData& s = m_stops.back();
        if (!s.active) return;

        // Long stop, stop triggers on low <= stopPrice
        if (s.side == trd::Side::Buy && current.low <= s.stopPrice) {

            //std::cout << "Stop hit for BUY at epoch " << current.epoch
                      //<< " stop price : " << s.stopPrice
                     // << " current low : " << current.low << "\n";

            // Deactivate immediately so it cannot fire again this bar
            s.active = false;

            m_dispatcher.schedule(
                events::OrderEvent{
                    current.epoch,
                    trd::Side::Sell,
                    s.qty
                }
            );

            return;
        }

        // Short stop, stop triggers on high >= stopPrice 
        if (s.side == trd::Side::Sell && current.high >= s.stopPrice) {

            //std::cout << " Short stop triggered at epoch " << current.epoch
                      //<< " stop price : " << s.stopPrice
                      //<< " current high : " << current.high << "\n";

            // Deactivate immediately so it cannot fire again this bar
            s.active = false;

            m_dispatcher.schedule(
                events::OrderEvent{
                    current.epoch,
                    trd::Side::Buy,
                    s.qty
                }
            );

            return;
        }
    }

   private:

    DispatchT& m_dispatcher;
    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    RingBuffer<stopData, 1> m_stops; 

};