// risk_handler.hpp

// -----
// Currently holds only trivial risk layers, extensive ones will be stored in include/risklayers
// -----

#pragma once

#include <algorithm>
#include <cstdint>
#include <cmath>

#include "events/events.hpp"
#include "core/portfolio.hpp"
#include "data/config.hpp" // for QTY_SCALE
#include "data/market_state.hpp"
#include "backtesting/excecution.hpp"
#include "events/ring_buffer.hpp"

constexpr int atrBars=4; 


struct RiskMetaData{ // Stores the risk for each attempted entry ( Orderbook ) 
    trd::price risk;
    trd::timestamp epoch;
    trd::Side side;
};

template <typename DispatchT>
struct RiskData { 

    // The backtest creates one of this struct used for Risk metadata, which is passed onto each 
    // risk model function 
    
    RiskData(Portfolio& portfolio, trd::MarketState& marketState,DispatchT& dispatcher)
    : m_portfolio(portfolio), m_marketState(marketState), m_dispatcher(dispatcher) {}

    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    DispatchT& m_dispatcher;

    std::optional<double> peakATR;

    // Function pointer to the current risk Model 
    using riskModel=void (*)(RiskData&,  const events::SignalEvent& event);
    riskModel current=nullptr;

    double calculateATR(){

        if (barHistory.size() < atrBars) return 0.0; // Need at least 2 bars

        double sumTR = 0.0;
        int N=0;

        for (size_t i = barHistory.size()-1; i!=0; i--) {

            const trd::Bar& curr = barHistory[i];
            const trd::Bar& prev = barHistory[i - 1];

            double highMlow = curr.high - curr.low;
            double highmPrevClose=curr.high-prev.close;
            double lowMPrevClose=curr.low-prev.close;
            double TR = std::max({std::abs(highMlow), std::abs(highmPrevClose), std::abs(lowMPrevClose)});
          //  std::cout << "TR for bar " << curr.epoch << " : " << TR << "\n\n";

            sumTR += TR;
            N++;

        }

        return sumTR/N;

    }

    void on(const events::MarketEvent& ev){
        trd::Bar current=ev.bar;
        barHistory.overwrite(current);
    }

    void on(const events::FillEvent& ev){ 

        int neg=1;
        if (ev.side==trd::Side::Sell) neg=-1;

        for (std::size_t i=0;i<riskHistory.size();i++){
            if (riskHistory[i].epoch==ev.epoch && ev.stop.has_value()) {
                totalOpenRisk+=neg*descaleQty(ev.qty)*ev.stop.value().trailDist;
            }
        }

    }

    bool barCapacity () const { return barHistory.full(); }

    RingBuffer<trd::Bar,atrBars> barHistory;
    RingBuffer<trd::Bar,5> riskHistory; // Stores last 5 attempted orders, to check if they filled to add to total open 
    trd::price totalOpenRisk=0;
    
};

// Risk Functions 

template <typename DispatchT>
void FollowThrough(RiskData<DispatchT>& riskData, const events::SignalEvent& event){

    if (event.side!=trd::Side::Hold){
        riskData.m_dispatcher.schedule(
            events::OrderEvent{event.epoch, event.side, QTY_SCALE} // Will only fill when vol >=1 
        );
    }

}

template <typename DispatchT>
void TrivialRisk(RiskData<DispatchT>& riskData, const events::SignalEvent& event){

    trd::Side side;
    trd::quantity qty;

    switch (event.side){
        case trd::Side::Sell: 
         side=trd::Side::Sell;
         qty=riskData.m_portfolio.pos;
         break;
        case trd::Side::Buy:
         side=trd::Side::Buy;
         qty=QTY_SCALE; // Buys one asset 
         break;
        default: return;
    }

    riskData.m_dispatcher.schedule(
        events::OrderEvent{event.epoch, side, qty}
    );

}

