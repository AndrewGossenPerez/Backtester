
#pragma once 
#include <cstdint>

namespace trd{ 

    using timestamp=std::int64_t; // Epoch in seconds
    using price=double;
    using quantity=int;

    enum Side{
        Buy,Sell,Hold
    }; 
    
};