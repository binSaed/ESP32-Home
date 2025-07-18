#include "HomeSpan.h"
#include "LED.h"
#include "Insights.h"
#include <WiFi.h>
#include <NetworkClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>

WebServer server(80);

void handleRoot() {
  String html = "<!DOCTYPE html><html>"
                "<head><meta name='viewport' content='width=device-width, initial-scale=1'>"
                "<style>body{font-family:Arial;text-align:center;margin:50px;background:#f0f0f0;}"
                ".container{background:white;padding:30px;border-radius:15px;box-shadow:0 4px 6px rgba(0,0,0,0.1);max-width:400px;margin:0 auto;}"
                ".status{font-size:24px;font-weight:bold;margin:20px 0;}"
                ".status.on{color:#4CAF50;} .status.off{color:#f44336;}"
                ".switch{position:relative;display:inline-block;width:80px;height:40px;margin:20px;}"
                ".switch input{opacity:0;width:0;height:0;}"
                ".slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;transition:.4s;border-radius:40px;}"
                ".slider:before{position:absolute;content:'';height:32px;width:32px;left:4px;bottom:4px;background-color:white;transition:.4s;border-radius:50%;}"
                "input:checked + .slider{background-color:#4CAF50;}"
                "input:checked + .slider:before{transform:translateX(40px);}"
                ".switch-label{font-size:18px;margin:10px 0;color:#333;}"
                "h1{color:#333;margin-bottom:30px;}</style>"
                "<script>"
                "let currentLedState = false;"
                "function updateStatus(){"
                "fetch('/api/led/status').then(response=>response.json()).then(data=>{"
                "const statusElement=document.getElementById('ledStatus');"
                "const switchElement=document.getElementById('ledSwitch');"
                "const isOn=data.led;"
                "currentLedState=isOn;"
                "statusElement.textContent=isOn?'ON':'OFF';"
                "statusElement.className=isOn?'status on':'status off';"
                "switchElement.checked=isOn;"
                "}).catch(err=>console.error('Error:',err));"
                "}"
                "function toggleLED(){"
                "fetch('/api/led/toggle').then(response=>response.json()).then(data=>{"
                "if(data.status==='success'){updateStatus();}else{alert('Error: '+data.message);}"
                "}).catch(err=>{console.error('Error:',err);alert('Connection error');});"
                "}"
                "setInterval(updateStatus,1000);"
                "window.onload=updateStatus;"
                "</script></head>"
                "<body>"
                "<div class='container'>"
                "<div class='switch-label'>LED Status: <span id='ledStatus' class='status'>Loading...</span></div>"
                "<label class='switch'>"
                "<input type='checkbox' id='ledSwitch' onchange='toggleLED()'>"
                "<span class='slider'></span>"
                "</label>"
                "</div>"
                "</body></html>";
  server.send(200, "text/html", html);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
const char insights_auth_key[] = "eyJhbGciOiJSUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VyIjoiOGYwOTU1ZWEtOTdmYS00ZjZkLWE3ODAtODE0NzZiNGUxMTJiIiwiaXNzIjoiZTMyMmI1OWMtNjNjYy00ZTQwLThlYTItNGU3NzY2NTQ1Y2NhIiwic3ViIjoiM2MxOTA1MDUtZTIwZS00NDI4LWIwODktOTgzOWE2NDY2YTBlIiwiZXhwIjoyMDY4MTY3MzYyLCJpYXQiOjE3NTI4MDczNjJ9.aztwsnE2te9AkilElUOwHQm965m3TUCXIVZUfUZE7h_2XISDcSkLDeBki-XvbRxGkHrCfli2t67qSHQDGAIl61asLkTWVPzBthHacw1UkMtvxX4HW284Jv5cX9nAnWX0iHuzvkaJwkuoQXBHEfHFLs74EjlCynW0IHtM1Lis5juii2GqrsfHgfOJQg41QZugcW61H-s31rqdUFVDb7Lj4QUaGcopuLJRoX-jh3mpysQapVsPQe3Zgf2iso0gdTELs-K9ZEcX7LXMNNdE9r2JOtbnWlgAWKwGozGTEalxD5w-pjjXH_RIh_aQ_kDg87WvMjl6U3i-5HKM3ac8APq73w";

#define WIFI_SSID "Abdelrahman"
#define WIFI_PASSPHRASE "Abdo546453@#@"

// Global reference to LED object for web control
LED *ledDevice = nullptr;

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);

  while (WiFi.status() != WL_CONNECTED) {
    delay(600);
    Serial.print(".");
  }

  Serial.println("");
  Serial.printf("WiFi connected %s\n", WIFI_SSID);

  // Initialize Insights after WiFi connection
  if (!Insights.begin(insights_auth_key)) {
    Serial.println("Failed to initialize Insights!");
    return;
  }
  Serial.println("Insights initialized successfully!");

  homeSpan.setWifiCredentials(WIFI_SSID, WIFI_PASSPHRASE);

  // Enable persistent storage for pairing data
  homeSpan.enableAutoStartAP();
  homeSpan.setApSSID("Esp-Home-Setup");
  homeSpan.setApPassword("12345678");

  homeSpan.setPairingCode("11122333");
  homeSpan.setQRID("111-22-333");

  // Set device information for proper identification
  homeSpan.setHostNameSuffix("-ESP32");
  homeSpan.setPortNum(1201);

  homeSpan.begin(Category::Bridges, "HomeSpan Bridge");

  new SpanAccessory();
  new Service::AccessoryInformation();
  new Characteristic::Name("Built-in LED");
  new Characteristic::Manufacturer("HomeSpan");
  new Characteristic::SerialNumber("LED-001");
  new Characteristic::Model("ESP32-LED");
  new Characteristic::FirmwareRevision("1.0.0");
  new Characteristic::Identify();
  ledDevice = new LED(2);  // Store reference for web control

  if (MDNS.begin("esp320")) {
    Serial.println("MDNS responder started");
    // Add HTTP service to mDNS-SD
    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "path", "/");
    Serial.println("HTTP service added to mDNS");
  } else {
    Serial.println("Error starting MDNS responder!");
  }

  server.on("/", handleRoot);


  server.on("/led/toggle", []() {
    if (ledDevice) {
      boolean currentState = ledDevice->getLED();
      ledDevice->setLED(!currentState);
      Serial.println("LED toggled via web - HomeKit updated");
    }
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.on("/api/led/status", []() {
    String json = "{\"led\":" + String(ledDevice->getLED() ? "true" : "false") + "}";
    server.send(200, "application/json", json);
  });

  server.on("/api/led/toggle", []() {
    if (ledDevice) {
      boolean currentState = ledDevice->getLED();
      ledDevice->setLED(!currentState);
      String newState = !currentState ? "ON" : "OFF";
      server.send(200, "application/json", "{\"status\":\"success\",\"message\":\"LED " + newState + "\",\"homekit_updated\":true}");
    } else {
      server.send(500, "application/json", "{\"error\":\"LED device not initialized\"}");
    }
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("Open http://");
  Serial.print(WiFi.localIP());
  Serial.println("/ to control LED");
  Serial.println("Or try: http://esp32.local/");
}

void loop() {
  homeSpan.poll();
  server.handleClient();
}
