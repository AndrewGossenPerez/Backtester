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

    void on(const events::StopPlanEvent& e) {
        pendingPlan = e;
    }

    void on(const events::FillEvent& fill) {

        // Activate stop only on the entry fill

        if (pendingPlan &&
            pendingPlan->epoch == fill.epoch &&
            pendingPlan->side == fill.side)
        {
            activeStop = ActiveStop{
                fill.epoch,
                fill.side,
                pendingPlan->stopPrice,
                fill.qty
            };
            pendingPlan.reset();
            return;
        }

        // If we already have an active stop and we see an exit fill,
        // the position is gone, therefore clear stop

        if (activeStop && m_portfolio.pos==0) { // Flat
            activeStop.reset();
        }
    }

    bool last=false;
    // Check stop on the bar after each bar entry
    void on(const events::MarketEvent&) {

        if (!activeStop) return;

        const trd::Bar& bar = m_marketState.next;
        const ActiveStop& s = *activeStop;

        bool triggered = false;

        if (s.entrySide == trd::Side::Buy)
            triggered = (bar.low <= s.stopPrice);
        else
            triggered = (bar.high >= s.stopPrice);

        if (!triggered) {
            if (last){
            std::cout << "Avoided selling epoch : " << s.epoch << "\n";
            }
            last=false;
            return;
        }

        // Gap-aware fill 
        double fillPx = s.stopPrice;
        if (s.entrySide == trd::Side::Buy && bar.open < s.stopPrice) fillPx = bar.open;
        else if (s.entrySide == trd::Side::Sell && bar.open > s.stopPrice) fillPx = bar.open;

        if (m_portfolio.pos <= 0) { activeStop.reset(); return; }

        std::cout << "Selling epoch : " << s.epoch << "\n";
        last=true;
        m_dispatcher.schedule(events::StopFillEvent{
            s.epoch,
            trd::Side::Sell,
            m_portfolio.pos,
            fillPx
        });


    }
    
    private:

    struct ActiveStop {
        trd::timestamp epoch;
        trd::Side entrySide;
        double stopPrice;
        trd::quantity qty;
    };

    DispatchT& m_dispatcher;
    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;

    std::optional<events::StopPlanEvent> pendingPlan;
    std::optional<ActiveStop> activeStop;

};
