#include "WebManager.h"

// singleton object
AsyncWebServer server(80);

void WebManager_::begin() {
  #ifdef DEBUG_ON
  debugMsg("Setting up server endpoints");
  #endif
  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");

  // Summary and diagnostics
  server.on("/api/getSummary", HTTP_GET, getSummaryDataHandler);
  server.on("/api/getDiags", HTTP_GET, getDiagsDataHandler);
  server.on("/api/postDiags", HTTP_POST, postDiagsDataHandler);
  
  // Configure time server
  server.on("/api/getTimeserver", HTTP_GET, getTimeserverDataHandler);
  server.on("/api/postTimeserver", HTTP_POST, postTimeserverDataHandler);
  
  // Configure options
  server.on("/api/getConfig", HTTP_GET, getConfigDataHandler);
  server.on("/api/postConfig", HTTP_POST, postConfigDataHandler);

  // wifi credentials
  server.on("/api/postWiFiCredentials", HTTP_POST, postWiFiDataHandler);
  server.on("/api/credentials", HTTP_GET, getCredentialsHandler);
  server.on("/api/getWiFiConnected", HTTP_GET, getWifiConnected);

  // Utilities
  server.on("/utils/resetWifi", HTTP_GET, resetWifiHandler);
  server.on("/utils/scanI2C", HTTP_GET, getI2CScanHandler);
  server.on("/utils/saveStats", HTTP_GET, saveStatsHandler);
  server.on("/utils/ntpupdate", HTTP_GET, [] (AsyncWebServerRequest *request) {
    ntpManager.resetNextUpdate();
        request->redirect("/utility.html");;
    });
  server.on("/utils/resetwifi", HTTP_GET, [] (AsyncWebServerRequest *request) {
    resetWifi();
        request->redirect("/utility.html");;
    });
  server.on("/utils/resetoptions", HTTP_GET, [] (AsyncWebServerRequest *request) {
    resetOptions();
        request->redirect("/utility.html");;
    });
  server.on("/utils/resetall", HTTP_GET, [] (AsyncWebServerRequest *request) {
    resetAll();
        request->redirect("/utility.html");;
    });
  server.on("/utils/restart", HTTP_GET, restartHandler);

  server.onNotFound([](AsyncWebServerRequest *request){
      request->send(404, "text/plain", "The content you are looking for was not found.");
  });

  #ifdef DEBUG_ON
  debugMsg("Start up web server");
  #endif

  server.begin();
}

void WebManager_::doStuff() {
}

// ************************************************************
// Output a logging message to the debug output, if set
// ************************************************************
void WebManager_::debugMsg(String message) {
  if (_dbcb != NULL && _debug) {
    _dbcb("[WEB]: " + message);
  }
}

// ************************************************************
// Set the callback for outputting debug messages
// ************************************************************
void WebManager_::setDebugCallback(DebugCallback dbcb) {
  _dbcb = dbcb;
  debugMsg("Debugging started, callback set");
}

// ************************************************************
// set the update interval
// ************************************************************
void WebManager_::setDebugOutput(bool newDebug) {
  _debug = newDebug;
}

// ************************************************************
// Get singleton instance
// ************************************************************
WebManager_ &WebManager_::getInstance() {
  static WebManager_ instance;
  return instance;
}

WebManager_ &webServer = webServer.getInstance();