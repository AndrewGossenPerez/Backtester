// market_state.hpp, created by Andrew Gossen.

// ----
// Simply stores the current and next bar  
//

#pragma once 
#include "core/types.hpp"
#include "data/bar.hpp"

namespace trd{

struct MarketState{
    trd::Bar prev{};
    trd::Bar current{};
    trd::Bar next{};
    bool hasPrev{false};
};

}