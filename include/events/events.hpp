// events.hpp, created by Andrew Gossen.

// ----
// Establishes each event's metadats struct 
// ---- 

#pragma once
#include <variant>
#include "core/types.hpp"
#include "data/bar.hpp"

namespace events {


// Dispatch Events 
struct MarketEvent { trd::Bar bar; trd::Bar next; }; // Event for when a new bar arrives 
struct SignalEvent { std::int64_t epoch; trd::Side side; }; // Event for when there is a desire to perform a 'side' action
struct OrderEvent { std::int64_t epoch; trd::Side side; trd::quantity qty; }; // Event to carry out a trade 
struct FillEvent { std::int64_t epoch; trd::Side side; trd::quantity qty; trd::price px; trd::price fee; }; // Event data for trade that has been completed 

// RiskManagement Events

struct StopPlanEvent {
    trd::timestamp epoch;     
    trd::Side side;    
    double stopPrice;  
};

struct StopFillEvent {
    trd::timestamp epoch;
    trd::Side side;         
    trd::quantity qty;
    double price;
};

using Event = std::variant<MarketEvent, SignalEvent, OrderEvent, FillEvent,StopPlanEvent,StopFillEvent>;

} 

