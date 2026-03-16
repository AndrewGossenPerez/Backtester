// types.hpp, created by Andrew Gossen.

// Gives standard types trading aliases 

#pragma once 
#include <cstdint>

namespace trd{ 

    using timestamp=std::int64_t; // Epoch in seconds
    using price=double;
    using quantity=std::int64_t; // Can hold fractional quantities by using a scaling factor (e.g. 1.5 shares = 150000 with QTY_SCALE=100000)

    enum Side{
        Buy,Sell,Hold
    }; 
    
};