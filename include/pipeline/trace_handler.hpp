
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

