// excecution.hpp, created by Andrew Gossen.

// ----
// Holds utility functions to be used the excecution_handler.hpp in the pipeline
// Allows for slippage calculation (Simulate the buy/sell price being different at time of 
// order excecution, causing pure loss for this simulation )
// ---- 

#pragma once
#include "core/types.hpp"
#include "data/config.hpp"
#include "utility/scaler.hpp"
#include <cstdint>
#include <algorithm>

struct Excecution {

    // Apply deterministic slippage to a reference price 
    trd::price slip(trd::price px, trd::Side side) const {

        trd::price slippage = px * (SLIP_BPS/1000.0);

        switch (side) {
            case trd::Side::Buy:  return px + slippage;
            case trd::Side::Sell: return px - slippage;
            default: return slippage; // Shouldn't occur 
        }

    }

    // Calculates market fee based on FEE BPS
    trd::price feeFor(trd::quantity qtyScaled, trd::price px) const {

        if (qtyScaled <= 0 || px <= 0) return 0; 
        auto unscaledQty=descaleQty(qtyScaled);
        auto notional = unscaledQty*px; 
        auto fee = notional * (FEE_BPS/1000.0);
        return fee; // Deduct a fee of (FEE_BPS/1000.0)% from the notional ( price of the trade ) 

    }

};
