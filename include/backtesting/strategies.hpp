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
#include "events/ring_buffer.hpp"
#include <random>
#include <iostream>

struct Signal{
    trd::Side side=trd::Side::Hold;
};

struct Strategy{
    
    public: 

    virtual ~Strategy()=default;
    virtual Signal onBar(const trd::Bar&)=0; // Pure vxirtual function, 
    virtual void onFill(const events::FillEvent &e)=0;
    virtual void onMarketData(const events::MarketEvent &m)=0;
    // each derieved strategy will define it's own way to interpret bars into signals

};

// --- Very basic sanity check strategy implementations below 

class BuyAndHold : public Strategy { // Buy once then hold forever 

    public:
    BuyAndHold() = default;

    Signal onBar(const trd::Bar&) override {
        if (!m_hasBought) {
            return { trd::Side::Buy };   
        }
        return { trd::Side::Hold };     
    }

    void onMarketData(const events::MarketEvent& ) override {return;}

    void onFill(const events::FillEvent& e) override {
        m_hasBought=(e.side==trd::Side::Buy);
    }

    private:
    bool m_hasBought = false;

};

