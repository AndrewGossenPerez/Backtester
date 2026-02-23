
// backtester.cpp, created by Andrew Gossen.

// This is the implementation of backtesting.hpp 
// and the Backtest::run() function 
// 
// Design :
//  - Runs an event-driven backtest over a time ordered bar series gathered from the CSV ingestor
//  - Each bar pair ( current , next ) generates a market event which propagates through the pipeline
//  - The propogation is in the following order : 
//  - MarketEvent -> StrategyHandler -> SignalEvent -> RiskHandler -> OrderEvent -> ExcecutionHandler -> 
//    FillEvent -> PortfolioHandler 
//
// Notes : 
// -- The dispatcher curently processes events sequentially

#include <iostream>
#include "core/types.hpp"
#include "data/bar.hpp"
#include "backtesting/backtesting.hpp"
#include "backtesting/strategies.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"
#include "data/market_state.hpp"
#include "API/helper.hpp"

#include <chrono>
#include <thread>
#include <iostream>
#include <vector>

// -- CONFIG --
int waitTime=65; // Waits this amount of time before trying to fetch a new bar again
// -----------


trd::Result trd::Backtest::run(std::vector<trd::Bar>& bars, Strategy& strategy, bool live) {
    
    std::vector<trd::Bar> newBars;
    newBars.reserve(2);

    MarketState marketState;
    trd::Result result;

    events::Dispatcher<2048> dispatcher(strategy, marketState, m_portfolio, result);

    if (!live) {
        
        dispatcher.getReportHandler().getTrades().reserve(bars.size());
        dispatcher.getReportHandler().getEquityPoints().reserve(bars.size());

        std::cout << " NON LIVE BACKTESTING COMMENCED \n ";

        // Historical backtest
        for (std::size_t i = 0; i + 1 < bars.size(); ++i) {
            if (m_portfolio.balance <= 0.0) break;

            if (i > 0) { marketState.prev = bars[i - 1]; marketState.hasPrev = true; }
            marketState.current = bars[i];
            marketState.next = bars[i + 1];

            dispatcher.schedule(events::MarketEvent{ marketState.current, marketState.next});
            dispatcher.run();
        }

        result.finalEquity = m_portfolio.equity(bars.back().close);
        
    }

    else {

        dispatcher.getReportHandler().getTrades().reserve(5000);
        dispatcher.getReportHandler().getEquityPoints().reserve(5000);

        std::cout << " LIVE TRADING COMMENCED \n ";
        trd::timestamp lastEpoch = 0;

        std::cout << " Process initial chunk, bars : " << bars.size() << "\n";
        // Process initial chunk

        for (std::size_t i = 0; i + 1 < bars.size(); ++i) {
            
            if (m_portfolio.balance <= 0.0) break;
            if (i > 0) { marketState.prev = bars[i - 1]; marketState.hasPrev = true; }
            marketState.current = bars[i];
            marketState.next = bars[i + 1];
            dispatcher.schedule(events::MarketEvent{ marketState.current, marketState.next });
            dispatcher.run();

        }

        while (m_portfolio.balance > 0.0) {

            std::cout << "\n Fetching latest two bars... \n";
            std::cout << "Balance : " << m_portfolio.balance << " | Position : " << descaleQty(m_portfolio.pos) << "\n";    

            bool success = addBar(newBars, 2); 
            if (!success) continue;

            std::cout << "Current Bar LE: " << bars.back().epoch << "\n";
            std::cout << "Bar epochs: " << newBars[0].epoch << " | " << newBars[1].epoch << "\n";

            if (newBars.empty()) {
                std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                continue;
            }

            trd::timestamp latestEpoch = newBars.back().epoch;
            if (latestEpoch == lastEpoch) {
                std::this_thread::sleep_for(std::chrono::seconds(waitTime));
                continue;
            }

            lastEpoch = latestEpoch;
            if (!bars.empty() && bars.back().epoch == newBars[0].epoch) {
                bars.push_back(newBars[1]);
            } else {
                bars.insert(bars.end(), newBars.begin(), newBars.end());
            }

            size_t i = bars.size() - 2;  // second-last bar is current
            if (i > 0) { marketState.prev = bars[i - 1]; marketState.hasPrev = true; }
            marketState.current = bars[i];
            marketState.next = bars[i + 1];
            dispatcher.schedule(events::MarketEvent{ marketState.current, marketState.next });
            dispatcher.run();

            std::cout << "Processed new live bar (epoch): " << marketState.current.epoch << std::endl;
            std::cout << "New bar size : " << bars.size() << "\n";

            newBars.clear();

        }


        result.finalEquity = m_portfolio.equity(bars.back().close);

    }

    result.equityPoints = dispatcher.getReportHandler().getEquityPoints();
    result.trades = dispatcher.getReportHandler().getTrades();

    return result;

}
