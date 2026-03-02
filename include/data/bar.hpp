// bar.hpp, created by Andrew Gossen.

// Stores bar metadata 

#pragma once 
#include "core/types.hpp"

namespace trd{

    struct Bar{
        timestamp epoch; 
        price open,high,low,close;
        quantity volume;
    };

    void printBar(const Bar& bar);

} 