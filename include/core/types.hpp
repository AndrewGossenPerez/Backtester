// types.hpp, created by Andrew Gossen.

// ----
// Gives standard types trading aliases for clarity 
// ---- 

#pragma once 
#include <cstdint>

namespace trd{ 

    using timestamp=std::int64_t; // Epoch in seconds
    using price=double;
    using quantity=std::int64_t;

    enum Side{
        Buy,Sell,Hold
    }; 
    
};