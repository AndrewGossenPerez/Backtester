// strategies.hpp, created by Andrew Gossen.

// ----
// This is the header to create new strategies 
// Each strategy is derived from 'Strategy' which holds an onBar function
// which is exceuted on each bar in the bars vector to decide whether to buy/sell/hold 
// ---- 

#pragma once 
#include "core/types.hpp"
#include "data/bar.hpp"
#include <random>

struct Signal{
    trd::Side side=trd::Side::Hold;
};

struct Strategy{
    
    public: 

    virtual ~Strategy()=default;
    virtual Signal onBar(const trd::Bar& bar)=0; // Pure virtual function, 
    // each derieved strategy will define it's own way to interpret bars into signals

};

// Strategy implementations below 

class CoinFlipStrategy : public Strategy { // Just a simple coin-flip strategy to sanity check the engine 

    public:

    CoinFlipStrategy() : m_rng(std::random_device{}()), m_dist(0, 1) {}

    Signal onBar(const trd::Bar&) override {
        int flip = m_dist(m_rng);
        if (flip == 0) return { trd::Side::Buy };
        else return { trd::Side::Sell };
    }

    private:

    std::mt19937 m_rng;
    std::uniform_int_distribution<int> m_dist;

};

class BuyAndHold : public Strategy { // Buy once then hold forever 

    public:

    BuyAndHold() = default;

    Signal onBar(const trd::Bar&) override {
        if (!m_hasBought) {
            m_hasBought = true;
            return { trd::Side::Buy };   
        }
        return { trd::Side::Hold };   
    }

    private:

    bool m_hasBought = false;

};
