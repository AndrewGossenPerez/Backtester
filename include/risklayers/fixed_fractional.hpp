
#pragma once 

#include <optional>
#include <algorithm>

#include "data/bar.hpp"
#include "core/portfolio.hpp"
#include "pipeline/risk_handler.hpp"

// -- CONFIG -- 

// PnL awareness sigmoid function
constexpr double a = 5.5;  // Steepness
constexpr double b = 0.35; // Midpoint of equity fraction

// Core risk sizing
double riskFactor = 0.015;       // 1.5% of equity risked per trade (slightly higher to capture early trends)
double stopFactor = 0.02;        // Fallback stop distance (2% of price before ATR is ready)
double volatility = 1.0;         // Multiplier for ATR (1.0 is standard)
double slippageBuffer = 0.02;    // 2% buffer to stop distance for slippage
double minStopPct = 0.01;        // Minimum stop distance 1% of price
double maxPos = 0.2;             // Max 20% of equity in a single trade
double maxStartingEquity = 0.03; // Only risk 3% of equity per trade for first ATRBars
double tanhScale = 5.0;          // Smooth out sell pressure response
// -------------

int i=0;

template <typename DispatchT>
void FixedFractionalRisk(RiskData<DispatchT>& riskData,const events::SignalEvent& event){

    double marketChange=event.marketChange.value_or(0.0);

    if (event.side==trd::Side::Hold) return;

    const auto& market = riskData.m_marketState;
    if (!market.current.epoch) return;

    const trd::Bar& current = market.current;

    const double equity = riskData.m_portfolio.equity(current.close);
    if (equity <= 0.0) return;

    double allowableRisk = equity * riskFactor;
    if (allowableRisk <= 0.0) return;

    double stopDist = 0.0;
    
    if (riskData.barCapacity()) {
        stopDist = volatility * riskData.calculateATR();
        stopDist = std::max(stopDist, current.close * minStopPct);
    }  else { 
        stopDist = current.close * stopFactor; // Fallback
    }

    stopDist = std::max(stopDist, current.close * minStopPct);
    double effectiveStopDist = stopDist * (1.0 + slippageBuffer); // Inclusive of slippage 

    // PnL awareness 
    double pnlFraction = riskData.m_portfolio.equity(current.close) 
    / riskData.m_portfolio.startingBalance;
    double pnlMultiplier = 1.0 / (1.0 + std::exp(a * (pnlFraction - b)));
    allowableRisk *= pnlMultiplier;

    // ---- Position sizing
    double currentPos = descaleQty(riskData.m_portfolio.pos); // existing position in asset units
    double maxTradeValue = riskData.barCapacity() ? equity * maxPos : equity * maxStartingEquity;
    double maxQtyByValue = maxTradeValue / (current.close * (1.0 + slippageBuffer));
    // Compute the max new position we can take 
    double maxNewQty = std::max(0.0, maxQtyByValue - currentPos); // Only take the qty deducted from our current pos to avoid initial blowups 
    // Raw qty based on risk
    double rawQty = allowableRisk / effectiveStopDist;
    // Cap the raw qty to the maximum you are allowed to buy on top of  the current position 
    double cappedQty = std::min(rawQty, maxNewQty);
    if (cappedQty <= 0.0) return;

    // Scaling 
    const trd::quantity scaledQty=cappedQty*QTY_SCALE;
    const long double actualRisk = descaleQty(scaledQty) * effectiveStopDist;
    if (actualRisk>allowableRisk) return;

    // Stop price
    const double entryPrice = current.close;
    const double stopPrice = entryPrice - stopDist;

    // Don't enter a position if the stop would be immediately hit 
    if (market.current.epoch && current.low<=stopPrice) return;
    if (scaledQty==0) return;

    // If we're in a sell signal apply risk reduction 
    if (event.side == trd::Side::Sell ){ // MarketChange always negative 

        // Note, that the stop price will ensure that we close, however this is used to close early on downing trends 

        if (riskData.m_portfolio.pos<=0) return; // No leverage, avoid expensive calculations 

        double strength = std::tanh(-marketChange*tanhScale);
        double sellReal = strength * currentPos;

        trd::quantity sellQty = static_cast<trd::quantity>(sellReal * QTY_SCALE);

        if (sellQty>0){
            riskData.m_dispatcher.schedule(events::OrderEvent{
                event.epoch, trd::Side::Sell, sellQty
            });
        }

        return;
        
    };

    // At this point, we're in a buy signal 
    std::optional<stopData> data=stopData{
        event.epoch,
        trd::Side::Buy,
        stopPrice,
        scaledQty
    };

    if (i<100){
        std::cout << "Placing a BUY for QTY : " << descaleQty(scaledQty) << " Which is ~$" << descaleQty(scaledQty)*current.close << " With a current balance of : " << riskData.m_portfolio.balance << " \n";
        std::cout << "TREND : " << marketChange;
    }

    riskData.m_dispatcher.schedule(events::OrderEvent{
        event.epoch, trd::Side::Buy, scaledQty, std::move(data)
    } );

    i++;

}

