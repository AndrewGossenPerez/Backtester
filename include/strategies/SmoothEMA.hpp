// SmoothEMA.hpp, created by Andrew Gossen - LOVE SOSA :-) 

#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include <iostream>
#include <cmath>         
#include "core/types.hpp"
#include "core/portfolio.hpp"
#include "backtesting/excecution.hpp"
#include "backtesting/strategies.hpp"
#include "events/events.hpp"

// Early stage implementation of an EMA Signaller, will signal a buy/sell upon a change in crossover

template <std::size_t NFast, std::size_t NSlow>
class SmoothEMA : public Strategy {

   public:

    SmoothEMA(std::optional<bool> threshEnabled = std::nullopt,
              std::optional<double> pThresh = std::nullopt,
              std::optional<float> alphaFast = std::nullopt,
              std::optional<float> alphaSlow = std::nullopt)
        : m_thresholdEnabled(threshEnabled.value_or(true)),
          m_pThresh(pThresh.value_or(0.001)),
          // If not provide it wil be calculated from alpha = 2/(N+1)
          m_alphaFast(alphaFast.value_or(2.0f / (static_cast<float>(NFast) + 1.0f))),
          m_alphaSlow(alphaSlow.value_or(2.0f / (static_cast<float>(NSlow) + 1.0f)))
        {}

    Signal onBar(const trd::Bar&) override {
        return {currentSignal, currentMarketChange};
    }

    // This is for plotting for the py analysis 
    std::vector<trd::price>& getHistory(bool isFast) {
        return isFast ? m_fastHistory : m_slowHistory;
    }

   private:

    bool m_thresholdEnabled;
    double m_pThresh;
    float m_alphaFast;
    float m_alphaSlow;
    trd::Side prevSignal{trd::Side::Hold}; // Won't prevent a buy/sell on first bar 

    std::vector<trd::price> m_fastHistory;
    std::vector<trd::price> m_slowHistory;

    double m_fastEMA = 0.0f;
    double m_slowEMA = 0.0f;

    trd::Side currentSignal{trd::Side::Hold};
    double currentMarketChange{0.0};

    void onFill(const events::FillEvent&) override { 
        return;
    }

    void onMarketData(const events::MarketEvent& m, double atr) override {
        float price = m.bar.close;

        // --- Compute EMA 
        m_fastEMA = m_fastHistory.empty() ? price : m_alphaFast * price + (1.0f - m_alphaFast) * m_fastEMA;

        m_slowEMA = m_slowHistory.empty() ? price : m_alphaSlow * price + (1.0f - m_alphaSlow) * m_slowEMA;

        m_fastHistory.push_back(m_fastEMA);
        m_slowHistory.push_back(m_slowEMA);

        // --- Market change for threshold 
        currentMarketChange = (m_fastEMA - m_slowEMA) / m_slowEMA;

        if (m_thresholdEnabled && std::abs(currentMarketChange) < m_pThresh) {
            currentSignal = trd::Side::Hold;
        } else {

            if (m_fastEMA > m_slowEMA && prevSignal != trd::Side::Buy ) { 
                currentSignal = trd::Side::Buy;
            } else if (m_fastEMA < m_slowEMA && prevSignal != trd::Side::Sell) {
                currentSignal = trd::Side::Sell;
            } else { currentSignal = trd::Side::Hold; } 

            if (currentSignal!=trd::Side::Hold) prevSignal=currentSignal;

        }

    }

};  