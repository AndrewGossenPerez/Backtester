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

    std::printf("--- BACK TEST STARTING ---\n");

    trd::price startingEquity{100'000};
    trd::csvReader reader;

    Portfolio portfolio;
    portfolio.setEquity(startingEquity);
    Excecution exce(1.0);
    CoinFlipStrategy strat; 
    trd::Backtest bt(portfolio, exce);

    // CSV Ingestion
    auto t1CSV = clock::now();
    std::vector<trd::Bar> bars = reader.loadBars("samples/aapl.csv");
    auto t2CSV = clock::now();

    double secondsCSV = std::chrono::duration<double>(t2CSV - t1CSV).count();
    const double barsProcessed = (bars.size() >= 2) ? double(bars.size() - 1) : 0.0;

    // Backtesting warmup
    for (int i = 0; i < 5; ++i) {
        portfolio.setEquity(startingEquity);
        bt.run(bars, strat);
    }

    // Measures N runs
    constexpr int N = 500;
    std::vector<double> secs;
    std::vector<double> fills;
    secs.reserve(N);
    fills.reserve(N);

    volatile double sink = 0.0;

    for (int i = 0; i < N; ++i) {
        portfolio.setEquity(startingEquity);

        auto t1 = clock::now();
        trd::Result re = bt.run(bars, strat);
        auto t2 = clock::now();

        double s = std::chrono::duration<double>(t2 - t1).count();
        secs.push_back(s);
        fills.push_back(double(re.trades.size()));
        sink += re.finalEquity;
    }

    std::vector<double> secs_sorted = secs;
    std::sort(secs_sorted.begin(), secs_sorted.end());

    auto median = secs_sorted[secs_sorted.size() / 2];
    auto p90 = secs_sorted[(secs_sorted.size() * 90) / 100];
    auto minT = secs_sorted.front();
    auto maxT = secs_sorted.back();

    // Median fills 
    std::vector<double> fills_sorted = fills;
    std::sort(fills_sorted.begin(), fills_sorted.end());
    double medianFills = fills_sorted[fills_sorted.size() / 2];

    // Throughput from median time
    double barsPerSecMedian = barsProcessed / median;
    double barsPerSecP90 = barsProcessed / p90;
    double fillsPerSecMedian = medianFills / median;

    portfolio.setEquity(startingEquity);
    trd::Result re=bt.run(bars,strat);

    std::cout << "\n -- BACKTEST RESULT --\n";
    std::printf("[Starting Equity]: %d \n",(int)startingEquity);
    std::printf("[Final Equity]: %d \n",(int)re.finalEquity);
    std::printf("[Fills completed]: %d \n",(int)re.trades.size());
    std::printf("[Equity Points]: %d \n",(int)re.equityPoints.size());

    std::cout << "\n--- BENCHMARKS BACKTESTER (" << N << " runs) ---\n";
    std::printf("[median time]: %.9f s\n", median);
    std::printf("[p90 time]: %.9f s\n", p90);
    std::printf("[min/max]: %.9f / %.9f s\n", minT, maxT);
    std::printf("[median bars/sec]: %.0f\n", barsPerSecMedian);
    std::printf("[p90 bars/sec]: %.0f\n", barsPerSecP90);
    std::printf("[median fills]: %.0f\n", medianFills);
    std::printf("[median fills/sec]: %.0f\n", fillsPerSecMedian);

    std::cout << "\n--- BENCHMARKS CSV INGESTION ---\n";
    std::printf("[bars/sec]: %.0f\n", (bars.size() / secondsCSV));
    std::printf("[elapsed]: %.9f s\n", secondsCSV);

    // stop unused warning
    (void)sink;

}
