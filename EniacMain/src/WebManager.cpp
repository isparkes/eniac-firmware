#include "WebManager.h"

// This include has to be here, anbd not in the header file
#include <ElegantOTA.h>

#include "LoopTasks.h"

// I had to fiddle in the ElegantOTA source to get this to work
// Line 27: #define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
// #define ELEGANTOTA_USE_ASYNC_WEBSERVER

// ************************************************************
// Wrap a handler so that it runs on the loop task.
//
// Handlers are called on the async_tcp task, but they read and
// write the config, SPIFFS, I2C and the OLED, which loop() uses
// too. The async_tcp task waits while loop() runs the handler,
// so nothing else happens on the connection in the meantime.
// ************************************************************
static ArRequestHandlerFunction inLoop(ArRequestHandlerFunction handler) {
  return [handler](AsyncWebServerRequest *request) {
    runInLoop([&]() { handler(request); });
  };
}

// ************************************************************
// Open up the normal page handlers
// ************************************************************
void WebManager_::begin() {
  debugMsgWbm("Setting up server endpoints");
  server.reset();
  server.serveStatic("/", SPIFFS, "/web/").setDefaultFile("index.html");

  // Summary and diagnostics
  server.on("/api/getSummary", HTTP_GET, inLoop(getSummaryDataHandler));
  server.on("/api/getDiags", HTTP_GET, inLoop(getDiagsDataHandler));
  server.on("/api/postDiags", HTTP_POST, inLoop(postDiagsDataHandler));
  
  // Configure time server
  server.on("/api/getTimeserver", HTTP_GET, inLoop(getTimeserverDataHandler));
  server.on("/api/postTimeserver", HTTP_POST, inLoop(postTimeserverDataHandler));
  server.on("/api/getZonesList", HTTP_GET, inLoop(getZonesListDataHandler));
  
  // Configure options
  server.on("/api/getConfig", HTTP_GET, inLoop(getConfigDataHandler));
  server.on("/api/postConfig", HTTP_POST, inLoop(postConfigDataHandler));

  // wifi credentials
  server.on("/api/postWiFiCredentials", HTTP_POST, inLoop(postWiFiCredentialsHandler));
  server.on("/api/credentials", HTTP_GET, inLoop(getCredentialsHandler));

  // Value
  server.on("/api/setValue", HTTP_GET, inLoop(postValueHandler));

  // Utilities
  server.on("/utils/resetwifi", HTTP_GET, inLoop(resetWifiHandler));
  server.on("/utils/scanI2C", HTTP_GET, inLoop(getI2CScanHandler));
  server.on("/utils/scanSPIFFS", HTTP_GET, inLoop(getSPIFFSScanHandler));
  server.on("/utils/saveStats", HTTP_GET, inLoop(saveStatsHandler));
  server.on("/utils/ntpupdate", HTTP_GET, inLoop([] (AsyncWebServerRequest *request) {
    ntpManager.resetNextUpdate();
        request->redirect("/utility.html");;
    }));
  server.on("/utils/resetoptions", HTTP_GET, inLoop([] (AsyncWebServerRequest *request) {
    resetOptions();
        request->redirect("/utility.html");;
    }));
  server.on("/utils/resetall", HTTP_GET, inLoop([] (AsyncWebServerRequest *request) {
    resetAll();
        request->redirect("/utility.html");;
    }));
  server.on("/utils/restart", HTTP_GET, inLoop(restartHandler));

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
    request->send(SPIFFS, "/web/portal.html", String(), false);
  }
};

// ************************************************************
// Open up the Portal Page
// ************************************************************
void WebManager_::beginPortal() {
  debugMsgWbm("Setting up server endpoints for Portal");
  server.reset();

  // serve the captive page
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);

  // wifi credentials
  server.on("/api/postWiFiCredentials", HTTP_POST, inLoop(postWiFiCredentialsHandler));
  server.on("/api/credentials", HTTP_GET, inLoop(getCredentialsHandler));
  server.on("/api/getWiFiNetworks", HTTP_GET, inLoop(getWiFiNetworksHandler));

  // Utilities
  server.on("/utils/resetwifi", HTTP_GET, inLoop(resetWifiHandler));
  server.on("/utils/scanI2C", HTTP_GET, inLoop(getI2CScanHandler));
  server.on("/utils/scanSPIFFS", HTTP_GET, inLoop(getSPIFFSScanHandler));
  server.on("/utils/saveStats", HTTP_GET, inLoop(saveStatsHandler));

  // All your DNS requests are belong to us
  wifiManager.startDNSD();

  debugMsgWbm("Start up web server");

  server.begin();
}

// ************************************************************
// Start the OTA service
// ************************************************************
void WebManager_::startOTA() {
  ElegantOTA.begin(&server, "admin", "update");
}

// ************************************************************
// Get singleton instance
// ************************************************************
WebManager_ &WebManager_::getInstance() {
  static WebManager_ instance;
  return instance;
}

WebManager_ &webManager = webManager.getInstance();