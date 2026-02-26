// SMA.hpp, created by Andrew Gossen.

// ------
// A simple SMA crossover strategy 
// -----

// For branch prediciton
#define LIKELY(x)   __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)

#pragma once 
#include <cstdint>
#include <vector>
#include "core/types.hpp"
#include "core/portfolio.hpp"
#include "backtesting/excecution.hpp"
#include "backtesting/strategies.hpp"
#include "events/events.hpp"

template <std::size_t NFast,std::size_t NSlow>
class SMA : public Strategy { // An EMA average moving crossover implementation 
    
    public:

    SMA(
        std::optional<bool> threshEnabled=std::nullopt,
        std::optional<double> pThresh=std::nullopt
    ) : m_thresholdEnabled(threshEnabled.value_or(true)),m_pThresh(pThresh.value_or(0.00))  {
        std::cout << "SMA set with percentage thresholding : " << (m_thresholdEnabled ? "ENABLED" : "DISABLED") << " , THRESHOLD: " << m_pThresh*100.0 << "% \n";
    } 

    Signal onBar(const trd::Bar&) override {
        //if (currentSignal==trd::Side::Sell) std::cout << "| - Requesting a SELL for a market change : " << currentMarketChange*100.0<<"% \n";
        return {currentSignal,currentMarketChange};
    }

    std::vector<trd::price>& getHistory(bool isFast) { 
        return isFast ? m_fastHistory : m_slowHistory;
    }
    
    private:

    bool m_thresholdEnabled;
    double m_pThresh;

    //  (Not reserved, however just used to plot the fast/slow ema)
    std::vector<trd::price> m_fastHistory;
    std::vector<trd::price> m_slowHistory;

    // Lookback windows 
    RingBuffer<trd::price,NFast> m_fast; 
    RingBuffer<trd::price,NSlow> m_slow;

    float currentFastEMA=0.0f;
    float currentSlowEMA=0.0f;


    trd::Side currentSignal{trd::Side::Hold};
    double currentMarketChange{0.0};

    void onFill(const events::FillEvent&) override {return;};

    void onMarketData(const events::MarketEvent& m) override {

        m_fast.overwrite(m.bar.close);
        m_slow.overwrite(m.bar.close);

        if (m_slow.size()!=NSlow) return; // Both lookback windows are not full

        // -- Crossover check --

        currentFastEMA=computeSMA(m_fast);
        currentSlowEMA=computeSMA(m_slow);

        // Comment out 
        m_fastHistory.push_back(currentFastEMA);
        m_slowHistory.push_back(currentSlowEMA);

        double marketChange=(currentFastEMA-currentSlowEMA)/currentSlowEMA;

        if (m_thresholdEnabled && std::abs(marketChange)<m_pThresh){ // Filter out noise, prevent whipsaw signals 
            currentSignal=trd::Side::Hold;
            return; 
        }

        // Trend logic
        if (currentFastEMA > currentSlowEMA) {
            currentSignal=trd::Side::Buy;
        } else if (currentFastEMA < currentSlowEMA) {
            currentSignal=trd::Side::Sell;
        } else if (UNLIKELY(currentSignal!=trd::Side::Hold)) {
            currentSignal=trd::Side::Hold;
        }

        
    }

    template<std::size_t M>
    float computeSMA(RingBuffer<trd::price,M>& m_prices){ // Later add summing as an optimisation to avoid calling this every time 
        if (m_prices.size()==0) return 0.0f; // Obviously don't want division by zero 
        float netSum=0;
        for (std::size_t i=0;i<m_prices.size();++i){
            netSum+=m_prices[i];
        }
        return (netSum/static_cast<float>(m_prices.size()));
    }

};