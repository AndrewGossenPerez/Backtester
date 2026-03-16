// config.hpp, created by Andrew Gossen.

// Allows configurations to the engine

#pragma once 
#include "core/types.hpp"

// Configs below 
constexpr int QTY_SCALE = 100'000'000;
constexpr trd::timestamp TS_SCALE = 1'000'000; // microseconds

constexpr double SLIP_BPS = 2.8;  //  1/10000
constexpr double FEE_BPS  = 1.0; // 1/10000
constexpr double MAX_LEVERAGE = 1.0; // 1x notional vs equity for shorting 
