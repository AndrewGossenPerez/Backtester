// config.hpp, created by Andrew Gossen.

// ----
// Allows configurations to the engine
// ----

#pragma once 
#include "core/types.hpp"

// Configs below 
constexpr trd::price SLIP_BPS = 5; // 0.05% Slip basis points per FillEvent
constexpr int QTY_SCALE = 100'000'000;
constexpr trd::timestamp TS_SCALE = 1'000'000; // microseconds
