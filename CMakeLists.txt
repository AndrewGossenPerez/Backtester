cmake_minimum_required(VERSION 3.12)
project(pybind_vid LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(engine_core
    src/backtesting/backtester.cpp
    src/data/csv_reader.cpp
)

target_include_directories(engine_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

add_subdirectory(externals/pybind11)

pybind11_add_module(trading_engine
    src/binding/binding.cpp
)

target_link_libraries(trading_engine PRIVATE engine_core)
