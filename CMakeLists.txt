cmake_minimum_required(VERSION 3.12)
project(trading_engine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Find libraries
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

# Engine core library
add_library(engine_core STATIC
    src/backtesting/backtester.cpp
    src/data/csv_reader.cpp
)

# Regonise Include dir 
target_include_directories(engine_core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

# Link curl and json properly
target_link_libraries(engine_core PUBLIC
    nlohmann_json::nlohmann_json
    CURL::libcurl
)

# Pybind11 
add_subdirectory(externals/pybind11)

pybind11_add_module(trading_engine
    src/binding/binding.cpp
)

# Link engine_core to pybind module
target_link_libraries(trading_engine PRIVATE engine_core)

# Standalone exe
add_executable(trading_main src/main.cpp src/benchmarking/benchmark.cpp)

# Add include path for trading_main
target_include_directories(trading_main PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# Link engine_core to trading_main 
target_link_libraries(trading_main PRIVATE engine_core)