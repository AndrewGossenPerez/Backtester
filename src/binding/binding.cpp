// binding.cpp, created by Andrew Gossen.

// This establishes the core types for the Python interface
// This is currently just a quick prototype, will be refined later on 

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include "backtesting/strategies.hpp"
#include "backtesting/backtesting.hpp"
#include "data/csv_reader.hpp"
#include <pybind11/stl.h>
#include <iostream>
#include "data/config.hpp"
#include <cstdint>

#include "strategies/EMA.hpp"

namespace py = pybind11;

static trd::Result run_backtest(int startingAmount) {

    trd::price startingEquity=static_cast<trd::price>(startingAmount);
    trd::csvReader reader;
    std::vector<trd::Bar> bars = reader.loadBars("samples/BTCREC2.csv");
    Portfolio p;
    p.setEquity(startingEquity);
   
    trd::Backtest bt(p);
    //ExponentialMovingAverage<16,24> strat;
    BuyAndHold strat;

    trd::Result re=bt.run(bars,strat);
    return re;

}

static py::list tradelogs_to_pylist(const std::vector<trd::TradeLog>& trades) {
    py::list py_trades;

    for (const auto& t : trades) {
        py::dict d;
        d["epoch"] = static_cast<std::int64_t>(t.epoch);
        d["side"]  = static_cast<int>(t.side);    // 0=Hold,1=Buy,2=Sell
        d["qty"]   = static_cast<double>(t.qty);
        d["price"] = static_cast<double>(t.price);
        d["fee"]   = static_cast<double>(t.fee);
        py_trades.append(d);
    }

    return py_trades;
}

static py::dict result_arrays(const trd::Result& r) {

    const auto& pts = r.equityPoints;
    const auto& trds = r.trades;  // std::vector<TradeLog>

    const py::ssize_t n = static_cast<py::ssize_t>(pts.size());

    auto epoch = py::array_t<std::int64_t>(n);
    auto equity = py::array_t<double>(n);
    auto pos = py::array_t<std::int64_t>(n);

    auto e = epoch.mutable_unchecked<1>();
    auto q = equity.mutable_unchecked<1>();
    auto p = pos.mutable_unchecked<1>();

    for (py::ssize_t i = 0; i < n; ++i) {
        e(i) = static_cast<std::int64_t>(pts[i].epoch);
        q(i) = static_cast<double>(pts[i].equity);
        p(i) = static_cast<std::int64_t>(pts[i].pos);
    }

    py::dict d;
    d["epoch"] = std::move(epoch);
    d["equity"] = std::move(equity);
    d["pos"] = std::move(pos);
    d["trades"] = tradelogs_to_pylist(trds);

    return d;
}


PYBIND11_MODULE(trading_engine, m) {

    m.doc() = "The python interface for the trading engine";
    
    m.def("result_arrays", &result_arrays, "Return epoch/equity/pos as numpy arrays");

    m.def("run_arrays", [](int startingAmount) {
        trd::Result r = run_backtest(startingAmount);
        return result_arrays(r);
    }, "Run backtest and return numpy arrays");

    m.attr("TIME_SCALE") = py::int_(TS_SCALE);
    m.attr("QTY_SCALE") = py::int_(QTY_SCALE);

    m.def("run", &run_backtest, "Run a backtest with predefined data");

}