// created by Andrew Gossen.

// Benchmark harness for the trading engine
// Used to measure CSV ingestion throughput and backtesting performance

#include "data/csv_reader.hpp"
#include "backtesting/strategies.hpp"
#include "backtesting/backtesting.hpp"
#include "backtesting/backtesting.hpp"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <vector>

int main() {

    using clock = std::chrono::steady_clock;

    std::printf("--- BACK TEST BENCHMARK STARTING :) ---\n");

    trd::price startingEquity{100'000};
    trd::csvReader reader;

    Portfolio portfolio;
    portfolio.setEquity(startingEquity);

    MovingAverageCrossover<24,100> strat; 
    trd::Backtest bt(portfolio);
    std::vector<trd::Bar> testBars = reader.loadBars("samples/aapl.csv");

    // CSV Ingestion
    auto t1CSV = clock::now();
    std::vector<trd::Bar> mainBars = reader.loadBars("samples/aapl.csv");
    auto t2CSV = clock::now();

    std::printf("\n -- BARS LOADED -- \n");

    auto warmUp=[&](){
        std::printf("\n -- ENGINE WARMUP -- \n");
        for (int i=0;i<100;i++) {
            bt.run(testBars,strat);
        }
        std::printf("\n -- ENGINE WARMUP COMPLETED -- \n");
    };
    (void) warmUp; // stop the warning ( temporary)
    //warmUp();

    double secondsCSV = std::chrono::duration<double>(t2CSV - t1CSV).count();
    const double barsProcessed = (mainBars.size() >= 2) ? double(mainBars.size() - 1) : 0.0;

    // Measure N backtest Runs 
    constexpr int N = 1;
    std::vector<double> secs;
    std::vector<double> fills;
    secs.reserve(N);
    fills.reserve(N);
    trd::Result re; 
    
    for (int i = 0; i < N; ++i) { // Backtest N times against the actual data 
        portfolio.setEquity(startingEquity);
        auto t1 = clock::now();
        re=bt.run(mainBars, strat);
        auto t2 = clock::now();
        double s = std::chrono::duration<double>(t2 - t1).count();
        secs.push_back(s);
        fills.push_back(double(re.trades.size()));
    }

    std::vector<double> secs_sorted = secs;
    std::sort(secs_sorted.begin(), secs_sorted.end());

    // Median, p90, max and min backtest elapsed time 
    auto median= secs_sorted[secs_sorted.size() / 2];
    auto p90 = secs_sorted[(secs_sorted.size() * 90) / 100];
    auto minT = secs_sorted.front();
    auto maxT = secs_sorted.back();

    // Median amount of fills excecued
    std::vector<double> fills_sorted = fills;
    std::sort(fills_sorted.begin(), fills_sorted.end());
    double medianFills = fills_sorted[fills_sorted.size() / 2];

    // Throughput calculation from median time
    double barsPerSecMedian = barsProcessed / median;
    double barsPerSecP90 = barsProcessed / p90;
    double fillsPerSecMedian = medianFills / median;

    std::cout << "\n--- TRADING RESULTS ---\n";
    std::printf("[final position]: %.5f assets\n",(double)re.equityPoints.back().pos);
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

}
