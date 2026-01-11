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
    virtual Signal onBar(const trd::Bar& bar)=0; // Pure virtual function, 
    virtual void onFill(const events::FillEvent& ev) {return;};
    virtual void onMarketData(const events::MarketEvent& event){return;};
    // each derieved strategy will define it's own way to interpret bars into signals

};

// --- Strategy implementations below 
class BuyAndHold : public Strategy { // Buy once then hold forever 

    public:
    BuyAndHold() = default;

    Signal onBar(const trd::Bar& bar) override {
        if (!m_hasBought && bar.volume>0) {
            return { trd::Side::Buy };   
        }
        return { trd::Side::Hold };     
    }

    void onFill(const events::FillEvent& e) override {
        m_hasBought=(e.side==trd::Side::Buy);
    }

    private:
    bool m_hasBought = false;

};

template <std::size_t NFast, std::size_t NSlow>
class MovingAverageCrossover : public Strategy { 
    
    public:
    Signal onBar(const trd::Bar& bar) override { 
        if (bar.volume == 0) return {trd::Side::Hold};
        return {currentSignal};
    }
    
    private:
    // Ring buffers for storing last N prices
    RingBuffer<trd::price, NSlow> m_Slow; // Slow MA window (e.g., 200)
    RingBuffer<trd::price, NFast> m_Fast; // Fast MA window (e.g., 50)

    double m_prevFastMA = -1;
    double m_prevSlowMA = -1;
    double m_fastSum = 0.0;
    double m_slowSum = 0.0;

    trd::Side currentSignal = trd::Side::Hold;
    bool posOpen = false;

    // Minimum number of bars to allow signals (for the warm-up threshold)
    static constexpr std::size_t MIN_SLOW_BARS = 10;

    void onFill(const events::FillEvent& e) override {
        posOpen = (e.side == trd::Side::Buy);
    }

    void onMarketData(const events::MarketEvent& ev) override {

        // fast MA 
        if (m_Fast.full()) m_fastSum -= m_Fast.front(); // remove oldest
        m_Fast.push(ev.bar.close);
        m_fastSum += ev.bar.close;

        // slow MA
        if (m_Slow.full()) m_slowSum -= m_Slow.front();
        m_Slow.push(ev.bar.close);
        m_slowSum += ev.bar.close;

        // Compute mas ( with a warmup now)
        if (m_Slow.size() >= MIN_SLOW_BARS) {
            const double fastMA = m_fastSum / m_Fast.size();
            const double slowMA = m_slowSum / m_Slow.size();

            // Initialise previous MAs
            if (m_prevFastMA == -1) {
                m_prevFastMA = fastMA;
                m_prevSlowMA = slowMA;
            }

            // Generate singal
            if (!posOpen && (m_prevFastMA <= m_prevSlowMA || m_prevFastMA == -1) && fastMA > slowMA) {
                std::cout << "BUY\n";
                currentSignal = trd::Side::Buy;
            } else if (posOpen && m_prevFastMA >= m_prevSlowMA && fastMA < slowMA) {
                std::cout << "SELL\n";
                currentSignal = trd::Side::Sell;
            }

            // Update previous MAs for next bar
            m_prevFastMA = fastMA;
            m_prevSlowMA = slowMA;
        }
    }

};
