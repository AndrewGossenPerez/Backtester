// fixed_fractional.hpp, created by Andrew Gossen.

// ------
// First prototype of a fixed fractional risk layer 
// -----

#pragma once
#include <optional>
#include <algorithm>
#include <limits>
#include "data/bar.hpp"
#include "core/portfolio.hpp"
#include "pipeline/risk_handler.hpp"

// ------- CONFIG -----
float risk=0.02; // 2% of spending power 
float barsReq=2; // Bars until pyramiding allowed 
float atrMult=1.5; // How much we multiply ATR by to get a stop distance 
float minStopPct=0.75;
float exitPct=0.7; // How much to exit by when selling 
// --------------------

int consecutiveTrend=0;

template <typename DispatchT>
void FixedFractionalRisk(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {
   
    if (event.side == trd::Side::Hold) return;

    const auto& market = riskData.m_marketState;
    if (!market.current.epoch) return;

    const trd::Bar& bar = market.current; // Current market bar, no look ahead bias 
    double spendingPower = riskData.m_portfolio.balance;
    if (spendingPower <= 0.0) return;

    // Stop loss 
    double stopDist = 0.0;
    double stopPrice = 0.0;
    double atr=riskData.calculateATR();

    if (riskData.barCapacity()) { // If there are enough atr bars 
        stopDist = bar.close * ( (atr/bar.close) * atrMult );
        stopPrice=  bar.close-stopDist;
    } else {
        stopPrice = bar.close * minStopPct;
        stopDist = bar.close - stopPrice;
    }

    double allowablePosition = (spendingPower*risk)/(stopDist);

    // Selling
    double currentQty = descaleQty(riskData.m_portfolio.pos);
    if (event.side == trd::Side::Sell) {
          
        double sellQty = currentQty * exitPct;
        if (sellQty <= 0) return;

        trd::quantity scaledSell = static_cast<trd::quantity>(sellQty * QTY_SCALE);

        riskData.m_dispatcher.schedule(events::OrderEvent{
            event.epoch,
            trd::Side::Sell,
            scaledSell
        });

        return;
        
    } 

    trd::quantity scaledQty = static_cast<trd::quantity>(allowablePosition * QTY_SCALE);
    
    if (spendingPower*allowablePosition <= 0.0 || scaledQty <= 0 ) return; 

    std::optional<stopData> stop=stopData{
        event.epoch,
        trd::Side::Buy,
        stopPrice,
        scaledQty,
        stopDist
    };

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch,
        trd::Side::Buy,
        scaledQty,
        std::move(stop)
    });

}
