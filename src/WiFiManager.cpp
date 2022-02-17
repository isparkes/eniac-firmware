#include "WiFiManager.h"

// forward private decls
void processScanResults();

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
  case SYSTEM_EVENT_AP_START:
    #ifdef DEBUG_ON
    debugMsgWfm("AP Mode Started");
    #endif
    startWiFiServicesPortal();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    #ifdef DEBUG_ON
    debugMsgWfm("Connected to:" + WiFi.SSID() + ", password: " + WiFi.psk());
    debugMsgWfm("IP Address: " + WiFi.localIP().toString());
    debugMsgWfm("MAC Address: " + WiFi.macAddress());
    debugMsgWfm("Host name: " + String(WiFi.getHostname()));
    #endif
    saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
    startWiFiServices();
    flashMenuMessage("WiFi Status", "WiFi connected to\nSSID:\n"+WiFi.SSID());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    #ifdef DEBUG_ON 
    debugMsgWfm("Disconnected from station");
    #endif
    if (doAutoReconnect) {
      #ifdef DEBUG_ON 
      debugMsgWfm("autoreconnect on, trying reconnect");
      #endif
      WiFi.reconnect();
    }
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    #ifdef DEBUG_ON
    debugMsgWfm("WPS Successfull, saving credentials. SSID: |" + WiFi.SSID() + "| password: |" + WiFi.psk() + "|");
    #endif
    saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
    esp_wifi_wps_disable();
    flashMenuMessage("WPS Status", "WPS was successful\nPassword:\n"+WiFi.psk());
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
  case SYSTEM_EVENT_SCAN_DONE:
    #ifdef DEBUG_ON
    debugMsgWfm("Scan complete");
    #endif
    processScanResults();
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

void startScanWiFiNetworks() {
  WiFi.onEvent(WiFiEvent, SYSTEM_EVENT_SCAN_DONE);

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  delay(500);

  WiFi.scanNetworks(true);
}

void processScanResults() {
  int n = WiFi.scanComplete();
  if (n == 0) {
    #ifdef DEBUG_ON
    debugMsgWfm("no networks found");
    #endif
    flashMenuMessage("Scan Done", "No WiFi\nnetworks\nfound.");
  } else {
    #ifdef DEBUG_ON
    debugMsgWfm("");
    debugMsgWfm(String(n) + " networks found");
    #endif
    flashMenuMessage("Scan Done", "Found\n" + String(n) + "\nnetworks.");
    String result = "";
    for (int i = 0; i < n; ++i) {
      #ifdef DEBUG_ON
      // Print SSID and RSSI for each network found
      bool encrypted = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
      String msg = String(i) + " : " + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")";
      if (encrypted) {
        msg = msg + " *";
      }
      debugMsgWfm(msg);
      #endif
      if (result.length() > 0) {
        result = result + ",";
      }
      result = result + WiFi.SSID(i);
    }
    #ifdef DEBUG_ON
    debugMsgWfm("Returning network list: " + result);
    #endif
    lastWiFiScan = result;
  }
}

// http://www.iotsharing.com/2017/05/how-to-use-smartconfig-on-esp32.html
void startSmartConfig() {
  flashMenuMessage("Smartconfig", "Starting\"Smartconfig""\nmode.");
  WiFi.disconnect();
  delay(500);

  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
}

void connectToLastAP() {
  if(wifiCredentialsReceived()) {
    #ifdef DEBUG_ON
    debugMsgWfm("Trying to reconnect to last known AP");
    #endif
    flashMenuMessage("Reconnect","Trying to reconnect to:\nSSID:\n" + cc->WiFiSSID + "\n");
    wifiBeginWithCredentials();
  }
}

bool connectWithWPS() {
  // Autoreconnect is needed for WPS!
  doAutoReconnect = true;

  if (WiFi.status() != WL_CONNECTED) {
    #ifdef DEBUG_ON
    debugMsgWfm("Connect using WPS");
    #endif
    // ToDo show this status better
    // oled.showScrollingMessage("Connect using WPS");

    WiFi.mode(WIFI_STA);
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
    debugMsgWfm("Already connected, won't do WPS");
    #endif
    return false;
  }
}

void openAccessPortal() {
  // Captive portal
  if (WiFi.status() != WL_CONNECTED) {
    // preload the wifi list 
    startScanWiFiNetworks();

    #ifdef DEBUG_ON
    debugMsgWfm("");
    debugMsgWfm("Portal mode");
    #endif
    oled.showScrollingMessage("Portal mode");

    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_AP_STA);
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
  } else {
    #ifdef DEBUG_ON
    debugMsgWfm("No WiFi, skipping web services startup");
    #endif
  }
}

void startWiFiServicesPortal() {
  #ifdef DEBUG_ON
  debugMsgWfm("Start up WebServer for Portal services" );
  #endif

  webManager.beginPortal();
}

void wifiBeginWithCredentials() {
  WiFi.disconnect();
  delay(1000);
  WiFi.mode(WIFI_MODE_STA);
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