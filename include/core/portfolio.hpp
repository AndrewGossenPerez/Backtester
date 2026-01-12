// portfolio.hpp, created by Andrew Gossen.

// ----
// Notes:
// For buy/sell, the quantity is assumed to be already scaled to QTY_SCALE
// ---- 

#pragma once
#include "core/types.hpp"
#include "data/config.hpp"
#include "utility/scaler.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>

struct Portfolio {

    trd::price balance{100000.0}; 
    trd::price startingBalance{0.0}; // Used for pnL 
    trd::quantity pos{0}; 

    int i=0;

    void buy(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        auto qty=descaleQty(qtyScaled);
        auto cost = px*qty+fee;
        i++;
        balance-=cost;
        pos+=qtyScaled;

        std::cout<< "BUY #" << i << " QTY: " << qty << " @ " << px << " FEE: " << fee << " COST: " << cost << " NEW BALANCE: " << balance << "\n";
    
    }

    void sell(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        auto qty=descaleQty(qtyScaled);
        auto gain = px*qty-fee;
        balance+=gain;
        pos-=qtyScaled;

        std::cout << "SELL #" << i << " QTY: " << qty << " @ " << px << " FEE: " << fee << " GAIN: " << gain << " NEW BALANCE: " << balance << "\n";
    }

    trd::price equity(trd::price markPx) const { // Return total portfolio value for a current market price 
        return balance + (descaleQty(pos) * (long double)markPx);
    }

    void setEquity(trd::price e) { 
        balance = e;startingBalance=e;
     }

    void unrealisedPnL(trd::price markPx){
       
    }

};