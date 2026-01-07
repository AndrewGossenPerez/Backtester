
#include "data/csv_reader.hpp"
#include <iostream> 
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <charconv>
#include <cstdint>

static std::string readFile(const std::string& file){

    // Convert CSV into a single string buffer 
    std::ifstream f(file, std::ios::binary);
    if (!f) throw std::runtime_error("CSV file failed to open.");

    f.seekg(0,std::ios::end);
    std::string contents;
    contents.resize(f.tellg());
    f.seekg(0,std::ios::beg);
    f.read(contents.data(),static_cast<std::streamsize>(contents.size()));
    
    return contents;

}

// --- Utility helpers

static int digit(char c){ 
    // Convert ASCII char to a digit 
    return c-'0';
}

static void nextLine(const char*& p, const char* end){
    // Advance to the next line in the csv 
    while ( p<end && *p!='\n') p++;
    if (p<end) p++; // Skip the newline char 
}

template<typename T>
static bool parseOHLCV(const char*&p,const char* end, T& out){

    // Parses the OHLCV columns of a csv (Open,high,low,close,volume)
    double v=0.0;
    const char* start=p;

    // Sum up the integer parts before the decimal point is reached 
    while (p < end) {
        char c = *p;
        if (c >= '0' && c <= '9') { v = v * 10.0 + (c - '0'); ++p; }
        else break;
    }

    // Sum up fractional parts after the decimal point is reached 
    if (p < end && *p == '.') {
        ++p;
        double place=0.1;
        while (p < end) {
            char c = *p;
            if (c >= '0' && c <= '9') { v += (c - '0') * place; place *= 0.1; ++p; }
            else break;
        }
    }

    if (p == start) return false; // Nothing was parsed 
    if (p < end && *p == ',') { ++p; out = v; return true; } // Parsed and next value awaiting
    if (p < end && (*p == '\n' || *p == '\r')) { out = v; return true; } // Parsed and newline reached 
    if (p == end) { out = v; return true; } // End of the file reached 

    return false;

}

static int64_t days_from_civil(unsigned y, unsigned m, unsigned d) {
    // Trasnform YY-MM-DD into an epoch 
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);   
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; 
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;       
    return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
}

static bool parseYMD(bool epochGiven,const char*&p,const char* end, trd::timestamp& out){

    // At this point, we are at the start of a row, i.e. 2010-01-04, 
    // Parse a YY-MM-DD into an int64_t epoch second timestamp ( UTC ) 

    if (!epochGiven){ // In YYYY-MM-DD format 
        // Firstly, validate format with cheap checks
        if (end - p < 10) return false;
        if (p[4] != '-' || p[7] != '-') return false;

        // Apply digit math to the col
        unsigned y,m,d;
        y=digit(p[0])*1000 + digit(p[1])*100 + digit(p[2])*10 + digit(p[3]);
        p+=5;
        m=digit(p[0])*10+digit(p[1]);
        p+=3;
        d=digit(p[0])*10+digit(p[1]);
        p+=3;

        // Convert to an epoch (in seconds)
        int64_t days = days_from_civil(y, m, d);
        int64_t seconds = days * 86400;
        out=seconds; 

    } else{ 
        parseOHLCV(p,end,out);
    }

    return true;
    
}

// -- Defined method functions

std::vector<trd::Bar> trd::csvReader::loadBars(const std::string& file,bool epochGiven){

    std::string fileContents=readFile(file); // Load a string buffer for the csv 
    
    size_t lines = std::count(
        fileContents.begin(), fileContents.end(), '\n'
    ); // Establishes how many bars we need to reserve 

    std::vector<trd::Bar> bars;
    if (fileContents.empty()){
        throw std::runtime_error("Could not open file.");
    } else { 
        bars.reserve(lines);
    }

    const char* p = fileContents.data(); // Get first char pointer in the string buffer 
    // Will be incremented to parse through the csv 
    const char* end = p + fileContents.size(); 
 
    nextLine(p,end); // Skip the header row (Date,open,high,low,close,volume)

    while (p<end){

        // Allocate a bar to each row in the csv
        trd::Bar bar; 
        int membersInitialised{0}; // If less than 6 members were initialised, bar is corrupted

        membersInitialised+=parseYMD(epochGiven,p,end,bar.epoch);
        membersInitialised+=parseOHLCV(p,end,bar.open);
        membersInitialised+=parseOHLCV(p,end,bar.high);
        membersInitialised+=parseOHLCV(p,end,bar.low);
        membersInitialised+=parseOHLCV(p,end,bar.close);
        membersInitialised+=parseOHLCV(p,end,bar.volume);

        if (membersInitialised==6) bars.emplace_back(bar);

        nextLine(p,end);

    }

    return bars;

};

void trd::printBar(const Bar& bar){
    std::cout << "Bar - Epoch : " << bar.epoch
    << " Open : " <<bar.open << " High : " <<bar.high << " Low : " <<bar.low << " Close :"<<bar.close
    <<" Volume : " << bar.volume << std::endl;
}