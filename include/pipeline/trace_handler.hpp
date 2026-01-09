// trace_handler.hpp, created by Andrew Gossen.

// ----
// This is not currently in use, but is useful for debugging
// Will print the current event and it's attributes (Obviously expensive and not used in the actual backtest)
// ---- 

#pragma once
#include <iostream>
#include <variant>
#include "events/events.hpp"
#include "utility/scaler.hpp"

struct TraceHandler {

    TraceHandler()=default;

    void on(const events::Event& e) {

        if (++y%i!=0) return;
        y=0;

        if (std::holds_alternative<events::MarketEvent>(e)) {
            std::cout << "MarketEvent\n";
        } else if (std::holds_alternative<events::SignalEvent>(e)) {
            auto& s = std::get<events::SignalEvent>(e);
            std::cout << "SignalEvent side=" << (int)s.side << "\n";
        } else if (std::holds_alternative<events::OrderEvent>(e)) {
            auto& o = std::get<events::OrderEvent>(e);
            std::cout << "OrderEvent qty=" << descaleQty(o.qty) << "\n";
        } else if (std::holds_alternative<events::FillEvent>(e)) {
            auto& f = std::get<events::FillEvent>(e);
            std::cout << "FillEvent qty=" << descaleQty(f.qty) << " price=" << f.px << " fee=" << f.fee << "\n";
        }
    }

    private:

    int i=1000; // Only show the trace every 'i' bars 
    int y=0;
    
};

