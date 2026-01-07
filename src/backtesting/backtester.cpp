
#include <iostream>
#include "core/types.hpp"
#include "data/bar.hpp"
#include "backtesting/backtesting.hpp"
#include "backtesting/strategies.hpp"
#include "events/dispatcher.hpp"
#include "events/events.hpp"
#include "data/market_state.hpp"

trd::Result trd::Backtest::run(const std::vector<trd::Bar>& bars,Strategy& strategy){

    // Event-driven backtester 
    // For each bar creates a marketEvent
    // This marketEvent is handled by the strategy handler which produces it's own event to be handled by yet another handler
    // This process repeats/propagates through the pipeline, eventually reaching the FillEvent if a trade has been excecuted 

    MarketState marketState;
    trd::Result result;

    if (bars.size()<2) { // Not enough bars to do a simulation, in reality no one should be using this engine on so little bars anyway 
        result.finalEquity= bars.empty() ? 0.0 : m_portfolio.equity(bars.back().close);
        return result;
    }

    events::Dispatcher<2048> dispatcher(
        m_exce,strategy,marketState,m_portfolio,result
    );

    // Reserve the stored trades and equity points to their maximum possible value to avoid reallocations
    dispatcher.getReportHandler().getTrades().reserve(bars.size());
    dispatcher.getReportHandler().getEquityPoints().reserve(bars.size());

    // Main loop 
    for (std::size_t i=0; i+1 < bars.size();++i){
        marketState.current=bars[i];
        marketState.next=bars[i+1];
        // Create marketEvent which will propogate new events through the pipeline
        dispatcher.schedule(events::MarketEvent{marketState.current,marketState.next});
        dispatcher.run();
    }

    { // Format result 

    result.finalEquity=m_portfolio.equity(bars.back().close);
    result.equityPoints=dispatcher.getReportHandler().getEquityPoints();
    result.trades=dispatcher.getReportHandler().getTrades();

    }

    return result;

}