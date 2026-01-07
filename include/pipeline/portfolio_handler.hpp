// portfolio_handler.hpp, created by Andrew Gossen.

// ----
// The portfolio handler takes in a FillEvent from the excecution handler 
// Will excecute the requested buy/sell, updating the portfolio's balance,position, and equity
// ----  

#pragma once 
#include "core/portfolio.hpp"
#include "events/events.hpp"
#include <iostream>

class PortfolioHandler{

    public:

    explicit PortfolioHandler(Portfolio& portfolio) : m_portfolio(portfolio) {}

    void on(const events::FillEvent& ev){

        // Event is a Fill event
        if (ev.side == trd::Side::Buy) { 
            // Apply a buy action
            m_portfolio.buy(ev.qty,ev.px,ev.fee);
        } else if (ev.side == trd::Side::Sell) { 
            m_portfolio.sell(ev.qty,ev.px,ev.fee);
        }

    }

    private: 

    Portfolio& m_portfolio;

};