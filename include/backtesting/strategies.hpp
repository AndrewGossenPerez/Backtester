// strategies.hpp, created by Andrew Gossen.

// ----
// This is the header to create new strategies 
// Each strategy is derived from 'Strategy' which holds an onBar function
// which is exceuted on each bar in the bars vector to decide whether to buy/sell/hold 
// ---- 

#pragma once 
#include "core/types.hpp"
#include "data/bar.hpp"
#include "utility/scaler.hpp"
#include "events/events.hpp"
#include <random>
#include <iostream>

struct Signal{
    trd::Side side=trd::Side::Hold;
};

struct Strategy{
    
    public: 

    virtual ~Strategy()=default;
    virtual Signal onBar(const trd::Bar& bar)=0; // Pure virtual function, 
    virtual void onFill(const events::FillEvent)=0;
    // each derieved strategy will define it's own way to interpret bars into signals


};

// Strategy implementations below 


class BuyAndHold : public Strategy { // Buy once then hold forever 

    public:

    BuyAndHold() = default;

    Signal onBar(const trd::Bar& bar) override {

        if (!m_hasBought && bar.volume>0) {
            return { trd::Side::Buy };   
        }
        return { trd::Side::Hold };   
        
    }

    void onFill(const events::FillEvent) override {
        m_hasBought=true;
    }

    bool m_hasBought = false;

};

