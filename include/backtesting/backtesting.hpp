// backtesting.hpp, created by Andrew Gossen.

// ----
// Holds the TradeLog metadata, stored to m_trades in the report handler 
// Holds EquityPoint metadata, stored to m_equityCurve in the report handler 
// Holds Trade metadata, used for the FillEvent 
// Holds Result metadata, allowing whatever ran the backtest to access it's results in a single manifold 
// Holds the backtest class of which a program will construct and run '.run()' to backtest
// ---- 

#pragma once 
#include <cstdint>
#include <vector>
#include "core/types.hpp"
#include "core/portfolio.hpp"
#include "backtesting/excecution.hpp"
#include "backtesting/strategies.hpp"

namespace trd{ 

struct TradeLog { // Stores each completed trade 

    trd::timestamp epoch;  
    trd::Side side;     
    long double qty;    
    trd::price price;      
    trd::price fee;     

    TradeLog(trd::timestamp e,trd::Side s,long double q,trd::price p,trd::price f)
    : epoch(e), side(s), qty(q), price(p), fee(f) {}
    
};

struct EquityPoint { // Used to graph equity as a function of time

    trd::timestamp epoch;
    trd::price equity;
    long double pos;
    EquityPoint(trd::timestamp e,trd::price eq,long double qty) : epoch(e), equity(eq), pos(qty) {} 

};

struct Trade{ // An excecuted trade during a FillEvent evnet 
    trd::timestamp epoch{0};
    trd::Side side=trd::Side::Hold;
    trd::quantity qtr{0};
    trd::price price{0.0};
    trd::price fee{0.0};
};

struct Result{  // Final backtest result metadata 

    // Equity curve marked each bar 
    std::vector<EquityPoint> equityPoints;
    // Log trades 
    std::vector<TradeLog> trades;

    // Summary fields for the final output 
    trd::price finalEquity{0.0};
    trd::price maxDD{0.0};
    trd::price netFees{0.0};
    trd::quantity netBuys{0};
    trd::quantity netSells{0};

};

class Backtest{

    public:

    Backtest(Portfolio& portfolio) : m_portfolio(portfolio) {}

    Result run(const std::vector<trd::Bar>& bars,Strategy& strategy);

    private: 

    Portfolio& m_portfolio;

};

}