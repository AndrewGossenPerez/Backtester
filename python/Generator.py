import pandas as pd
from alpaca_trade_api.rest import REST, TimeFrame, TimeFrameUnit
from datetime import datetime, timezone

APCA_API_KEY_ID = "PKFR6XOJVZFKSZTR7OHAPQ44UC"
APCA_API_SECRET_KEY = "GNZxBSShCafKPAMi2TweR5gHunoPAg3htikSgWwjmiWP"
BASE_URL = "https://data.alpaca.markets"


api = REST(APCA_API_KEY_ID, APCA_API_SECRET_KEY, BASE_URL, api_version="v2")

def download(symbol, start_date, end_date, output_csv):
    # Convert to proper UTC ISO format
    start = datetime.fromisoformat(start_date).replace(tzinfo=timezone.utc).isoformat().replace("+00:00", "Z")
    end   = datetime.fromisoformat(end_date).replace(tzinfo=timezone.utc).isoformat().replace("+00:00", "Z")

    bars = api.get_bars(
        symbol,
        TimeFrame(5, TimeFrameUnit.Minute),
        start=start,
        end=end,
        adjustment="raw"
    ).df

    if bars.empty:
        print("No data returned — check symbol, date range, or permissions.")
        return

    # Handle multi-index
    if isinstance(bars.index, pd.MultiIndex):
        bars = bars.xs(symbol)

    df = bars[['open', 'high', 'low', 'close', 'volume']].copy()

    # ---- Make sure index is tz-aware UTC ----
    # Alpaca usually returns UTC timestamps, but be defensive:
    if df.index.tz is None:
        df.index = df.index.tz_localize("UTC")

    # ---- Filter to Regular Trading Hours (NYSE) ----
    # Convert to NY time, filter, convert back to UTC
    df = df.tz_convert("America/New_York")

    # IMPORTANT:
    # between_time is inclusive by default; exclude the 16:00 bar (after-hours bar start)
    df = df.between_time("09:30", "16:00", inclusive="left")  # keep 09:30..15:55 :contentReference[oaicite:1]{index=1}

    df = df.tz_convert("UTC")

    df.to_csv(output_csv, index=True)
    print(f"Saved {len(df)} 5-minute RTH bars to {output_csv}")

    # Sanity checks
    print("\nTime-of-day range (NY):", df.index.tz_convert("America/New_York").time.min(),
          "->", df.index.tz_convert("America/New_York").time.max())
    print("\nMost common time differences:")
    print(df.index.to_series().diff().value_counts().head())

if __name__ == "__main__":
    SYMBOL = "SPY"
    START_DATE = "2020-01-03"
    END_DATE   = "2026-01-03"
    OUTPUT_CSV = "RecentGeneration.csv"

    download(SYMBOL, START_DATE, END_DATE, OUTPUT_CSV)