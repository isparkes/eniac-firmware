

On JSON POSTs, don't set the content type!


Timeserver api
==============
curl -v -X GET "http://esp32-612ce4.local/api/getTimeserver"

curl -v -X POST -d '{"ntpPool":"pool","ntpUpdateInterval":7262,"tzs":"CEST"}' "http://esp32-612ce4.local/api/postTimeserver"

WiFi credentials
================
curl -v -X POST -d '{"SSID":"TESTSSID","password":"1234"}' "http://esp32-612ce4.local/api/postWiFiCredentials"

Release versions
v0.6.0.0
Add Decatron slave type

v0.6.0.1
Allow split dimming of tubes and LEDs

v0.6.0.2
Fix tower dimming
Event driven switch handling

v0.6.0.3
Correct backlights

v0.6.0.4
Add Reverse Digit "RD" tag to Features string

v0.6.0.5
Add UDP server, update blanking mode manager

v0.6.0.6
Fix tower blanking, fix quote server

v0.6.0.8
New UART serial protocol for Decatron slave
- Replaces I2C with unidirectional UART (Serial2, 115200 8N1, TX on GPIO0)
- 5-byte framed packet: 0xAA | Hour | Minute | Second | Control
- More Decatron display options via control byte mode field
- Stop blue status LED flashing when display is blanked
- Remove cog crank output (GPIO0 repurposed as UART TX to Decatron)

v0.6.0.7
Enhanced ticker display with 6 trend indicators
- Each digit now shows its own trend indicator via LED backlight color
- Indicator periods: Yesterday, Today, 4h, 1h, 15m, 1m
- Colors: Green=Up, Red=Down, Off=Unchanged
