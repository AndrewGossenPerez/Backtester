// report_handler.hpp, created by Andrew Gossen.

// ----
// The report handler logs all completed trades during a FillEvent, 
// and the equity for each bar (setEquity is called in backtester.cpp )
// the m_trades and m_equityCurve vectors are reserved to fit bars.size() 
// ---- 

#pragma once 
#include "events/events.hpp"
#include "core/types.hpp"
#include "data/market_state.hpp"
#include "core/portfolio.hpp"
#include "backtesting/backtesting.hpp"
#include "utility/scaler.hpp"
#include <vector>
#include <iostream>

class ReportHandler{ 

    public:

    ReportHandler(trd::MarketState& marketState,Portfolio& portfolio,trd::Result& result) : m_marketState(marketState), 
    m_portfolio(portfolio), m_result(result) {}

    void on(const events::FillEvent& event){ 

        // Trade was completed, log this trade 
        m_trades.emplace_back( 
            event.epoch,
            event.side,
            descaleQty(event.qty),
            event.px,
            event.fee
        );

        m_result.netFees+=event.fee;
        
    }

    void setEquity(){
        m_equityCurve.emplace_back( 
            m_marketState.current.epoch,
            m_portfolio.equity(m_marketState.current.close),
            descaleQty(m_portfolio.pos) 
        );
    }

    std::vector<trd::TradeLog>& getTrades() { return m_trades; }
    std::vector<trd::EquityPoint>& getEquityPoints() { return m_equityCurve; }

    private:

    trd::MarketState& m_marketState;
    Portfolio& m_portfolio;
    trd::Result& m_result;

    // The below vectors are reserved in size in backtester.cpp
    std::vector<trd::TradeLog> m_trades; // Stores each succesful trade ( i.e. fill ) after an excecution is completed 
    std::vector<trd::EquityPoint> m_equityCurve; // Stores the portfolio's equity after each market event 
    
};