// benchmark.hpp, created by Andrew Gossen.


#include "data/csv_reader.hpp"
#include "backtesting/signaller.hpp"
#include "backtesting/backtesting.hpp"
#include "signallers/SmoothEMA.hpp"
#include "API/helper.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <vector>

namespace {

using clock_type = std::chrono::steady_clock;

struct Summary {
    double median;
    double p90;
    double min;
    double max;
    double mean;
    double stdev;
};

Summary summarize(std::vector<double> values) {
    std::sort(values.begin(), values.end());

    const std::size_t n = values.size();

    double sum = 0.0;
    for (double v : values) {
        sum += v;
    }

    const double mean = sum / static_cast<double>(n);

    double sq = 0.0;
    for (double v : values) {
        const double d = v - mean;
        sq += d * d;
    }

    const double stdev = (n > 1)
        ? std::sqrt(sq / static_cast<double>(n - 1))
        : 0.0;

    const std::size_t p90Index =
        std::min(n - 1, static_cast<std::size_t>(n * 0.90));

    return Summary{
        values[n / 2],
        values[p90Index],
        values.front(),
        values.back(),
        mean,
        stdev
    };
}

} // namespace

void benchMark() {
    std::printf("--- BACKTEST BENCHMARK STARTING :) ---\n");

    constexpr int BACKTEST_RUNS = 1000;
    constexpr int CSV_RUNS = 1000;

    const trd::price startingEquity{1'000};

    trd::csvReader reader;

    // Load once for the actual backtest benchmark.
    std::vector<trd::Bar> mainBars =
        reader.loadBars("samples/BTC.csv");

    if (mainBars.empty()) {
        std::printf("No bars loaded.\n");
        return;
    }

    std::printf("\n-- BARS LOADED --\n");
    std::printf("[bars loaded]: %zu\n", mainBars.size());

    // Warm up using separate objects so the measured runs do not inherit state.
    std::printf("\n-- ENGINE WARMUP --\n");
    for (int i = 0; i < 20; ++i) {
        BacktestPortfolio warmPortfolio;
        warmPortfolio.setBalance(startingEquity);

        SmoothEMA<30, 90> warmStrat(false, 0.0015);
        trd::Backtest warmBt(warmPortfolio);

        (void) warmBt.run(mainBars, warmStrat, false);
    }
    std::printf("-- ENGINE WARMUP COMPLETED --\n");

    std::vector<double> backtestSecs;
    std::vector<double> fills;

    backtestSecs.reserve(BACKTEST_RUNS);
    fills.reserve(BACKTEST_RUNS);

    trd::Result lastResult;
    BacktestPortfolio lastPortfolio;

    for (int i = 0; i < BACKTEST_RUNS; ++i) {
        BacktestPortfolio runPortfolio;
        runPortfolio.setBalance(startingEquity);

        SmoothEMA<30, 90> runStrat(false, 0.0015);
        trd::Backtest runBt(runPortfolio);

        const auto t1 = clock_type::now();
        trd::Result result = runBt.run(mainBars, runStrat, false);
        const auto t2 = clock_type::now();

        const double seconds = std::chrono::duration<double>(t2 - t1).count();

        backtestSecs.push_back(seconds);
        fills.push_back(static_cast<double>(result.trades.size()));

        lastResult = std::move(result);
        lastPortfolio = runPortfolio;
    }

    const Summary backtest = summarize(backtestSecs);
    const Summary fillStats = summarize(fills);

    const double barsProcessed =
        (mainBars.size() >= 2) ? static_cast<double>(mainBars.size() - 1) : 0.0;

    const double medianBarsPerSec = barsProcessed / backtest.median;
    const double p90BarsPerSec = barsProcessed / backtest.p90;
    const double meanBarsPerSec = barsProcessed / backtest.mean;

    const double medianFillsPerSec = fillStats.median / backtest.median;

    std::cout << "\n--- TRADING RESULTS ---\n";

    if (!lastResult.equityPoints.empty()) {
        std::printf("[final equity]: $%f\n",
                    lastResult.equityPoints.back().equity);
    }

    std::printf("[last run trades]: %zu\n", lastResult.trades.size());

    std::cout << "\n--- BENCHMARKS BACKTESTER (" << BACKTEST_RUNS << " runs) ---\n";
    std::printf("[median time]: %.9f s\n", backtest.median);
    std::printf("[mean time]: %.9f ± %.9f s\n", backtest.mean, backtest.stdev);
    std::printf("[p90 time]: %.9f s\n", backtest.p90);
    std::printf("[min/max]: %.9f / %.9f s\n", backtest.min, backtest.max);
    std::printf("[median bars/sec]: %.0f\n", medianBarsPerSec);
    std::printf("[mean bars/sec]: %.0f\n", meanBarsPerSec);
    std::printf("[p90 bars/sec]: %.0f\n", p90BarsPerSec);
    std::printf("[median fills]: %.0f\n", fillStats.median);
    std::printf("[median fills/sec]: %.0f\n", medianFillsPerSec);

    // CSV ingestion benchmark.
    std::vector<double> csvSecs;
    csvSecs.reserve(CSV_RUNS);

    // One untimed load to warm OS cache/parser path.
    (void) reader.loadBars("samples/BTC.csv");

    for (int i = 0; i < CSV_RUNS; ++i) {
        const auto t1 = clock_type::now();
        std::vector<trd::Bar> bars = reader.loadBars("samples/BTC.csv");
        const auto t2 = clock_type::now();

        if (bars.empty()) {
            std::printf("CSV benchmark failed: no bars loaded.\n");
            return;
        }

        csvSecs.push_back(std::chrono::duration<double>(t2 - t1).count());
    }

    const Summary csv = summarize(csvSecs);

    std::cout << "\n--- BENCHMARKS CSV INGESTION (" << CSV_RUNS << " runs) ---\n";
    std::printf("[median elapsed]: %.9f s\n", csv.median);
    std::printf("[mean elapsed]: %.9f ± %.9f s\n", csv.mean, csv.stdev);
    std::printf("[p90 elapsed]: %.9f s\n", csv.p90);
    std::printf("[median bars/sec]: %.0f\n",
                static_cast<double>(mainBars.size()) / csv.median);
    std::printf("[mean bars/sec]: %.0f\n",
                static_cast<double>(mainBars.size()) / csv.mean);

    std::printf("\n- Program finished :) -\n");
}