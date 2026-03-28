#include <Arduino.h>

// ============================================================
// DecatronTest - Serial tester for EniacDecatron slave
//
// Protocol: 5 bytes sent once per second via Serial2 (115200 baud)
//   Byte 0: Start byte (0xAA)
//   Byte 1: Hours   (0-23)
//   Byte 2: Minutes (0-59)
//   Byte 3: Seconds (0-59)
//   Byte 4: Control
//     Bit 0:    Blanked (1 = display blanked)
//     Bits 1-4: Primary display mode (0-15)
//
// Wiring: ESP32 Serial2 TX (GPIO17) -> D1 Mini RX (D9/GPIO3)
//
// Serial commands (115200 baud, newline terminated):
//   send HH MM SS CTRL   - send raw 4 bytes (CTRL is decimal)
//   time HH MM SS        - send time with current mode/blank state
//   blank                - set blanked bit and send
//   unblank              - clear blanked bit and send
//   mode N               - set display mode (0-15) and send
//   auto                 - send incrementing time every second
//   stop                 - stop auto-send
//   help                 - print this help
// ============================================================

#define DECATRON_TX_PIN   12

#define SERIAL_START_BYTE 0xAA

#define CTRL_BLANKED      0x01
#define CTRL_MODE_SHIFT   1

// Current state
uint8_t g_hour    = 0;
uint8_t g_minute  = 0;
uint8_t g_second  = 0;
uint8_t g_mode    = 0;
bool    g_blanked = false;
bool    g_auto    = false;

unsigned long g_lastAutoSend = 0;
unsigned long g_autoStartMs  = 0;

String g_inputBuf = "";

// -------------------------------------------------------
uint8_t buildControl() {
  return (g_blanked ? CTRL_BLANKED : 0) | ((g_mode & 0x0F) << CTRL_MODE_SHIFT);
}

// -------------------------------------------------------
void sendToDecatron(uint8_t h, uint8_t m, uint8_t s, uint8_t ctrl) {
  Serial2.write(SERIAL_START_BYTE);
  Serial2.write(h);
  Serial2.write(m);
  Serial2.write(s);
  Serial2.write(ctrl);
  Serial.printf("  Sent: %02d:%02d:%02d  ctrl=0x%02X (mode=%d, blanked=%d)\n",
                h, m, s, ctrl, (ctrl >> CTRL_MODE_SHIFT) & 0x0F, ctrl & CTRL_BLANKED);
}

// -------------------------------------------------------
void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  send HH MM SS CTRL  - send raw 4 bytes (CTRL decimal)");
  Serial.println("  time HH MM SS       - send time with current mode/blank state");
  Serial.println("  blank               - set blanked bit and send");
  Serial.println("  unblank             - clear blanked bit and send");
  Serial.println("  mode N              - set display mode 0-15 and send");
  Serial.println("  auto                - send incrementing time every second");
  Serial.println("  stop                - stop auto-send");
  Serial.println("  help                - show this help");
  Serial.println();
  Serial.printf("Decatron TX pin: GPIO%d (Serial2)\n", DECATRON_TX_PIN);
  Serial.printf("Current state: %02d:%02d:%02d  mode=%d  blanked=%d\n",
                g_hour, g_minute, g_second, g_mode, g_blanked);
  Serial.println();
}

// -------------------------------------------------------
void processCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  Serial.println("> " + cmd);

  if (cmd == "help") {
    printHelp();

  } else if (cmd == "blank") {
    g_blanked = true;
    sendToDecatron(g_hour, g_minute, g_second, buildControl());

  } else if (cmd == "unblank") {
    g_blanked = false;
    sendToDecatron(g_hour, g_minute, g_second, buildControl());

  } else if (cmd == "auto") {
    g_auto = true;
    g_autoStartMs = millis();
    g_lastAutoSend = 0;
    Serial.println("  Auto-send started. Type 'stop' to halt.");

  } else if (cmd == "stop") {
    g_auto = false;
    Serial.println("  Auto-send stopped.");

  } else if (cmd.startsWith("mode ")) {
    int n = cmd.substring(5).toInt();
    if (n < 0 || n > 15) {
      Serial.println("  Error: mode must be 0-15");
    } else {
      g_mode = (uint8_t)n;
      sendToDecatron(g_hour, g_minute, g_second, buildControl());
    }

  } else if (cmd.startsWith("time ")) {
    int h, m, s;
    if (sscanf(cmd.c_str() + 5, "%d %d %d", &h, &m, &s) == 3) {
      if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) {
        Serial.println("  Error: values out of range");
      } else {
        g_hour = h; g_minute = m; g_second = s;
        sendToDecatron(g_hour, g_minute, g_second, buildControl());
      }
    } else {
      Serial.println("  Usage: time HH MM SS");
    }

  } else if (cmd.startsWith("send ")) {
    int h, m, s, ctrl;
    if (sscanf(cmd.c_str() + 5, "%d %d %d %d", &h, &m, &s, &ctrl) == 4) {
      sendToDecatron((uint8_t)h, (uint8_t)m, (uint8_t)s, (uint8_t)ctrl);
    } else {
      Serial.println("  Usage: send HH MM SS CTRL");
    }

  } else {
    Serial.println("  Unknown command. Type 'help' for usage.");
  }
}

// -------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(500);

  Serial2.begin(115200, SERIAL_8N1, -1, DECATRON_TX_PIN);  // TX only

  Serial.println();
  Serial.println("=== DecatronTest Serial Tester ===");
  printHelp();
}

// -------------------------------------------------------
void loop() {
  // Read serial input
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (g_inputBuf.length() > 0) {
        processCommand(g_inputBuf);
        g_inputBuf = "";
      }
    } else {
      g_inputBuf += c;
    }
  }

  // Auto-send: increment time each second
  if (g_auto) {
    unsigned long now = millis();
    if (now - g_lastAutoSend >= 1000) {
      g_lastAutoSend = now;
      sendToDecatron(g_hour, g_minute, g_second, buildControl());
      g_second++;
      if (g_second > 59) { g_second = 0; g_minute++; }
      if (g_minute > 59) { g_minute = 0; g_hour++; }
      if (g_hour   > 23) { g_hour = 0; }
    }
  }
}
