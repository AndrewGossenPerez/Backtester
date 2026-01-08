
// scaler.hpp, created by Andrew Gossen

// ----
// Holds utility functions to descale QTY
// ---- 

#pragma once 
#include "core/types.hpp"
#include "data/config.hpp"

inline long double descaleQty(trd::quantity scaledQty){
    return scaledQty/QTY_SCALE;
}

