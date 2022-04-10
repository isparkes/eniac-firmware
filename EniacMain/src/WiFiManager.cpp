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

void WiFiEvent(WiFiEvent_t event, system_event_info_t info)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    debugMsgWfm("Station Mode Started");
    break;
  case SYSTEM_EVENT_AP_START:
    debugMsgWfm("AP Mode Started");
    wifiManager.startWiFiServicesPortal();
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    debugMsgWfm("Connected to:" + WiFi.SSID() + ", password: " + WiFi.psk());
    debugMsgWfm("IP Address: " + WiFi.localIP().toString());
    debugMsgWfm("MAC Address: " + WiFi.macAddress());
    debugMsgWfm("Host name: " + String(WiFi.getHostname()));
    wifiManager.saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
    wifiManager.startWiFiServices();
    flashMenuMessage("WiFi Status", "WiFi connected to\n"+WiFi.SSID());
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    debugMsgWfm("Disconnected from station");
    if (doAutoReconnect) {
      debugMsgWfm("autoreconnect on, trying reconnect");
      WiFi.reconnect();
    }
    break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:
    debugMsgWfm("WPS Successfull, saving credentials. SSID: |" + WiFi.SSID() + "| password: |" + WiFi.psk() + "|");
    wifiManager.saveWiFiCredentials(WiFi.SSID(), WiFi.psk());
    esp_wifi_wps_disable();
    flashMenuMessage("WPS Status", "WPS was successful\nPassword:\n"+WiFi.psk());
    break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:
    debugMsgWfm("WPS Failed, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:
    debugMsgWfm("WPS Timedout, retrying");
    esp_wifi_wps_disable();
    esp_wifi_wps_enable(&wps_config);
    esp_wifi_wps_start(0);
    break;
  case SYSTEM_EVENT_SCAN_DONE:
    debugMsgWfm("Scan complete");
    wifiManager.processScanResults();
    break;
  default:
    break;
  }
}

void WiFiManager_::setUpWiFi() {
  WiFi.onEvent(WiFiEvent);

  String mac = String(WiFi.macAddress());
  mac.replace(":","");
  uniqHostname = "ESP32-"+mac.substring(6);

  debugMsgWfm("Unique hostname: " + uniqHostname);
  WiFi.setHostname(uniqHostname.c_str());
}

void WiFiManager_::startScanWiFiNetworks() {
  WiFi.onEvent(WiFiEvent, SYSTEM_EVENT_SCAN_DONE);

  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();
  delay(500);

  WiFi.scanNetworks(true);
}

