
// backtester.cpp, created by Andrew Gossen.

// This is the implementation of backtesting.hpp 
// and the Backtest::run() function 
// 
// Design :
//  - Runs an event-driven backtest over a time ordered bar series gathered from the CSV ingestor
//  - Each bar pair ( current , next ) generates a market event which propagates through the pipeline
//  - The propogation is in the following order : 
//  - MarketEvent -> SignalHandler -> SignalEvent -> RiskHandler -> OrderEvent -> ExcecutionHandler -> 
//    FillEvent -> PortfolioHandler 
//
// Notes : 
// -- The dispatcher curently processes events sequentially

#include <iostream>
#include "core/types.hpp"
#include "data/bar.hpp"
#include "backtesting/backtesting.hpp"
#include "backtesting/signaller.hpp"
#include "signallers/BuyNHold.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"
#include "data/market_state.hpp"
#include "API/helper.hpp"

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>


trd::Result trd::Backtest::run(std::vector<trd::Bar>& bars, Signaller& strategy, bool live) {
    
    std::vector<trd::Bar> newBars;
    newBars.reserve(2);

    MarketState marketState;
    trd::Result result;

    events::Dispatcher<2048> dispatcher(strategy, marketState, m_portfolio, result);
    
    dispatcher.getReportHandler().getTrades().reserve(bars.size());
    dispatcher.getReportHandler().getEquityPoints().reserve(bars.size());

    //std::cout << " NON LIVE BACKTESTING COMMENCED \n ";

    result.stockCloses.reserve(bars.size());

    // Historical backtest
    for (std::size_t i = 0; i + 1 < bars.size(); ++i) {

        if (i > 0) { marketState.prev = bars[i - 1]; marketState.hasPrev = true; }
        marketState.current = bars[i];
        marketState.next = bars[i + 1];

        result.stockCloses.push_back(marketState.current.close);

        dispatcher.schedule(events::MarketEvent{marketState.current, marketState.next});
        dispatcher.run();

    }

    result.finalEquity = m_portfolio.equity(bars.back().close);

    result.equityPoints = dispatcher.getReportHandler().getEquityPoints();
    result.trades = dispatcher.getReportHandler().getTrades();
    result.atrs = dispatcher.getRiskHandler().getATRs();

    return result;

}
