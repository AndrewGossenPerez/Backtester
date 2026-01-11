
// Average Moving Crossover Strategy to trendfollow

#pragma once 
#include <cstdint>
#include <vector>
#include "core/types.hpp"
#include "core/portfolio.hpp"
#include "backtesting/excecution.hpp"
#include "backtesting/strategies.hpp"

template <std::size_t NFast, std::size_t NSlow>
class TrendFollowing : public Strategy {
    
    public:

    Signal onBar(const trd::Bar&) override {
        Signal s{currentSignal};
        currentSignal = trd::Side::Hold;
        return s;
    }
    
    private:

    RingBuffer<trd::price, NFast> m_Fast;
    RingBuffer<trd::price, NSlow> m_Slow;

};