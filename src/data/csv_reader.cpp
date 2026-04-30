// csv_reader.cpp, created by Andrew Gossen Andrew Gossen

// Implementation of csv_reader.hpp loadBar() function
// Loads OHLCV bar data from CSV into a vector<trd::Bar>

// This is the current design:
// - Reads the entire file into a contiguous std::string buffer
// - Parses in-place using pointer arithmetic (no per-field allocations)
// - Can now work on YY:MM:DD, YY:MM:DD HH:MM:SS, and timezone formats (though ignored)

// Notes:
// It is assumed a header row is present and skips the first line

#include "data/csv_reader.hpp"
#include <iostream> 
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <ctime>
#include <charconv>
#include <cstdint>
#include "data/config.hpp"
#include <cstdint>
#include <limits>

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

// --- Helpers

void trd::printBar(const Bar& bar){
    std::cout << "Bar - Epoch : " << bar.epoch
    << " Open : " <<bar.open << " High : " <<bar.high << " Low : " <<bar.low << " Close :"<<bar.close
    <<" Volume : " << bar.volume << std::endl;
}

static int64_t daysFromEpoch(unsigned y, unsigned m, unsigned d) {
    // Transsforms YY-MM-DD into an epoch 
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);   
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1; 
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;       
    return static_cast<int64_t>(era) * 146097 + static_cast<int64_t>(doe) - 719468;
}

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
static bool parseOHLCV(const char*&p,const char* end, T& out){ // General parser 

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

static bool parseQuantity(const char*& p, const char* end,std::int64_t scale, std::int64_t& out){

    if (p >= end) return false;

    // Must start with digit or '.'
    if ((*p < '0' || *p > '9') && *p != '.') return false;

    // Parse integer part
    std::int64_t ip = 0;
    while (p < end && *p >= '0' && *p <= '9') {
        int d = *p - '0';
        // overflow check for ip = ip*10 + d
        if (ip > (std::numeric_limits<std::int64_t>::max() - d) / 10) return false;
        ip = ip * 10 + d;
        ++p;
    }

    // Parse fractional part scaled to 'scale'
    std::int64_t fp = 0;
    std::int64_t place = scale;

    if (p < end && *p == '.') {
        ++p;
        while (p < end && *p >= '0' && *p <= '9') {
            place /= 10;
            if (place == 0) return false; // too many fractional digits for this scale
            fp += (*p - '0') * place;
            ++p;
        }
    }

    if (p < end && *p == ',') ++p;

    if (ip > (std::numeric_limits<std::int64_t>::max() - fp) / scale) return false;
    out = ip * scale + fp;
    return true;

}

static bool parseTimestampField(const char*& p, const char* end, trd::timestamp& out) {
    if (p >= end) return false;

    // ISO-like timestamp:
    // YYYY-MM-DD
    // YYYY-MM-DD HH:MM:SS
    // YYYY-MM-DD HH:MM:SS.123456
    if (end - p >= 10 && p[4] == '-' && p[7] == '-') {
        unsigned y = digit(p[0]) * 1000 + digit(p[1]) * 100 + digit(p[2]) * 10 + digit(p[3]);
        unsigned m = digit(p[5]) * 10 + digit(p[6]);
        unsigned d = digit(p[8]) * 10 + digit(p[9]);
        p += 10;

        int64_t days = daysFromEpoch(y, m, d);
        int64_t seconds = days * 86400;

        if (p < end && (*p == ' ' || *p == ',')) ++p;

        if (end - p >= 8 && p[2] == ':' && p[5] == ':') {
            unsigned hh = digit(p[0]) * 10 + digit(p[1]);
            unsigned mm = digit(p[3]) * 10 + digit(p[4]);
            unsigned ss = digit(p[6]) * 10 + digit(p[7]);

            seconds += hh * 3600 + mm * 60 + ss;
            p += 8;
        }

        int64_t frac = 0;

        if (p < end && *p == '.') {
            ++p;
            int digits = 0;

            while (p < end && *p >= '0' && *p <= '9' && digits < 6) {
                frac = frac * 10 + (*p - '0');
                ++p;
                ++digits;
            }

            while (digits < 6) {
                frac *= 10;
                ++digits;
            }

            while (p < end && *p >= '0' && *p <= '9') ++p;
        }

        if (p < end && (*p == '+' || *p == '-') && (end - p >= 6) && p[3] == ':') {
            p += 6;
        }

        if (p < end && *p == ',') ++p;

        out = seconds * TS_SCALE + frac;
        return true;
    }

    // Unix timestamp:
    // 1325413440
    // 1325413440.0
    // 1325413440.123456
    if (*p >= '0' && *p <= '9') {
        int64_t seconds = 0;

        while (p < end && *p >= '0' && *p <= '9') {
            int d = *p - '0';

            if (seconds > (std::numeric_limits<int64_t>::max() - d) / 10) {
                return false;
            }

            seconds = seconds * 10 + d;
            ++p;
        }

        int64_t frac = 0;

        if (p < end && *p == '.') {
            ++p;

            int digits = 0;
            while (p < end && *p >= '0' && *p <= '9' && digits < 6) {
                frac = frac * 10 + (*p - '0');
                ++p;
                ++digits;
            }

            while (digits < 6) {
                frac *= 10;
                ++digits;
            }

            while (p < end && *p >= '0' && *p <= '9') ++p;
        }

        if (p < end && *p != ',') return false;
        ++p;

        out = seconds * TS_SCALE + frac;
        return true;
    }

    return false;
}

// -- Methods 

std::vector<trd::Bar> trd::csvReader::loadBars(const std::string& file){

    std::string fileContents=readFile(file); // Load a string buffer for the csv 
    
    std::vector<trd::Bar> bars;
    bars.reserve(fileContents.size()/64);

    const char* p = fileContents.data(); // Get first char pointer in the string buffer 
    // Will be incremented to parse through the csv 
    const char* end = p + fileContents.size(); 
 
    nextLine(p,end); // Skip the header row (Date,open,high,low,close,volume)

    while (p<end){

        // Allocate a bar to each row in the csv

        trd::Bar bar; 
        int membersInitialised{0}; // If less than 6 members were initialised, bar is corrupted

        membersInitialised+=parseTimestampField(p,end,bar.epoch); // Timestamp/epoch
        membersInitialised+=parseOHLCV(p,end,bar.open); // Open
        membersInitialised+=parseOHLCV(p,end,bar.high); // High
        membersInitialised+=parseOHLCV(p,end,bar.low); // Low 
        membersInitialised+=parseOHLCV(p,end,bar.close); // Close 
        
        trd::quantity vol_ticks;
        membersInitialised += parseQuantity(p, end, QTY_SCALE, vol_ticks); // Bar Vol
        bar.volume=static_cast<trd::quantity>(vol_ticks); 

        if (membersInitialised==6) bars.push_back(std::move(bar));

        nextLine(p,end);

    }

    return bars;

};

