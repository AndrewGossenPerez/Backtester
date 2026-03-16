// csv_reader.hpp, created by Andrew Gossen.

// Stores the csvReader class, defined in src/data/csv_reader.cpp 

#pragma once 
#include <vector>
#include <string>
#include "data/bar.hpp"

namespace trd { 

class csvReader{
   
   public:
   static std::vector<Bar> loadBars(const std::string& file); // Parses CSV file returning vector of Bars 

};

} 