// dispatcher.hpp, created by Andrew Gossen.

#pragma once 
#include <vector>
#include <functional>
#include <utility>
#include <deque>
#include <iostream>

#include "events/events.hpp"
#include "events/ring_buffer.hpp"
#include "data/market_state.hpp"
#include "backtesting/backtesting.hpp"

#include "pipeline/strategy_handler.hpp"
#include "pipeline/risk_handler.hpp"
#include "pipeline/portfolio_handler.hpp"
#include "pipeline/excecution_handler.hpp"
#include "pipeline/trace_handler.hpp"
#include "pipeline/report_handler.hpp"

namespace events{

template <std::size_t capacity>
class Dispatcher{

    // StrategyHandler - ( MarketEvent -> SignalEvent)
    // RiskHandler - ( SignalEvent -> OrderEvent )
    // ExcecutionHandler - (OrderEvent -> FillEvent )
    // PortfolioHandler - ( FillEvent -> Final action ) 

    public:

    Dispatcher(Excecution& exce,Strategy& strat,trd::MarketState& marketState,Portfolio& portfolio,trd::Result& result) : 
    m_handlerStrat(strat,*this),
    m_handlerRisk(portfolio,*this),
    m_handlerExce(exce,marketState,*this),
    m_handlerPort(portfolio),
    m_handlerReport(marketState,portfolio,result)
    {}

    using queue=RingBuffer<Event,capacity>;

    bool schedule(Event ev){ // Schedule an event, i.e. add to the ring-buffer to be dispatched 
        return m_queue.push(std::move(ev));
    }

    void run(){  // Dispatch all events in the ringbuffer ( Calls 'on' )
        Event e;
        while (m_queue.pop(e)){
            dispatch(e);
        }
    }

    // Overload 'on' functions to run the appropraite handler 
    void on(const events::MarketEvent& ev) { m_handlerStrat.on(ev); m_handlerReport.setEquity(); } 
    void on(const events::SignalEvent& ev) { m_handlerRisk.on(ev); }
    void on(const events::OrderEvent& ev) { m_handlerExce.on(ev); }
    void on(const events::FillEvent& ev) { m_handlerPort.on(ev); m_handlerReport.on(ev); }

    ReportHandler getReportHandler(){
        return m_handlerReport;
    }

    private: 

    void dispatch(Event& e){ // Will dispatch an event, i.e. run the appropriate handler through overload resolution 
        std::visit( // Pass the variant value directly into the lambda and call appropriate ocerloaded on function
            [this](auto const& event){
                this->on(event);
            },
            e
        );
    }

    queue m_queue{};

    StrategyHandler<Dispatcher> m_handlerStrat;
    RiskHandler<Dispatcher> m_handlerRisk;
    ExcecutionHandler<Dispatcher> m_handlerExce;
    PortfolioHandler m_handlerPort;
    ReportHandler m_handlerReport;

    

};


}