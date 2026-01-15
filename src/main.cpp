
#include "data/csv_reader.hpp"
#include "backtesting/strategies.hpp"
#include "backtesting/backtesting.hpp"
#include "strategies/EMA.hpp"
#include "API/helper.hpp"

#include <algorithm>
#include <string>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <vector>

int main() {

    LivePortfolio pf;

    std::cout << "Live portfolio set up, balance & pos : " << pf.balance << " | " << descaleQty(pf.pos) <<"\n";
    ExponentialMovingAverage<12,24> strat(true,0.001);
    trd::Backtest bt(pf);

    std::vector<trd::Bar> bars;
    addBar(bars,18); // Load the past 18 bars to fill the lookback windows 
    bt.run(bars,strat,true);

}
