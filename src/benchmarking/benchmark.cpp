
// benchmark.hpp, created by Andrew Gossen.

// Benchmark harness for the trading engine
// Used to measure CSV ingestion throughput and backtesting performance

#include "data/csv_reader.hpp"
#include "backtesting/strategies.hpp"
#include "backtesting/backtesting.hpp"
#include "strategies/SmoothEMA.hpp"
#include "API/helper.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <vector>


void benchMark() {

    using clock = std::chrono::steady_clock;

    std::printf("--- BACK TEST BENCHMARK STARTING :) ---\n");

    trd::price startingEquity{1'000};
    trd::csvReader reader;

    BacktestPortfolio portfolio;
    portfolio.setBalance(startingEquity);
    SmoothEMA<30,90> strat(false, 0.0015);

    trd::Backtest bt(portfolio);
    std::vector<trd::Bar> testBars = reader.loadBars("samples/AAPL.csv");

    auto t1CSV = clock::now();
    std::vector<trd::Bar> mainBars = reader.loadBars("samples/BTC5Min.csv");
    auto t2CSV = clock::now();

    std::printf("\n -- BARS LOADED -- \n");

    auto warmUp=[&](){
        std::printf("\n -- ENGINE WARMUP -- \n");
        for (int i=0;i<100;i++) {
            bt.run(testBars,strat,false);
        }
        std::printf("\n -- ENGINE WARMUP COMPLETED -- \n");
    };
    (void) warmUp; // stop the warning ( temporary)
    //warmUp();

    double secondsCSV = std::chrono::duration<double>(t2CSV - t1CSV).count();
    const double barsProcessed = (mainBars.size() >= 2) ? double(mainBars.size() - 1) : 0.0;

    // Measure N backtest Runs 
    constexpr int N = 1000;
    std::vector<double> secs;
    std::vector<double> fills;
    secs.reserve(N);
    fills.reserve(N);
    trd::Result re; 
    
    for (int i = 0; i < N; ++i) { // Backtest N times against the actual data 
        portfolio.setBalance(startingEquity);
        auto t1 = clock::now();
        re=bt.run(mainBars, strat,false);
        auto t2 = clock::now();
        double s = std::chrono::duration<double>(t2 - t1).count();
        secs.push_back(s);
        fills.push_back(double(re.trades.size()));
    }

    std::vector<double> secsSorted = secs;
    std::sort(secsSorted.begin(), secsSorted.end());

    // Median, p90, max and min backtest elapsed time 
    auto median= secsSorted[secsSorted.size() / 2];
    auto p90 = secsSorted[(secsSorted.size() * 90) / 100];
    auto minT = secsSorted.front();
    auto maxT = secsSorted.back();

    // Median amount of fills excecued
    std::vector<double> fillsSorted = fills;
    std::sort(fillsSorted.begin(), fillsSorted.end());
    double medianFills = fillsSorted[fillsSorted.size() / 2];

    // Throughput calculation from median time
    double barsPerSecMedian = barsProcessed / median;
    double barsPerSecP90 = barsProcessed / p90;
    double fillsPerSecMedian = medianFills / median;

    std::cout << "\n--- TRADING RESULTS ---\n";
    std::printf("[final position]: %.5Lf assets\n",descaleQty(portfolio.pos));
    std::printf("[final equity]: $%f \n", re.equityPoints.back().equity);

    std::cout << "\n--- BENCHMARKS BACKTESTER (" << N << " runs) ---\n";
    std::printf("[bars loaded]: %zu bars\n", mainBars.size());
    std::printf("[median time]: %.9f s\n", median);
    std::printf("[p90 time]: %.9f s\n", p90);
    std::printf("[min/max]: %.9f / %.9f s\n", minT, maxT);
    std::printf("[median bars/sec]: %.0f\n", barsPerSecMedian);
    std::printf("[p90 bars/sec]: %.0f\n", barsPerSecP90);
    std::printf("[median fills]: %.0f\n", medianFills);
    std::printf("[median fills/sec]: %.0f\n", fillsPerSecMedian);

    std::cout << "\n--- BENCHMARKS CSV INGESTION ---\n";
    std::printf("[bars/sec]: %.0f\n", (mainBars.size() / secondsCSV));
    std::printf("[elapsed]: %.9f s\n", secondsCSV);

    return;

}

