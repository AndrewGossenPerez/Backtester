
#include "data/csv_reader.hpp"
#include "backtesting/signaller.hpp"
#include "backtesting/backtesting.hpp"
#include "signallers/SmoothEMA.hpp"
#include "API/helper.hpp"

#include <algorithm>
#include <string>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <vector>

int live() {

    LivePortfolio pf;

    std::cout << "Live portfolio set up, balance & pos : " << pf.balance << " | " << descaleQty(pf.pos) <<"\n";
    SmoothEMA<12,26> strat(false);
    trd::Backtest bt(pf);

    std::vector<trd::Bar> bars;
    addBar(bars,30); // Load the past 24 bars to fill the lookback windows 
    bt.run(bars,strat,true);

    return 1;

}

void benchMark();

int main() { 

    benchMark();
    return 1;


}