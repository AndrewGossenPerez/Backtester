// excecution.cpp, created by Andrew Gossen.

#include "backtesting/excecution.hpp"
#include "core/types.hpp"
#include "data/config.hpp"

// Applies determistic slippage to simulate real-life excecution costs.
// Slippage is applied in basis points from the SLIP_BPS value in config.hpp 

trd::price Excecution::slip(trd::price refPrice, trd::Side side) const {
    
    trd::price slipAmt = refPrice * SLIP_BPS / 10000.0;

    switch (side) {
        case trd::Side::Buy: return refPrice + slipAmt;
        case trd::Side::Sell: return refPrice - slipAmt;
        default: return refPrice;
    }

}
