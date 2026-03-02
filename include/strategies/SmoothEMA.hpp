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

// --- CONFIG -------
double minCrossover=1.0; // FastEma must be > minCrossOver * slowEma to trigger a buy, and vice versa for sell
// ------------------

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

    std::vector<trd::Side>& getSignalHistory() {
        return m_signalHistory;
    }

   private:

    bool m_thresholdEnabled;
    double m_pThresh;
    float m_alphaFast;
    float m_alphaSlow;

    std::vector<trd::price> m_fastHistory;
    std::vector<trd::price> m_slowHistory;
    std::vector<trd::Side> m_signalHistory;

    double m_fastEMA = 0.0f;
    double m_slowEMA = 0.0f;

    trd::Side currentSignal{trd::Side::Hold};
    double currentMarketChange{0.0};

    void onFill(const events::FillEvent&) override { return; }

    void onMarketData(const events::MarketEvent& m) override {
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
            // --- Trend logic based on EMA crossover 
            if (m_fastEMA > m_slowEMA) { 
                currentSignal = trd::Side::Buy;
            } else if (m_fastEMA < m_slowEMA) {
                currentSignal = trd::Side::Sell;
            } else { currentSignal = trd::Side::Hold; } 
        }

        m_signalHistory.push_back(currentSignal);
    }

};  