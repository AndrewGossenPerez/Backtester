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

// ------- CONFIG ---------------------------------

// Core risk parameters
constexpr double RISK_PER_TRADE = 0.01;     // 1% equity per add
constexpr double ATR_MULT = 1.95; // Stop distance is ATR * multiplier, thus smaller atr mult results in a faster exit 
constexpr double MIN_STOP_PCT = 0.01;     // 1% minimum stop
constexpr bool ALLOW_ON_LOSS=false;
// Pyramiding 
constexpr int MAX_ADDS = 4;   // Max number of adds per trend to limit pyramiding 
// Exiting
constexpr double ATR_CONTRACTION = 0.6 ; // 60% of peak
constexpr double EXIT_FRAC = 0.3; // When selling, what % of the normal buy qty to sell by


// ------------------------------------------

template <typename DispatchT>
void FixedFractionalRisk(RiskData<DispatchT>& riskData, const events::SignalEvent& event) {
   
    if (event.side == trd::Side::Hold) return;

    const auto& market = riskData.m_marketState;
    if (!market.current.epoch) return;

    const trd::Bar& bar = market.current;

    double equity = riskData.m_portfolio.equity(bar.close);
    if (equity <= 0.0) return;

    // stopDist calculation
    double stopDist = 0.0;
    double atr=riskData.calculateATR();

    if (riskData.barCapacity()) { // If there are enough atr bars 
        stopDist = ATR_MULT * atr;
    } else {
        stopDist = bar.close * MIN_STOP_PCT;
    }

    stopDist = std::max(stopDist, bar.close * MIN_STOP_PCT);
    stopDist *= (1.0 + (SLIP_BPS/10000.0));

    double allowableRisk = equity * RISK_PER_TRADE; // the FixedFraction 
    double currentQty  = descaleQty(riskData.m_portfolio.pos);

    if (currentQty > 0) {
        riskData.peakATR = riskData.peakATR ? std::max(*riskData.peakATR, atr) : atr;
    } else {
        riskData.peakATR.reset();
    }

    // Selling
    if (event.side == trd::Side::Sell && riskData.peakATR.has_value()) {
        if (atr < ATR_CONTRACTION * *riskData.peakATR) {
            double sellQty = currentQty * EXIT_FRAC;
            if (sellQty <= 0) return;

            trd::quantity scaledSell =
                static_cast<trd::quantity>(sellQty * QTY_SCALE);

            riskData.m_dispatcher.schedule(events::OrderEvent{
                event.epoch,
                trd::Side::Sell,
                scaledSell
            });

            return;
        }
    } 

    trd::price averagePrice=riskData.m_portfolio.avgPrice();
    double openPnL = (bar.close - averagePrice) * currentQty;
    if ( (averagePrice>0 && currentQty > 0 && openPnL < 0.0) || ALLOW_ON_LOSS) return; // Only prevent adding if losing
    double maxThisRisk = allowableRisk * MAX_ADDS;
    if (riskData.totalOpenRisk + allowableRisk >= maxThisRisk) return; // Safeguard, prevent this fill from going over the risk budget 

    double addRisk = allowableRisk;
    double addQty  = addRisk / stopDist;

    if (addQty <= 0.0) return;

    trd::quantity scaledQty = static_cast<trd::quantity>(addQty * QTY_SCALE);
    if (scaledQty <= 0) return;

    double entryPrice = bar.close;
    double stopPrice  = entryPrice - stopDist;

    if (bar.low <= stopPrice) return;  // Don't enter if stop will be hit immediately, prevents stupid fills 

    //std::cout << "Booking order for qty: " << descaleQty(scaledQty) << " @ " << descaleQty(scaledQty)*bar.close << "\n";

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
