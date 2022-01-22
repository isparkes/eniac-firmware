#include "WiFiManager.h"

void wpsInitConfig()
{
  wps_config.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  wps_config.wps_type = ESP_WPS_MODE;
  strcpy(wps_config.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(wps_config.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(wps_config.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(wps_config.factory_info.device_name, ESP_DEVICE_NAME);
}

String wpspin2string(uint8_t a[])
{
  char wps_pin[9];
  for (int i = 0; i < 8; i++)
  {
    wps_pin[i] = a[i];
  }
  wps_pin[8] = '\0';
  return (String)wps_pin;
}


// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void debugMsgWfm(String message) {
  debugManager.debugMsg("[WFM]: " + message);
}

void debugMsgContWfm(String message) {
  debugManager.debugMsgCont(message);
}

void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    #ifdef DEBUG_ON
    debugMsgWfm("Station Mode Started");
    #endif
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    #ifdef DEBUG_ON
    debugMsgWfm("Connected to :" + WiFi.SSID() + ", password: " + WiFi.psk());
    #endif
    saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    #ifdef DEBUG_ON 
    debugMsgWfm("Disconnected from station, attempting reconnection");
    #endif
    WiFi.reconnect();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    #ifdef DEBUG_ON
    debugMsgWfm("WPS Successfull, stopping WPS and connecting to: " + String(WiFi.SSID()));
    #endif
    esp_wifi_wps_disable();
    delay(10);
    WiFi.begin();
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    #ifdef DEBUG_ON
    debugMsgWfm("WPS Failed, retrying");
    #endif
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    #ifdef DEBUG_ON
    debugMsgWfm("WPS Timedout, retrying");
    #endif
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:
    #ifdef DEBUG_ON
    debugMsgWfm("WPS_PIN = " + wpspin2string(info.sta_er_pin.pin_code));
    #endif
    break;
  default:
    break;
  }
}

void setUpWiFi() {
  WiFi.onEvent(WiFiEvent);

  String mac = String(WiFi.macAddress());
  mac.replace(":","");
  uniqHostname = "ESP32-"+mac.substring(6);

  #ifdef DEBUG_ON
  debugMsgWfm("Unique hostname: " + uniqHostname);
  #endif
  WiFi.setHostname(uniqHostname.c_str());
}

void ScanWiFiNetworks() {
 int n = WiFi.scanNetworks();
  debugMsgWfm("scan done");
  if (n == 0) {
      debugMsgWfm("no networks found");
  } else {
    debugMsgWfm("");
    debugMsgWfm(String(n) + " networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      bool encrypted = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
      String msg = String(i) + " : " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")";
      if (encrypted) {
        msg = msg + " *";
      }
      debugMsgWfm(msg);
    }
  }
}

bool connectToLastAP() {
  if(cc->WiFiSSID.length() > 0) {
    WiFi.begin(cc->WiFiSSID.c_str(), cc->WiFiPassword.c_str());

    #ifdef DEBUG_ON
    debugMsgWfm("");
    debugMsgWfm("Trying to reconnect to last known AP");
    #endif
    oled.showScrollingMessage("Connect to last AP");

    unsigned long maxMillisWiFiWait = millis() + INTERVAL_WIFI;
    while (WiFi.status() != WL_CONNECTED)
    {
      if (previousMillisWiFi < maxMillisWiFiWait)
      {
        previousMillisWiFi = millis();
        #ifdef DEBUG_ON
        debugMsgContWfm(".");
        #endif

        delay(500);
      }
      else {
          #ifdef DEBUG_ON
          debugMsgWfm("");
          debugMsgWfm("Failed to connect");
          debugMsgWfm("");
          #endif
          break;
      }
    }
  } else {
    #ifdef DEBUG_ON
    debugMsgWfm("");
    debugMsgWfm("No AP known. skipping");
    #endif
    oled.showScrollingMessage("No AP known");
  }

  return WiFi.isConnected();
}

void connectWithWPS() {
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG_ON
    debugMsgWfm("");
    debugMsgWfm("Connect using WPS");
    #endif
    oled.showScrollingMessage("Connect using WPS");
    unsigned long maxMillisWiFiWait = millis() + INTERVAL_WPS;
    while (WiFi.status() != WL_CONNECTED)
    {
      wpsInitConfig();
      esp_wifi_wps_enable(&wps_config);

      if (previousMillisWiFi < maxMillisWiFiWait)
      {
        previousMillisWiFi = millis();
        #ifdef DEBUG_ON
        debugMsgContWfm(".");
        #endif

        esp_wifi_wps_start(500);
        delay(500);
      } else {
        #ifdef DEBUG_ON
        debugMsgWfm("");
        debugMsgWfm("Failed to connect");
        debugMsgWfm("");
        #endif
        break;
      }
    }
  }
}

void openAccessPortal() {
  // Captive portal
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG_ON
    debugMsgWfm("");
    debugMsgWfm("Portal mode");
    #endif
    oled.showScrollingMessage("Portal mode");

    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_MODE_APSTA);
    delay(100);
    // WiFi.softAPsetHostname(uniqHostname.c_str());
    delay(100);
    #ifdef DEBUG_ON
    debugMsgWfm("Setting soft-AP configuration ... ");
    #endif
    WiFi.softAP(uniqHostname.c_str());
    delay(100);
    #ifdef DEBUG_ON
    debugMsgWfm("Soft-AP IP address = ");
    debugMsgWfm(WiFi.softAPIP().toString());
    #endif
    oled.showScrollingMessage("IP: " + WiFi.softAPIP().toString());
    delay(500);
  }

  unsigned long maxMillisWiFiWait = millis() + INTERVAL_PORTAL;
  while (WiFi.status() != WL_CONNECTED)
  {
    if (previousMillisWiFi < maxMillisWiFiWait)
    {
      previousMillisWiFi = millis();
      #ifdef DEBUG_ON
      debugMsgContWfm(".");
      #endif
      delay(500);
    } else {
      #ifdef DEBUG_ON
      debugMsgWfm("");
      debugMsgWfm("Failed to connect");
      debugMsgWfm("");
      #endif
      break;
    }
  }
}

void startMDNS() {
  // The MDNS host name does not seem to work at the moment - it is being set by OTA
  if(!MDNS.begin(uniqHostname.c_str())) {
      #ifdef DEBUG_ON
      debugMsgWfm("Error starting mDNS");
      #endif
      return;
  }

  MDNS.addService("http", "tcp", 80);
}

void wifiBeginWithCredentials() {
  WiFi.disconnect();
  delay(1000);
  WiFi.mode(WIFI_MODE_STA);
  delay(1000);
  delay(1000);
  WiFi.begin(cc->WiFiSSID.c_str(), cc->WiFiPassword.c_str());
}

void saveWiFiCredentials(String newWiFiSSID, String newWiFiPassword) {
    
  if (cc->WiFiSSID != newWiFiSSID || cc->WiFiPassword != newWiFiPassword) {
    #ifdef DEBUG_ON
    debugMsgWfm("Updating stored WiFi credentials");
    #endif
    cc->WiFiSSID = newWiFiSSID;
    cc->WiFiPassword = newWiFiPassword;
    spiffsStorage.saveConfigToSpiffs(cc);
    #ifdef DEBUG_ON
    debugMsgWfm("Saved WiFi credentials");
    #endif
  } else {
    #ifdef DEBUG_ON
    debugMsgWfm("No changes to WiFi credentials saved");
    #endif
  }
}

