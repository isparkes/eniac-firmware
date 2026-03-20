# UDP Quote Server Specification

## Overview

A UDP server that provides Bitcoin price quotes from the DIA oracle API, formatted for display on ENIAC clock hardware. The server polls the API once per minute and responds to UDP requests on port 2222.

## Data Source

- **API**: `https://api.diadata.org/v1/assetQuotation/Bitcoin/0x0000000000000000000000000000000000000000`
- **Poll interval**: 60 seconds
- **Fields used**: `Price` (current BTC/USD price), `Time` (UTC timestamp of the quote)

## UDP Commands

All responses are null-padded to 16 bytes.

### RBTCUSDT

Raw price response.

- **Response**: The price as a decimal string, null-padded to 16 bytes.
- **Example**: `77485.26` → `77485.26\0\0\0\0\0\0\0\0`

### FBTCUSDT

Fixed-width formatted price for clock display.

- **Response**: `<digit_count><6_digits>`, null-padded to 16 bytes.
  - `digit_count`: number of digits before the decimal point (1 character)
  - `6_digits`: first 6 significant digits of the price (decimal removed), zero-padded on the left
- **Error**: Returns `ERROR` if the integer part exceeds 6 digits.
- **Example**: Price `77485.26` → integer part has 5 digits → `5077485\0\0\0\0\0\0\0\0\0`

### IBTCUSDT

Integer price with 6-digit trend indicator for clock display.

- **Response**: `<price>;<indicator>`, null-padded to 16 bytes.
  - `price`: integer part of the price, zero-padded to 6 digits
  - `indicator`: 6 characters, each `U` (up), `D` (down), or `-` (unchanged/no data)
- **Error**: Returns `ERROR` if the integer part exceeds 6 digits.
- **Example**: `077485;UU-UDD`

### Unknown command

- **Response**: `ERROR`

## Trend Indicator

The 6-character trend indicator compares the current price against snapshots taken at time-period boundaries. Each position represents a different time period:

| Position | Period | Boundary |
|----------|--------|----------|
| 1 | Yesterday | Midnight UTC crossing |
| 2 | Today | Midnight UTC crossing |
| 3 | 4 hours | Every 4-hour UTC block (0-3, 4-7, 8-11, 12-15, 16-19, 20-23) |
| 4 | 1 hour | Every hour |
| 5 | 15 minutes | Every 15-minute block (0-14, 15-29, 30-44, 45-59) |
| 6 | 1 minute | Every minute |

### Indicator values

- `U` — current price is **above** the snapshot for that period
- `D` — current price is **below** the snapshot for that period
- `-` — current price **equals** the snapshot, or no snapshot exists yet

### Snapshot behaviour

Snapshots are taken using the `Time` field from the API response (UTC). When the server detects that the API timestamp has crossed into a new time period, it saves the price at that moment as the new snapshot for that period.

- **First poll**: All snapshots initialise to the current price (all indicators will read `-`).
- **Midnight crossing**: The current "today" snapshot moves to "yesterday", and the new price becomes "today".
- **4-hour crossing**: The new price becomes the 4-hour snapshot. Also resets on day change.
- **Hour crossing**: The new price becomes the 1-hour snapshot.
- **15-minute crossing**: The new price becomes the 15-minute snapshot. Also resets on hour change.
- **Minute crossing**: The new price becomes the 1-minute snapshot.

### Startup behaviour

On server startup, all snapshots are uninitialised (value 0). The indicator returns `-` for any position with a 0 snapshot. After the first successful API poll, all snapshots are set to the initial price, so all indicators read `-` until time periods begin to roll over.

## Architecture

| File | Purpose |
|------|---------|
| `server.js` | UDP server, API polling, request/response handling |
| `priceTracker.js` | Price snapshot state, boundary detection, indicator generation |
| `priceTracker.test.js` | Test suite (Node.js built-in test runner) |

## Running

```
npm run server    # Start the UDP server on port 2222
npm test          # Run the test suite
```
