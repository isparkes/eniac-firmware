const { describe, it, beforeEach } = require('node:test');
const assert = require('node:assert/strict');
const { compare, getIndicator, updateSnapshots, reset, getSnapshots } = require('./priceTracker');

describe('compare', () => {
  it('returns - when reference is 0 (uninitialised)', () => {
    assert.equal(compare(100, 0), '-');
  });

  it('returns U when current > reference', () => {
    assert.equal(compare(100, 90), 'U');
  });

  it('returns D when current < reference', () => {
    assert.equal(compare(90, 100), 'D');
  });

  it('returns - when current === reference', () => {
    assert.equal(compare(100, 100), '-');
  });
});

describe('updateSnapshots', () => {
  beforeEach(() => {
    reset();
  });

  it('initialises all snapshots to current price on first call', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    const s = getSnapshots();
    assert.equal(s.priceYesterday, 50000);
    assert.equal(s.priceToday, 50000);
    assert.equal(s.price4h, 50000);
    assert.equal(s.price1h, 50000);
    assert.equal(s.price15m, 50000);
    assert.equal(s.price1m, 50000);
  });

  it('does not update snapshots when time period has not changed', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    updateSnapshots(51000, '2026-02-01T10:30:30Z'); // same minute
    const s = getSnapshots();
    // All snapshots should still be 50000 from first call
    assert.equal(s.priceYesterday, 50000);
    assert.equal(s.priceToday, 50000);
    assert.equal(s.price4h, 50000);
    assert.equal(s.price1h, 50000);
    assert.equal(s.price15m, 50000);
    assert.equal(s.price1m, 50000);
  });

  it('updates price1m when minute changes', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    updateSnapshots(51000, '2026-02-01T10:31:00Z');
    const s = getSnapshots();
    assert.equal(s.price1m, 50000); // snapshot holds the previous minute's price
    // Others unchanged
    assert.equal(s.price15m, 50000);
    assert.equal(s.price1h, 50000);
  });

  it('updates price15m when 15-minute block changes', () => {
    updateSnapshots(50000, '2026-02-01T10:14:00Z');
    updateSnapshots(51000, '2026-02-01T10:15:00Z'); // crosses into next 15m block
    const s = getSnapshots();
    assert.equal(s.price15m, 51000);
    assert.equal(s.price1h, 50000); // same hour
  });

  it('does not update price15m within same 15-minute block', () => {
    updateSnapshots(50000, '2026-02-01T10:15:00Z');
    updateSnapshots(51000, '2026-02-01T10:29:00Z'); // still in 15-29 block
    const s = getSnapshots();
    assert.equal(s.price15m, 50000);
  });

  it('updates price1h when hour changes', () => {
    updateSnapshots(50000, '2026-02-01T10:59:00Z');
    updateSnapshots(51000, '2026-02-01T11:00:00Z');
    const s = getSnapshots();
    assert.equal(s.price1h, 51000);
    assert.equal(s.price4h, 50000); // same 4h block (8-11)
  });

  it('updates price4h when 4-hour block changes', () => {
    updateSnapshots(50000, '2026-02-01T11:59:00Z');
    updateSnapshots(51000, '2026-02-01T12:00:00Z'); // crosses from block 2 (8-11) to block 3 (12-15)
    const s = getSnapshots();
    assert.equal(s.price4h, 51000);
  });

  it('does not update price4h within same 4-hour block', () => {
    updateSnapshots(50000, '2026-02-01T08:00:00Z');
    updateSnapshots(51000, '2026-02-01T11:59:00Z');
    const s = getSnapshots();
    assert.equal(s.price4h, 50000);
  });

  it('updates priceToday and priceYesterday when day changes', () => {
    updateSnapshots(50000, '2026-02-01T23:59:00Z');
    updateSnapshots(51000, '2026-02-02T00:00:00Z');
    const s = getSnapshots();
    assert.equal(s.priceYesterday, 50000); // previous priceToday becomes priceYesterday
    assert.equal(s.priceToday, 51000);
  });

  it('also resets 4h block when day changes', () => {
    updateSnapshots(50000, '2026-02-01T23:59:00Z'); // 4h block 5 (20-23)
    updateSnapshots(51000, '2026-02-02T00:00:00Z');  // 4h block 0 (0-3), new day
    const s = getSnapshots();
    assert.equal(s.price4h, 51000);
  });

  it('cascades all updates on hour boundary that is also a 15m and 4h boundary', () => {
    updateSnapshots(50000, '2026-02-01T03:59:00Z');
    updateSnapshots(51000, '2026-02-01T04:00:00Z'); // new hour, new 15m block, new 4h block
    const s = getSnapshots();
    assert.equal(s.price4h, 51000);
    assert.equal(s.price1h, 51000);
    assert.equal(s.price15m, 51000);
    assert.equal(s.price1m, 50000); // holds previous minute's price
  });
});

