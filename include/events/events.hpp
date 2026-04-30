// events.hpp, created by Andrew Gossen.

// Establishes the core event structs 

#pragma once
#include <variant>
#include <optional>

#include "core/types.hpp"
#include "data/bar.hpp"

// Single-stop data 
struct stopData {
    bool active{false};
    trd::Side side;       
    double stopPrice{0.0};
    trd::quantity qty{0};     
};

namespace events {


// Dispatch Events 
struct MarketEvent { trd::Bar bar; trd::Bar next; }; // Event for when a new bar arrives 
struct SignalEvent { trd::timestamp epoch; trd::Side side; std::optional<double> marketChange=std::nullopt; }; // Event for when there is a desire to perform a 'side' action
struct OrderEvent { trd::timestamp epoch; trd::Side side; trd::quantity qty; std::optional<stopData> stop=std::nullopt;}; // Event to carry out a trade 
struct FillEvent { trd::timestamp epoch; trd::Side side; trd::quantity qty; trd::price px; trd::price fee; std::optional<stopData> stop=std::nullopt; }; // Event data for trade that has been completed 

using Event = std::variant<MarketEvent, SignalEvent, OrderEvent, FillEvent>;

} // namespace events 

