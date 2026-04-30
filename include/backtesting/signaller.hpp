// signaller.hpp, created by Andrew Gossen.

// Each signaller is derived from 'Signaller' which holds an onBar function
// which is called for each new bar 

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
    std::optional<double> marketChange=std::nullopt;
};

struct Signaller{
    
    public: 

    virtual ~Signaller()=default;
    virtual Signal onBar(const trd::Bar&)=0; // Pure vxirtual function, 
    virtual void onFill(const events::FillEvent &e)=0;
    virtual void onMarketData(const events::MarketEvent &m)=0;
    // each derieved signaller will define it's own way to interpret bars into signals

};

