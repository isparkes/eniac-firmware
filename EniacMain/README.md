

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