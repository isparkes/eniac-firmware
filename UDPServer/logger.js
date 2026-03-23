const LEVELS = { DEBUG: 0, INFO: 1 };

let currentLevel = LEVELS[process.env.LOG_LEVEL] ?? LEVELS.INFO;

function setLevel(level) {
  if (LEVELS[level] === undefined) throw new Error(`Unknown log level: ${level}`);
  currentLevel = LEVELS[level];
}

function debug(...args) {
  if (currentLevel <= LEVELS.DEBUG) console.log('[DEBUG]', ...args);
}

function info(...args) {
  if (currentLevel <= LEVELS.INFO) console.log('[INFO] ', ...args);
}

module.exports = { setLevel, debug, info };
