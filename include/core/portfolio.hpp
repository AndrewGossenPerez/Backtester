// portfolio.hpp, created by Andrew Gossen.

// ----
// Notes:
// For buy/sell, the quantity is assumed to be already scaled to QTY_SCALE
// ---- 
#pragma once
#include "core/types.hpp"
#include "data/config.hpp"
#include "utility/scaler.hpp"
#include "API/helper.hpp"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <iostream>

struct Track{
    trd::price px;
    trd::quantity qty;
};

class Portfolio {

    public:

    trd::price balance{0.0};
    trd::quantity pos{0};
    trd::price lastPrice{0.0};

    trd::price equity(trd::price markPx = 0.0) const  {
        return balance + (descaleQty(pos) * (long double)markPx);
    }

    virtual trd::price avgPrice() const = 0;

    virtual void buy(trd::quantity qty, trd::price px = 0.0, trd::price fee = 0.0) = 0;
    virtual void sell(trd::quantity qty, trd::price px = 0.0, trd::price fee = 0.0) = 0;
    virtual void syncLive() {}   // only needed for live portfolios

};

class LivePortfolio : public Portfolio { 

    public:

    ~LivePortfolio()=default;

    std::string symbol{"AAPL"};  // default symbol

    LivePortfolio(const std::string& sym = "AAPL") : symbol(sym) {
        syncLive();
    }

    // Fetch latest account & position info from Alpaca
    void syncLive() override {
        try {
            auto account = getAccount(); // Alpaca API
            balance = std::stod(account["cash"].get<std::string>());

            try {

                auto position = httpGet("https://paper-api.alpaca.markets/v2/positions/" + symbol);
                // Alpaca returns numeric values as strings in JSON
                pos = static_cast<trd::quantity>( std::stod(position["qty"].get<std::string>()) * QTY_SCALE);

            } catch (...) {
                // no position, assume 0
                pos = 0;
            }

        } catch (std::exception& e) {
            std::cerr << "[LivePortfolio] Error syncing live portfolio: " << e.what() << std::endl;
        } 
    }

    trd::price avgPrice() const override { return pos; }

    void buy(trd::quantity qtyScaled, trd::price px = 0.0, trd::price fee = 0.0) override {

        std::cout << "Buying now for qty : " << descaleQty(qtyScaled) << " At price @ " << px*descaleQty(qtyScaled)+fee << "!\n";

        (void) fee;
        (void) px;

        try {
            nlohmann::json response=placeOrder(symbol, descaleQty(qtyScaled), "buy"); // Alpaca market order
            std::cout << " Response :" << response["status"] << "\n";
            syncLive(); // update local state
        } catch (std::exception& e) {
            std::cerr << "[LivePortfolio] Live buy failed: " << e.what() << std::endl;
        }
    }

    void sell(trd::quantity qtyScaled, trd::price px = 0.0, trd::price fee = 0.0) override {

        std::cout << "Selling now for qty : " << descaleQty(qtyScaled) << " At price @ " << px*descaleQty(qtyScaled)+fee << "!\n";

        (void) fee;
        (void) px;

        try {
            placeOrder(symbol, descaleQty(qtyScaled), "sell"); // Alpaca market order
            syncLive(); // update local state
        } catch (std::exception& e) {
            std::cerr << "[LivePortfolio] Live sell failed: " << e.what() << std::endl;
        }
    }

};

class BacktestPortfolio : public Portfolio {

    public:

    BacktestPortfolio(){ m_tracks.reserve(1000); }

    // Member functions 
    void buy(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        //std::cout << "Filling now for qty : " << descaleQty(qtyScaled) << " At price @ " << px*descaleQty(qtyScaled)+fee << "!\n";

        auto qty=descaleQty(qtyScaled);
        auto cost = px*qty+fee;
        if (cost>balance || qtyScaled<=0) return; // Leverage and sanity check 

        balance-=cost;
        pos+=qtyScaled;

        m_tracks.push_back(Track{px, qtyScaled});

        //std::cout << " Fill complete \n";

    }

    void sell(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        if (qtyScaled>pos) return; // Leverage and sanity check 

        auto qty=descaleQty(qtyScaled);
        auto gain = px*qty-fee;
        balance+=gain;
        pos-=qtyScaled;

        trd::quantity remaining = qtyScaled;

        for (auto& lot : m_tracks) {
            if (remaining <= 0.0) break;

            double closeQty = std::min(lot.qty, qtyScaled);
            lot.qty -= closeQty;
            remaining -= closeQty;
        }

        // Remove exhausted lots
        m_tracks.erase(
            std::remove_if(
                m_tracks.begin(), m_tracks.end(),
                [](const Track& t) { return t.qty <= 0.0; }
            ),
            m_tracks.end()
        );
    
    }

    trd::price avgPrice() const {

        if (m_tracks.empty()) return 0.0;

        double weighted = 0.0;
        double totalQty = 0.0;

        for (const auto& track : m_tracks) {
            double realQty=descaleQty(track.qty);
            weighted += track.px * realQty;
            totalQty += realQty;
        }

        return (totalQty > 0.0) ? weighted / totalQty : 0.0;

    }


    void setBalance(trd::price e) { 
        balance = e;
    }

    private:
    std::vector<Track> m_tracks;

};

