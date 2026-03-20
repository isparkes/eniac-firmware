// Price snapshots for up/down indicators
let priceYesterday = 0;  // snapshotted at midnight crossing
let priceToday = 0;      // snapshotted at midnight crossing
let price4h = 0;         // snapshotted every 4 hours
let price1h = 0;         // snapshotted every hour
let price15m = 0;        // snapshotted every 15 minutes
let price1m = 0;         // snapshotted every minute

// Track which periods we've last seen to detect boundary crossings
let lastDay = -1;
let last4h = -1;
let lastHour = -1;
let last15m = -1;
let lastMinute = -1;

function compare(current, reference) {
  if (reference === 0) return '-';
  if (current > reference) return 'U';
  if (current < reference) return 'D';
  return '-';
}

function getIndicator(currentPrice) {
  return compare(currentPrice, priceYesterday)
       + compare(currentPrice, priceToday)
       + compare(currentPrice, price4h)
       + compare(currentPrice, price1h)
       + compare(currentPrice, price15m)
       + compare(currentPrice, price1m);
}

function updateSnapshots(price, apiTime) {
  const t = new Date(apiTime);
  const day = t.getUTCDate();
  const hour = t.getUTCHours();
  const fourHourBlock = Math.floor(hour / 4);
  const minute = t.getUTCMinutes();
  const fifteenMinBlock = Math.floor(minute / 15);

  if (lastDay === -1) {
    // First run: initialise all snapshots to current price
    priceYesterday = price;
    priceToday = price;
    price4h = price;
    price1h = price;
    price15m = price;
    price1m = price;
  } else {
    if (day !== lastDay) {
      priceYesterday = priceToday;
      priceToday = price;
    }
    if (fourHourBlock !== last4h || day !== lastDay) {
      price4h = price;
    }
    if (hour !== lastHour) {
      price1h = price;
    }
    if (fifteenMinBlock !== last15m || hour !== lastHour) {
      price15m = price;
    }
    if (minute !== lastMinute) {
      price1m = price;
    }
  }

  lastDay = day;
  last4h = fourHourBlock;
  lastHour = hour;
  last15m = fifteenMinBlock;
  lastMinute = minute;
}

function reset() {
  priceYesterday = 0;
  priceToday = 0;
  price4h = 0;
  price1h = 0;
  price15m = 0;
  price1m = 0;
  lastDay = -1;
  last4h = -1;
  lastHour = -1;
  last15m = -1;
  lastMinute = -1;
}

function getSnapshots() {
  return { priceYesterday, priceToday, price4h, price1h, price15m, price1m };
}

module.exports = { compare, getIndicator, updateSnapshots, reset, getSnapshots };