describe('getIndicator', () => {
  beforeEach(() => {
    reset();
  });

  it('returns all dashes before any snapshots are taken', () => {
    assert.equal(getIndicator(50000), '------');
  });

  it('returns all dashes right after first snapshot (price equals all snapshots)', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    assert.equal(getIndicator(50000), '------');
  });

  it('returns all U when price has risen above all snapshots', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    assert.equal(getIndicator(51000), 'UUUUUU');
  });

  it('returns all D when price has dropped below all snapshots', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    assert.equal(getIndicator(49000), 'DDDDDD');
  });

  it('returns correct length of 6 characters', () => {
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    const indicator = getIndicator(50000);
    assert.equal(indicator.length, 6);
  });

  it('shows mixed indicators when snapshots differ', () => {
    // Set initial snapshots at 50000
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    // Advance 1 minute - price1m holds previous price (50000)
    updateSnapshots(51000, '2026-02-01T10:31:00Z');
    // Current price is 50500 - above all snapshots (all anchored at 50000)
    const indicator = getIndicator(50500);
    // yesterday=50000(U), today=50000(U), 4h=50000(U), 1h=50000(U), 15m=50000(U), 1m=50000(U)
    assert.equal(indicator, 'UUUUUU');
  });

  it('tracks price movement across multiple time boundaries', () => {
    // Initial price
    updateSnapshots(50000, '2026-02-01T10:30:00Z');
    // Price drops, minute changes - price1m holds 50000 (previous)
    updateSnapshots(49000, '2026-02-01T10:31:00Z');
    // Price rises, 15-min block changes - price15m=51000, price1m holds 49000 (previous)
    updateSnapshots(51000, '2026-02-01T10:45:00Z');

    // Current price 50000:
    // yesterday=50000(-), today=50000(-), 4h=50000(-), 1h=50000(-), 15m=51000(D), 1m=49000(U)
    assert.equal(getIndicator(50000), '----DU');
  });
});

describe('getIndicator across day boundary', () => {
  beforeEach(() => {
    reset();
  });

  it('reflects day change in yesterday indicator', () => {
    updateSnapshots(50000, '2026-02-01T23:59:00Z');
    updateSnapshots(52000, '2026-02-02T00:00:00Z');

    // yesterday=50000, today=52000
    // Current price 51000: above yesterday(U), below today(D)
    const indicator = getIndicator(51000);
    assert.equal(indicator[0], 'U'); // vs yesterday
    assert.equal(indicator[1], 'D'); // vs today
  });
});

describe('edge cases', () => {
  beforeEach(() => {
    reset();
  });

  it('handles very small price differences', () => {
    updateSnapshots(50000.01, '2026-02-01T10:30:00Z');
    assert.equal(compare(50000.02, 50000.01), 'U');
    assert.equal(compare(50000.00, 50000.01), 'D');
  });

  it('handles multiple day rollovers', () => {
    updateSnapshots(50000, '2026-02-01T12:00:00Z');
    updateSnapshots(51000, '2026-02-02T12:00:00Z');
    updateSnapshots(52000, '2026-02-03T12:00:00Z');
    const s = getSnapshots();
    assert.equal(s.priceYesterday, 51000); // day 2's priceToday
    assert.equal(s.priceToday, 52000);     // day 3's price
  });

  it('handles midnight exactly (hour 0, minute 0)', () => {
    updateSnapshots(50000, '2026-02-01T00:00:00Z');
    const s = getSnapshots();
    assert.equal(s.price4h, 50000);  // 4h block 0
    assert.equal(s.price1h, 50000);  // hour 0
    assert.equal(s.price15m, 50000); // 15m block 0
    assert.equal(s.price1m, 50000);  // minute 0
  });

  it('handles end of day (hour 23, minute 59)', () => {
    updateSnapshots(50000, '2026-02-01T23:59:00Z');
    updateSnapshots(51000, '2026-02-02T00:00:00Z');
    const s = getSnapshots();
    // Everything should have rolled over
    assert.equal(s.priceYesterday, 50000);
    assert.equal(s.priceToday, 51000);
    assert.equal(s.price4h, 51000);
    assert.equal(s.price1h, 51000);
    assert.equal(s.price15m, 51000);
    assert.equal(s.price1m, 50000); // holds previous minute's price
  });
});
