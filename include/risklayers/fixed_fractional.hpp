
#pragma once 
#include "data/bar.hpp"
#include "core/portfolio.hpp"
#include "pipeline/risk_handler.hpp"

// -- CONFIG -- 

// For PnL awarenes sigmoid function
constexpr double a=5.5;
constexpr double b=0.35;

double riskFactor=0.01;
double stopFactor=0.02;
double volatility=1.0;
double slippageBuffer=0.025;

// -------------

template <typename DispatchT>
void FixedFractionalRisk(RiskData<DispatchT>& riskData,const events::SignalEvent& event){

    // Solely Long-only for now, one position at a time 

    if (event.side == trd::Side::Sell){
        riskData.m_dispatcher.schedule(events::OrderEvent{ 
            event.epoch, trd::Side::Sell, riskData.m_portfolio.pos
         });
        return;
    };

    if (event.side==trd::Side::Hold) return;

    // One position at a time 
    if (riskData.m_portfolio.pos != 0) return;

    const auto& market = riskData.m_marketState;
    if (!market.current.epoch) return;

    const trd::Bar& current = market.current;

    const double equity = riskData.m_portfolio.equity(current.close);
    if (equity <= 0.0) return;

    double allowableRisk = equity * riskFactor;
    if (allowableRisk <= 0.0) return;

    // Stop distance 
    double stopDist = 0.0;

    if (market.hasPrev) {
        const trd::Bar& prev = market.prev;
        const double range = prev.high - prev.low;
        if (range > 0.0) {
            stopDist = volatility * range;
        }
    }

    // Fallback stop distance
    if (stopDist <= 0.0) {
        stopDist = current.close * stopFactor;
    }

    // Hard minimum stop 
    const double minStopDist = current.close * 0.001; // 0.1%
    if (stopDist < minStopDist) {
        stopDist = minStopDist;
    }

    // Slippage / gap buffer (must be >= 0)
    const double effectiveStopDist = stopDist * (1.0 + slippageBuffer);

    // pnl awareness 
    double pnlFraction = riskData.m_portfolio.equity(current.close) 
    / riskData.m_portfolio.startingBalance;
    double pnlMultiplier = 1.0 / (1.0 + std::exp(a * (pnlFraction - b)));
    allowableRisk *= pnlMultiplier;

    // ---- Position sizing

    // Raw size from fixed fractional risk
    const double rawQty = allowableRisk / effectiveStopDist;
    if (rawQty <= 0.0) return;

    // Capital constraint 
    double costPerUnit = current.close * (1.0 + slippageBuffer); // add expected slippage
    double maxAffordableQty = riskData.m_portfolio.balance / costPerUnit;

    const double cappedQty = std::min(rawQty, maxAffordableQty);
    if (cappedQty <= 0.0) return;

    const trd::quantity scaledQty=cappedQty*QTY_SCALE;
    const long double actualRisk = descaleQty(scaledQty) * effectiveStopDist;
   
    if (actualRisk>allowableRisk) {
        return;
    }


    // Stop price
    const double entryPrice = current.close;
    const double stopPrice = entryPrice - stopDist;

    if (market.current.epoch && current.low<=stopPrice) {
        // Don't enter a position if the stop would be immediately hit 
        return;
    }

    // ---- Dispatch ( only long for now )
    //if (scaledQty>0) std::cout << " STOP SET AT: " << stopPrice << " FOR ENTRY PRICE: " << entryPrice << " WITH QTY: " << scaledQty <<"\n";
    riskData.m_dispatcher.schedule(events::OrderEvent{ event.epoch, trd::Side::Buy, scaledQty });
    riskData.m_dispatcher.schedule(events::StopPlanEvent{ event.epoch, trd::Side::Buy, stopPrice });

}

