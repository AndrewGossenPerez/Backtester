// config.hpp, created by Andrew Gossen.

// ----
// Allows configurations to the engine
// ----

#pragma once 
#include "core/types.hpp"

// Configs below 
constexpr int QTY_SCALE = 100'000'000;
constexpr trd::timestamp TS_SCALE = 1'000'000; // microseconds

constexpr double SLIP_BPS = 200;  // 0.2 bp = 0.002%
constexpr double FEE_BPS  = 300;  // 0.3 bp = 0.003%