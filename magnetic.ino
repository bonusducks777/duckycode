#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "MagnetSensor-AP";
const char* password = "password123";

ESP8266WebServer server(80);

const int MAGNET_PIN = 2; // D4 on NodeMCU/ESP8266 (GPIO2)
volatile bool magnetUp = false;
volatile bool lastMagnetState = false;
volatile unsigned long lastChangeTime = 0;

String lastEvent = "None";

void IRAM_ATTR handleMagnetChange() {
  bool state = digitalRead(MAGNET_PIN);
  if (state != lastMagnetState) {
    lastMagnetState = state;
    lastChangeTime = millis();
    magnetUp = state;
    if (magnetUp) {
      lastEvent = "UP";
    } else {
      lastEvent = "DOWN";
    }
  }
}

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>Magnetic Field Sensor</title>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<style>body{font-family:Arial;}table{border-collapse:collapse;}th,td{padding:8px;border:1px solid #ccc;}</style>";
  html += "</head><body><h1>Magnetic Field Monitor</h1>";
  html += "<table><tr><th>Last Event</th><th>Current State</th></tr>";
  html += "<tr><td>" + lastEvent + "</td><td>" + (magnetUp ? "UP" : "DOWN") + "</td></tr></table>";
  html += "<p>AP IP: ";
  html += WiFi.softAPIP();
  html += "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(MAGNET_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MAGNET_PIN), handleMagnetChange, CHANGE);

  WiFi.softAP(ssid, password);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}
