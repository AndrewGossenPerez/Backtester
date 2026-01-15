// EMA.hpp, created by Andrew Gossen.

// ------
// First prototype of an EMA strategy
// -----

#pragma once 
#include <cstdint>
#include <vector>
#include "core/types.hpp"
#include "core/portfolio.hpp"
#include "backtesting/excecution.hpp"
#include "backtesting/strategies.hpp"
#include "events/events.hpp"


struct EMA{
    trd::price value{0.0};
    bool initialised{false};
};

template <std::size_t NFast,std::size_t NSlow>
class ExponentialMovingAverage : public Strategy { // An EMA average moving crossover impleentation 
    
    public:

    ExponentialMovingAverage(
        std::optional<bool> threshEnabled=std::nullopt,
        std::optional<double> pThresh=std::nullopt
    ) : m_thresholdEnabled(threshEnabled.value_or(true)),m_pThresh(pThresh.value_or(0.005))  {
        std::cout << "EMA set with percentage thresholding : " << (m_thresholdEnabled ? "ENABLED" : "DISABLED") << " , THRESHOLD: " << m_pThresh*100.0 << "% \n";
    } 

    Signal onBar(const trd::Bar&) override {
        //if (currentSignal==trd::Side::Sell) std::cout << "| - Requesting a SELL for a market change : " << currentMarketChange*100.0<<"% \n";
        return {currentSignal,currentMarketChange};
    }
    
    private:

    bool m_thresholdEnabled;
    double m_pThresh;

    RingBuffer<trd::price,NFast> m_fast;
    RingBuffer<trd::price,NSlow> m_slow;

    EMA m_fastEMA{0.0,false}; // Previous fast EMA 
    EMA m_slowEMA{0.0,false};
    trd::price prevFast=0.0;
    trd::price prevSlow=0.0;

    trd::Side currentSignal{trd::Side::Hold};
    double currentMarketChange{0.0};

    void onFill(const events::FillEvent&) override {return;};

    void onMarketData(const events::MarketEvent& m) override {

        // Update prices 
        m_fast.push(m.bar.close);
        m_slow.push(m.bar.close);

        if (m_fast.size() == NFast) {
            computeEMA(m.bar.close, m_fast, true);
        } else return;

        if (m_slow.size() == NSlow) {
            computeEMA(m.bar.close, m_slow, false);
        } else return;
        // Only apply EMA when the lookback windows are filled 

        // Check for crossover 
        if (m_fastEMA.initialised && m_slowEMA.initialised){

            double marketChange=(m_fastEMA.value-m_slowEMA.value)/m_slowEMA.value;

            if (m_thresholdEnabled && std::abs(marketChange)<m_pThresh){
                currentSignal=trd::Side::Hold;
                return; // Filter out noise and range-based market 
            }

            currentMarketChange=marketChange;
            double slope = m_fastEMA.value - prevFast;
            
            if (m_fastEMA.value>m_slowEMA.value){
                currentSignal=trd::Side::Buy;
            } else if (slope < 0 && m_fastEMA.value < m_slowEMA.value){
                currentSignal=trd::Side::Sell;
            } else { 
                currentSignal=trd::Side::Hold;
            }

            prevFast=m_fastEMA.value;

        }

    }

    template<std::size_t M>
    void computeEMA(trd::price pt,RingBuffer<trd::price,M>& m_prices,bool isFast=true){ // Later add summing as an optimisation to avoid calling this every time 

        trd::price alpha=(2.0/(M+1));

        EMA& ema = isFast ? m_fastEMA : m_slowEMA; // Reference to the prev slow/fast EMA 
 
        if (!ema.initialised){
            // Establish first EMA using SMA 
            trd::price sum=0.0;
            for ( std::size_t i=0;i<m_prices.size();i++){
                sum+=m_prices[i];
            }

            ema.initialised=true;
            ema.value=(sum/m_prices.size());
            return;

        }

        // First EMA is availalbe 
        ema.value=alpha*pt+(1-alpha)*ema.value;
        return;

    }

};