// ************************************************************
// Process the results of the SSID scan
// ************************************************************
void WiFiManager_::processScanResults() {
  int n = WiFi.scanComplete();
  if (n == 0) {
    debugMsgWfm("no networks found");
    flashMenuMessage("Scan Done", "No WiFi\nnetworks\nfound.");
  } else {
    debugMsgWfm("");
    debugMsgWfm(String(n) + " networks found");
    flashMenuMessage("Scan Done", "Found " + String(n) + " networks.");
    String result = "";
    for (int i = 0; i < n; ++i) {
      if (ssidList.containsIgnoreCase(WiFi.SSID(i))) {
        debugMsgWfm("Already have: " + WiFi.SSID(i));
      } else {
        debugMsgWfm("Add: " + WiFi.SSID(i));
        ssidList.add(WiFi.SSID(i));
      }
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
    debugMsgWfm("Returning network list: " + result);
    lastWiFiScan = result;
  }
}

// ************************************************************
// Get the number of SSIDs we found so far
// ************************************************************
int WiFiManager_::getLastScanResultCount() {
  return ssidList.length();
}

// ************************************************************
// Get the nth entry in the SSID list
// ************************************************************
String WiFiManager_::getLastScanResultSSID(int index) {
  auto ssid = ssidList.nth(index);
  return *ssid;
}

// ************************************************************
// Start up Smart Config - fire and forget
// http://www.iotsharing.com/2017/05/how-to-use-smartconfig-on-esp32.html
// ************************************************************
void WiFiManager_::startSmartConfig() {
  flashMenuMessage("Smartconfig", "Starting\nSmartconfig\nmode.");
  WiFi.disconnect();
  delay(500);

  WiFi.mode(WIFI_AP_STA);
  WiFi.beginSmartConfig();
}

// ************************************************************
// Reconnect to the last registered SSID
// ************************************************************
void WiFiManager_::connectToLastAP() {
  if(wifiCredentialsReceived()) {
    debugMsgWfm("Trying to reconnect to last known AP");
    flashMenuMessage("Reconnect","Reconnecting to:\n" + cc->WiFiSSID + "\n");
    wifiBeginWithCredentials();
  }
}

// ************************************************************
// Kick off WPS connect - fire and forget
// ************************************************************
bool WiFiManager_::connectWithWPS() {
  // Autoreconnect is needed for WPS!
  doAutoReconnect = true;

  if (WiFi.status() != WL_CONNECTED) {
    debugMsgWfm("Connect using WPS");
    // ToDo show this status better
    flashMenuMessage("WPS", "Connect using WPS");

    WiFi.mode(WIFI_STA);
    delay(1000);
      
    wpsInitConfig();

    esp_err_t retCodeEnable = esp_wifi_wps_enable(&wps_config);
    debugMsgWfm("WPS Enable Result: " + String(retCodeEnable));

    esp_err_t retCodeStart = esp_wifi_wps_start(0);
    debugMsgWfm("WPS Start Result: " + String(retCodeStart));

    return (retCodeEnable == 0 && retCodeStart == 0);
  } else {
    debugMsgWfm("Already connected, won't do WPS");
    return false;
  }
}

// ************************************************************
// Start up AP mode
// ************************************************************
void WiFiManager_::openAccessPortal() {
  // Captive portal
  if (WiFi.status() != WL_CONNECTED) {
    startScanWiFiNetworks();

    debugMsgWfm("");
    debugMsgWfm("Portal mode");
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    delay(100);
    debugMsgWfm("Setting soft-AP configuration ... ");
    WiFi.softAP(uniqHostname.c_str());
    delay(100);
    debugMsgWfm("Soft-AP IP address = ");
    debugMsgWfm(WiFi.softAPIP().toString());
    flashMenuMessage("Portal", "Opened access\nportal at IP: " + WiFi.softAPIP().toString());
  } else {
    flashMenuMessage("Portal", "WiFi is already\nconnected to:\n" + WiFi.SSID());
  }
}

// ************************************************************
// Start up mDNS
// ************************************************************
void WiFiManager_::startMDNS() {
  // The MDNS host name does not seem to work at the moment - it is being set by OTA
  if(!MDNS.begin(uniqHostname.c_str())) {
      debugMsgWfm("Error starting mDNS");
      return;
  }

  MDNS.addService("http", "tcp", 80);
}

// ************************************************************
// Start up the web server for normal use
// ************************************************************
void WiFiManager_::startWiFiServices() {
  if (WiFi.isConnected()) {
    // -------------------------------------------------------------------------

    debugMsgWfm("Start up NTP Time Updates...");
    nowMillis = millis();
    ntpManager.getTimeFromNTP();

    // -------------------------------------------------------------------------

    debugMsgWfm("Start up WebServer" );
    webManager.begin();

    // -------------------------------------------------------------------------
    
    debugMsgWfm("Start up OTA");
    webManager.startOTA();

    // -------------------------------------------------------------------------
    
    debugMsgWfm("Start up mDNS on http://" + String(WiFi.getHostname()) + ".local");
    startMDNS();
  } else {
    debugMsgWfm("No WiFi, skipping web services startup");
  }
}

// ************************************************************
// Start the web server with Access Portal set up
// ************************************************************
void WiFiManager_::startWiFiServicesPortal() {
  debugMsgWfm("Start up WebServer for Portal services" );

  webManager.beginPortal();
}

// ************************************************************
// Startup th WiFi using the credentials we have
// ************************************************************
void WiFiManager_::wifiBeginWithCredentials() {
  WiFi.disconnect();
  delay(1000);
  WiFi.mode(WIFI_MODE_STA);
  delay(1000);
  WiFi.begin(cc->WiFiSSID.c_str(), cc->WiFiPassword.c_str());
}

// ************************************************************
// Save the credentials to SPIFFS if they are valid
// ************************************************************
void WiFiManager_::saveWiFiCredentials(String newWiFiSSID, String newWiFiPassword) {
  if ((cc->WiFiSSID != newWiFiSSID || 
      cc->WiFiPassword != newWiFiPassword) && 
      newWiFiSSID.length() > 0 &&
      newWiFiPassword.length() > 0) {
    debugMsgWfm("Updating stored WiFi credentials");
    cc->WiFiSSID = newWiFiSSID;
    cc->WiFiPassword = newWiFiPassword;
    cc->WifiOnAtStart = true;
    spiffsStorage.saveConfigToSpiffs();
    debugMsgWfm("Saved WiFi credentials");
  } else {
    debugMsgWfm("No changes to WiFi credentials saved");
  }
}

// ************************************************************
// Undock from the WiFi mothership
// ************************************************************
void WiFiManager_::disconnectWiFi() {
  WiFi.disconnect();
}

// ************************************************************
// Clear down credentials
// ************************************************************
void WiFiManager_::resetWiFiCredentials() {
  cc->WiFiSSID = "";
  cc->WiFiPassword = "";
  cc->WifiOnAtStart = false;
  spiffsStorage.saveConfigToSpiffs();
}

// ************************************************************
// Return true if we have received and stored some credentials
// ************************************************************
bool WiFiManager_::wifiCredentialsReceived() {
  return (cc->WiFiSSID.length() > 0 && cc->WiFiPassword.length() > 0);
}

// ************************************************************
// Library internal singleton wiring
// ************************************************************
WiFiManager_ &WiFiManager_::getInstance() {
  static WiFiManager_ instance;
  return instance;
}

WiFiManager_ &wifiManager = wifiManager.getInstance();