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
    trd::quantity pos{0}; 

    int i=0;

    void buy(trd::quantity qtyScaled, trd::price px, trd::price fee) {
        auto qty=descaleQty(qtyScaled);
        auto cost = px*qty+fee;

        i++;

        if (i%1000==0){
            std::cout << " COST: " <<cost<<" FEE: " << fee<<"\n";
            if (cost>balance) return; // No leverage 
            balance-=cost;
            pos+=qtyScaled;
            std::cout << "BOUGHT, NEW QTY : " << (double)descaleQty(pos) << "\n";
        }
        
    }

    void sell(trd::quantity qtyScaled, trd::price px, trd::price fee) {
        auto qty=descaleQty(qtyScaled);
        auto gain = px*qty-fee;
        if (pos<qtyScaled) return; // We don't have the req assets 
        balance+=gain;
        pos-=qtyScaled;
    }

    trd::price equity(trd::price markPx) const { // Return total portfolio value for a current market price 
        return balance + (descaleQty(pos) * (long double)markPx);
    }

    void setEquity(trd::price e) { balance = e; }

};