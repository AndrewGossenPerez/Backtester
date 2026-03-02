// portfolio.hpp, created by Andrew Gossen.

// Notes:
// For buy/sell, the quantity is assumed to be already scaled to QTY_SCALE

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

    std::string symbol{"AAPL"}; // What to trade on Alpaca 

    LivePortfolio(const std::string& sym = "AAPL") : symbol(sym) {
        syncLive();
    }

    // Fetch latest account & position info from Alpaca
    void syncLive() override {
        try {
            auto account = getAccount(); // Alpaca API
            balance = std::stod(account["cash"].get<std::string>());

            try {
                // Alpaca returns numeric values as strings in JSON
                auto position = httpGet("https://paper-api.alpaca.markets/v2/positions/" + symbol);
                pos = static_cast<trd::quantity>( std::stod(position["qty"].get<std::string>()) * QTY_SCALE);
            } catch (...) {
                // If no position exists for this symbol treat it as 0
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

    // Simple leverage cap for both long & short 
    static constexpr double MAX_LEVERAGE = 2.0; // 2x notional vs equity

    void buy(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        if (qtyScaled <= 0) return;

        // Spend cash on buy
        const double qty = descaleQty(qtyScaled);
        const double cost = px * qty + fee;

        // Tentative new state
        trd::quantity newPos = pos + qtyScaled;
        double newBalance = balance - cost;

        // Simple leverage check using mark at fill px
        const double newEquity = newBalance + descaleQty(newPos) * px;
        const double newNotional = std::abs(descaleQty(newPos) * px);
        if (newEquity <= 0.0) return;
        if (newNotional > newEquity * MAX_LEVERAGE) return;

        // Apply
        balance = newBalance;
        pos = newPos;

        // buy closes shorts first, then opens long
        trd::quantity remaining = qtyScaled;

        // Cover shorts (lots with qty < 0)

        for (auto& lot : m_tracks) {

            if (remaining <= 0) break;
            if (lot.qty >= 0) continue;

            // lot.qty is negative (short). Buying reduces its magnitude toward 0.
            trd::quantity coverQty = std::min<trd::quantity>(remaining, static_cast<trd::quantity>(-lot.qty));
            lot.qty += coverQty;  
            remaining -= coverQty;

        }

        // Remove exhausted lots
        m_tracks.erase(
            std::remove_if(m_tracks.begin(), m_tracks.end(),[](const Track& t){ return t.qty == 0; }),
            m_tracks.end()
        );

        // Any leftover becomes a new long lot
        if (remaining > 0) {
            m_tracks.push_back(Track{px, remaining});
        }

    }

    void sell(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        if (qtyScaled <= 0) return;

        // Receive cash on sell
        const double qty = descaleQty(qtyScaled);
        const double gain = px * qty - fee;

        // Tentative new state
        trd::quantity newPos = pos - qtyScaled;
        double newBalance = balance + gain;

        // Simple leverage check using mark at fill px
        const double newEquity = newBalance + descaleQty(newPos) * px;
        const double newNotional = std::abs(descaleQty(newPos) * px);
        if (newEquity <= 0.0) return;
        if (newNotional > newEquity * MAX_LEVERAGE) return;

        // Apply
        balance = newBalance;
        pos = newPos;

        // sell closes longs first, then opens short
        trd::quantity remaining = qtyScaled;

        // Close longs
        for (auto& lot : m_tracks) {

            if (remaining <= 0) break;
            if (lot.qty <= 0) continue;

            trd::quantity closeQty = std::min<trd::quantity>(remaining, lot.qty);
            lot.qty -= closeQty;
            remaining -= closeQty;

        }

        // Remove exhaustedlots
        m_tracks.erase(
            std::remove_if(m_tracks.begin(), m_tracks.end(),[](const Track& t){ return t.qty == 0; }),
            m_tracks.end()
        );

        // Any leftover becomes a new short lot
        if (remaining > 0) {
            m_tracks.push_back(Track{px, static_cast<trd::quantity>(-remaining)}); // negative
        }
    }

    trd::price avgPrice() const {

        if (m_tracks.empty()) return 0.0;

        // Average entry price of the current net position direction.
        // If pos > 0 average long lots
        // If pos < 0 average short lots (returned as positive price)
        if (pos == 0) return 0.0;

        double weighted = 0.0;
        double totalQty = 0.0;

        if (pos > 0) {
            for (const auto& track : m_tracks) {
                if (track.qty <= 0) continue;
                const double q = descaleQty(track.qty);
                weighted += track.px * q;
                totalQty += q;
            }
        } else { // pos < 0
            for (const auto& track : m_tracks) {
                if (track.qty >= 0) continue;
                const double q = descaleQty(static_cast<trd::quantity>(-track.qty)); // abs
                weighted += track.px * q;
                totalQty += q;
            }
        }

        return (totalQty > 0.0) ? (weighted / totalQty) : 0.0;
    }

    void setBalance(trd::price e) {
        balance = e;
    }

    private:

    std::vector<Track> m_tracks;
    
};