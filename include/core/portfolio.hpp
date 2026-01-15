// portfolio.hpp, created by Andrew Gossen.

// ----
// Notes:
// For buy/sell, the quantity is assumed to be already scaled to QTY_SCALE
// ---- 

#pragma once
#include "core/types.hpp"
#include "data/config.hpp"
#include "utility/scaler.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>
#include <iostream>

struct Track{
    trd::price px;
    trd::quantity qty;
};

struct Portfolio {

    Portfolio(){
        m_tracks.reserve(1000);
    }

    trd::price balance{100000.0}; 
    trd::price startingBalance{0.0}; // Used for pnL 
    trd::quantity pos{0}; 
    std::optional<trd::price> lastAddPrice;


    // Member functions 
    void buy(trd::quantity qtyScaled, trd::price px, trd::price fee) {

        std::cout << "Filling now for qty : " << descaleQty(qtyScaled) << " At price @ " << px*descaleQty(qtyScaled)+fee << "!\n";

        auto qty=descaleQty(qtyScaled);
        auto cost = px*qty+fee;
        if (cost>balance || qtyScaled<=0) return; // Leverage and sanity check 

        balance-=cost;
        pos+=qtyScaled;

        m_tracks.push_back(Track{px, qtyScaled});

        std::cout << " Fill complete \n";

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


    trd::price equity(trd::price markPx) const { // Return total portfolio value for a current market price 
        return balance + (descaleQty(pos) * (long double)markPx);
    }

    void setEquity(trd::price e) { 
        balance = e;startingBalance=e;
    }

    private:

    std::vector<Track> m_tracks;

};