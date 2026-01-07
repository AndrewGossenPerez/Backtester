#include <pybind11/pybind11.h>
#include "backtesting/strategies.hpp"
#include "backtesting/backtesting.hpp"
#include "backtesting/backtesting.hpp"
#include "data/csv_reader.hpp"

namespace py = pybind11;

static trd::price run_backtest() {

    trd::price startingEquity{100'000};

    trd::csvReader reader;
    std::vector<trd::Bar> bars = reader.loadBars("samples/aapl.csv");

    Portfolio p;
    p.setEquity(startingEquity);

    Excecution exce(1.0);
    CoinFlipStrategy strat;

    trd::Backtest bt(p, exce);
    trd::Result re=bt.run(bars, strat);
    return re.finalEquity;

}

PYBIND11_MODULE(trading_engine, m) {
    m.doc() = "The python interface for the trading engine";
    m.def("run", &run_backtest, "Run a backtest with prefined data");
}
