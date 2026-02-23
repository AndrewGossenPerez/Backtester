// helper.hpp, created by Andrew Gossen.

// -----
// Utility functions for GET and POSTs to the Alpaca API for paper testing 
//  Implementations in src/helper.cpp
// ------ 

#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "data/bar.hpp"

nlohmann::json httpGet(const std::string& url);
nlohmann::json httpRequest(const std::string& url, const std::string& method = "GET", const std::string& body = "");
bool addBar(std::vector<trd::Bar>& bars, int N, const std::string& symbol = "AAPL");
nlohmann::json getAccount();
nlohmann::json placeOrder(const std::string& symbol, double qty, const std::string& side);
