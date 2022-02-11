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
    debugMsgWfm("IP Address: " + WiFi.localIP().toString());
    debugMsgWfm("MAC Address: " + WiFi.macAddress());
    debugMsgWfm("Host name: " + String(WiFi.getHostname()));
    flashMenuMessage("WiFi Status", "WiFi connected to\nSSID:\n"+WiFi.SSID());
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
    debugMsgWfm("WPS Successfull, saving credentials: " + WiFi.SSID() + ", password: " + WiFi.psk());
    #endif
    saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
    esp_wifi_wps_disable();
    flashMenuMessage("WPS Status", "WiFi connected to\nSSID:\n"+WiFi.SSID());
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

void scanWiFiNetworks() {
  if (WiFi.isConnected()) {
    debugMsgWfm("No scan done - WiFi is connected");
  } else {
    WiFi.mode(WIFI_MODE_STA);
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
}

void connectToLastAP() {
  if(wifiCredentialsReceived()) {
    #ifdef DEBUG_ON
    debugMsgWfm("Trying to reconnect to last known AP");
    #endif
    oled.showScrollingMessage("Connect to last AP");
    wifiBeginWithCredentials();

  //   unsigned long maxMillisWiFiWait = millis() + INTERVAL_WIFI;
  //   while (WiFi.status() != WL_CONNECTED)
  //   {
  //     if (previousMillisWiFi < maxMillisWiFiWait)
  //     {
  //       previousMillisWiFi = millis();
  //       #ifdef DEBUG_ON
  //       debugMsgContWfm(".");
  //       #endif

  //       delay(500);
  //     }
  //     else {
  //         #ifdef DEBUG_ON
  //         debugMsgWfm("");
  //         debugMsgWfm("Failed to connect");
  //         debugMsgWfm("");
  //         #endif
  //         break;
  //     }
  //   }
  // } else {
  //   #ifdef DEBUG_ON
  //   debugMsgWfm("");
  //   debugMsgWfm("No AP known. skipping");
  //   #endif
  //   oled.showScrollingMessage("No AP known");
  // }
  }
}

bool connectWithWPS() {
  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG_ON
    debugMsgWfm("Connect using WPS");
    #endif
    // ToDo show this status better
    // oled.showScrollingMessage("Connect using WPS");

    WiFi.mode(WIFI_MODE_STA);
    delay(1000);
      
    wpsInitConfig();

    esp_err_t retCodeEnable = esp_wifi_wps_enable(&wps_config);
    #ifdef DEBUG_ON
    debugMsgWfm("WPS Enable Result: " + String(retCodeEnable));
    #endif

    esp_err_t retCodeStart = esp_wifi_wps_start(0);
    #ifdef DEBUG_ON
    debugMsgWfm("WPS Start Result: " + String(retCodeStart));
    #endif

    return (retCodeEnable == 0 && retCodeStart == 0);
  } else {
    #ifdef DEBUG_ON
    debugMsgWfm("Already connected, cannot do WPS");
    #endif
    return false;
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


void startWiFiServices() {
  if (WiFi.isConnected()) {
    // -------------------------------------------------------------------------

    #ifdef DEBUG_ON
    debugMsgWfm("Start up NTP Time Updates...");
    #endif
    nowMillis = millis();
    ntpManager.getTimeFromNTP();

    // -------------------------------------------------------------------------

    #ifdef DEBUG_ON
    debugMsgWfm("Start up WebServer" );
    #endif

    webManager.begin();

    // -------------------------------------------------------------------------
    
    #ifdef DEBUG_ON
    debugMsgWfm("Start up OTA");
    #endif
    webManager.startOTA();

    // -------------------------------------------------------------------------
    
    #ifdef DEBUG_ON
    debugMsgWfm("Start up mDNS on http://" + String(WiFi.getHostname()) + ".local");
    #endif
    startMDNS();

    wifiServicesWereInitalised = true;
  } else {
    #ifdef DEBUG_ON
    debugMsgWfm("No WiFi, skipping web services startup");
    #endif
  }
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
  if ((cc->WiFiSSID != newWiFiSSID || 
      cc->WiFiPassword != newWiFiPassword) && 
      newWiFiSSID.length() > 0 &&
      newWiFiPassword.length() > 0) {
    #ifdef DEBUG_ON
    debugMsgWfm("Updating stored WiFi credentials");
    #endif
    cc->WiFiSSID = newWiFiSSID;
    cc->WiFiPassword = newWiFiPassword;
    cc->WifiOnAtStart = true;
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

void disconnectWiFi() {
  WiFi.disconnect();
}

void resetWiFiCredentials() {
  cc->WiFiSSID = "";
  cc->WiFiPassword = "";
  cc->WifiOnAtStart = false;
  spiffsStorage.saveConfigToSpiffs(cc);
}

bool wifiCredentialsReceived() {
  return (cc->WiFiSSID.length() > 0 && cc->WiFiPassword.length() > 0);
}