// portfolio.hpp, created by Andrew Gossen.

// ----
// Holds the Portfolio struct with member functions to buy/sell 
// return equity and set equity 
// 
//  Notes: 
// It is assumed that any strategies will work with unscaled quantites
// ----

#pragma once 
#include "core/types.hpp"
#include "data/config.hpp"
#include <iostream>

struct Portfolio{

    trd::price balance{10000.0};
    trd::quantity pos{0};

    void buy(trd::quantity qty,trd::price price,trd::price fee){
        // qty is scaled already 
        trd::price cost=(price*qty) - fee;
        if (cost > balance) return; // No leverage
        balance -= (price*qty) + fee;
        pos+=qty;
    }

    void sell(trd::quantity qty,trd::price price,trd::price fee){
        // qty is scaled already 
        trd::price cost=(price*qty) - fee;
        if (qty > pos) return; // Not enough assets 
        balance += cost;
        pos-=qty;
    }

    trd::price equity(trd::price markPx) const {
        return balance + (pos) * markPx;
    }

    void setEquity(trd::price equity){
        balance=equity;
    }

};