// binding.cpp, created by Andrew Gossen.

// This establishes the core types for the Python interface
// This is currently just a quick prototype, will be refined later on 

#include <pybind11/pybind11.h>
#include "backtesting/strategies.hpp"
#include "backtesting/backtesting.hpp"
#include "data/csv_reader.hpp"
#include <pybind11/stl.h>
#include <iostream>

namespace py = pybind11;

static trd::Result run_backtest(int startingAmount) {

    trd::price startingEquity=static_cast<trd::price>(startingAmount);
    trd::csvReader reader;
    std::vector<trd::Bar> bars = reader.loadBars("samples/Bitcoin.csv",true);
    Portfolio p;
    p.setEquity(startingEquity);

    Excecution exce(1.0);
    BuyAndHold strat;

    trd::Backtest bt(p, exce);
    trd::Result re=bt.run(bars, strat);

    return re;

}

PYBIND11_MODULE(trading_engine, m) {

    m.doc() = "The python interface for the trading engine";
    
    py::class_<trd::EquityPoint>(m,"EquityPoint")
       .def_readonly("epoch",&trd::EquityPoint::epoch)
       .def_readonly("equity",&trd::EquityPoint::equity);

    py::class_<trd::Result>(m, "Result")
        .def(py::init<>())
        .def_readonly("equityPoints", &trd::Result::equityPoints); 

    m.def("run", &run_backtest, "Run a backtest with predefined data");

}