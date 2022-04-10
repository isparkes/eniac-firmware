#include "WebManager.h"

// This include has to be here, anbd not in the header file
#include <AsyncElegantOTA.h>

// ************************************************************
// Open up the normal page handlers
// ************************************************************
void WebManager_::begin() {
  debugMsgWbm("Setting up server endpoints");
  server.reset();
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
  server.on("/api/postWiFiCredentials", HTTP_POST, postWiFiCredentialsHandler);
  server.on("/api/credentials", HTTP_GET, getCredentialsHandler);

  // Utilities
  server.on("/utils/resetwifi", HTTP_GET, resetWifiHandler);
  server.on("/utils/scanI2C", HTTP_GET, getI2CScanHandler);
  server.on("/utils/saveStats", HTTP_GET, saveStatsHandler);
  server.on("/utils/ntpupdate", HTTP_GET, [] (AsyncWebServerRequest *request) {
    ntpManager.resetNextUpdate();
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

  debugMsgWbm("Start up web server");

  server.begin();
}

// ************************************************************
// Handler for the captive page
// ************************************************************
class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
//    debugMsgWbm("Handling URL: " + request->url());
    if (request->url().startsWith("/api/")) return false;
    if (request->url().startsWith("/utils/")) return false;
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    debugMsgWbm("Sending captive page");
    request->send(SPIFFS, "/portal.html", String(), false);
  }
};

// ************************************************************
// Open up the Portal Page
// ************************************************************
void WebManager_::beginPortal() {
  debugMsgWbm("Setting up server endpoints for Portal");
  server.reset();
  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("portal.html");

  // serve the captive page
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

  // Summary and diagnostics
  server.on("/api/getSummary", HTTP_GET, getSummaryDataHandler);
  server.on("/api/getDiags", HTTP_GET, getDiagsDataHandler);
  server.on("/api/postDiags", HTTP_POST, postDiagsDataHandler);
  
  // wifi credentials
  server.on("/api/postWiFiCredentials", HTTP_POST, postWiFiCredentialsHandler);
  server.on("/api/credentials", HTTP_GET, getCredentialsHandler);
  server.on("/api/getWiFiNetworks", HTTP_GET, getWiFiNetworksHandler);

  // Utilities
  server.on("/utils/resetwifi", HTTP_GET, resetWifiHandler);
  server.on("/utils/scanI2C", HTTP_GET, getI2CScanHandler);
  server.on("/utils/saveStats", HTTP_GET, saveStatsHandler);

  debugMsgWbm("Start up web server");

  server.begin();

  // All your requests are belong to us
  DNSServer dnsServer;
  dnsServer.start(53, "*", WiFi.softAPIP());
}

// ************************************************************
// Start the OTA service
// ************************************************************
void WebManager_::startOTA() {
  AsyncElegantOTA.begin(&server, "admin", "update");
}

// ************************************************************
// Get singleton instance
// ************************************************************
WebManager_ &WebManager_::getInstance() {
  static WebManager_ instance;
  return instance;
}

WebManager_ &webManager = webManager.getInstance();