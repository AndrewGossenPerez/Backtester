// risk_handler.hpp, created by Andrew Gossen.

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

constexpr int atrBars=14;  // ATR(14)

struct RiskMetaData{
    trd::price risk;
    trd::timestamp epoch;
    trd::Side side;
};

template <typename DispatchT>
class RiskData { 

    // The backtest creates one of this struct used for risk metadata, which is given to each risk model via the dispatcher 
    // Stored as a private member of the Dispatch class 

    public:
    
    RiskData(Portfolio& portfolio, trd::MarketState& marketState,DispatchT& dispatcher)
    : m_portfolio(portfolio), m_marketState(marketState), m_dispatcher(dispatcher) {}

    Portfolio& m_portfolio;
    trd::MarketState& m_marketState;
    DispatchT& m_dispatcher;

    // -- Function pointer to the current risk Model, swapping allowed
    using riskModel=void (*)(RiskData&,  const events::SignalEvent& event);
    riskModel current=nullptr;
    // ------

    double calculateATR(){

        if (!barHistory.full()) return 0.0;

        double sumTR = 0.0;

        for (size_t i = 1; i < barHistory.size(); ++i) {

            const trd::Bar& curr = barHistory[i];
            const trd::Bar& prev = barHistory[i - 1];

            double tr1 = curr.high - curr.low;
            double tr2 = std::abs(curr.high - prev.close);
            double tr3 = std::abs(curr.low  - prev.close);

            double TR = std::max({tr1, tr2, tr3});
            sumTR += TR;
        }

        return sumTR / atrBars;

    }

    void on(const events::MarketEvent& ev){
        trd::Bar current=ev.bar;
        barHistory.overwrite(current); // Add the current bar to the history for ATR calculation

        // Store current ATR for plotting 
        getATRs().push_back(calculateATR());
    }

    //void on(const events::FillEvent& ev){ return; }

    bool barCapacity () const { return barHistory.full(); }

    std::vector<trd::price>& getATRs() {return m_atrs;}
    
    RingBuffer<trd::Bar, atrBars + 1> barHistory;

    private: 

    std::vector<trd::price> m_atrs;
    
};

// Risk Functions 

// Will simply follow the signal without any risk management
template <typename DispatchT>
void FollowThrough(RiskData<DispatchT>& riskData, const events::SignalEvent& event)
{
    return;


}

