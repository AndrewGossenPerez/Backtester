

#pragma once 
#include "backtesting/strategies.hpp"

class BuyAndHold : public Strategy { // Buy once then hold forever 

    public:
    BuyAndHold() = default;

    Signal onBar(const trd::Bar&) override {
        if (!m_hasBought) {
            return { trd::Side::Buy };   
        }
        return { trd::Side::Hold };     
    }

    void onMarketData(const events::MarketEvent&) override {return;}

    void onFill(const events::FillEvent& e) override {
        m_hasBought=(e.side==trd::Side::Buy);
    }

    private:
    bool m_hasBought = false;

};

