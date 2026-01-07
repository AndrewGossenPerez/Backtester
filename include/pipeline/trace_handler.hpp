// trace_handler.hpp, created by Andrew Gossen.

// ----
// This is not currently in use, but is useful for debugging
// Will print the current event and it's attributes (Obviously expensive and not used in the actual backtest)
// ---- 

#pragma once
#include <iostream>
#include <variant>
#include "events/events.hpp"

struct TraceHandler {

    void on(const events::Event& e) {
        if (std::holds_alternative<events::MarketEvent>(e)) {
            std::cout << "MarketEvent\n";
        } else if (std::holds_alternative<events::SignalEvent>(e)) {
            auto& s = std::get<events::SignalEvent>(e);
            std::cout << "SignalEvent side=" << (int)s.side << "\n";
        } else if (std::holds_alternative<events::OrderEvent>(e)) {
            auto& o = std::get<events::OrderEvent>(e);
            std::cout << "OrderEvent qty=" << o.qty << "\n";
        } else if (std::holds_alternative<events::FillEvent>(e)) {
            auto& f = std::get<events::FillEvent>(e);
            std::cout << "FillEvent qty=" << f.qty << " price=" << f.px << " fee=" << f.fee << "\n";
        }
    }
    
};

