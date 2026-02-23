
#include "API/helper.hpp"
#include "core/types.hpp"
#include "API/keys.hpp"

#include <curl/curl.h>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {

    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;

}

nlohmann::json httpGet(const std::string& url) {

    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (curl) {

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("APCA-API-KEY-ID: " + API_KEY).c_str());
        headers = curl_slist_append(headers, ("APCA-API-SECRET-KEY: " + SECRET_KEY).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            throw std::runtime_error(curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

    }

    return nlohmann::json::parse(readBuffer);

}

nlohmann::json httpRequest(const std::string& url, const std::string& method, const std::string& body) {

    CURL* curl = curl_easy_init();
    std::string readBuffer;

    if (curl) {

        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, ("APCA-API-KEY-ID: " + API_KEY).c_str());
        headers = curl_slist_append(headers, ("APCA-API-SECRET-KEY: " + SECRET_KEY).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        }

        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK)
            throw std::runtime_error(curl_easy_strerror(res));

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

    }

    return nlohmann::json::parse(readBuffer);

}

long toEpoch(const std::string& isoTime) {
    std::tm t = {};
    std::istringstream ss(isoTime);
    ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%S");
    if(ss.fail()) return -1;
    auto tp = std::chrono::system_clock::from_time_t(std::mktime(&t));
    return std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
}

void addBar(std::vector<trd::Bar>& bars, int N, const std::string& symbol) {
    nlohmann::json jBars = httpGet("https://data.alpaca.markets/v2/stocks/" + symbol + "/bars?timeframe=1Min&limit=" + std::to_string(N));
    if (jBars.contains("bars")) {
        for (const auto& b : jBars["bars"]) {
            trd::Bar bar;
            bar.epoch = toEpoch(b["t"].get<std::string>());
            bar.open = b["o"].get<double>();
            bar.high = b["h"].get<double>();
            bar.low = b["l"].get<double>();
            bar.close = b["c"].get<double>();
            bar.volume = b["v"].get<int>();
            bars.push_back(bar);
        }
    }
    std::cout << "Bar GET";
}

nlohmann::json getAccount() {
    return httpGet("https://paper-api.alpaca.markets/v2/account");
}

nlohmann::json placeOrder(const std::string& symbol, double qty, const std::string& side) {

    std::cout << "Placing order, qty : " << (double)qty << " side : " << side << "\n";

    nlohmann::json order;
    order["symbol"] = symbol;
    order["qty"] = qty;
    order["side"] = side;
    order["type"] = "market";
    order["time_in_force"] = "day";
    return httpRequest("https://paper-api.alpaca.markets/v2/orders", "POST", order.dump());

}
