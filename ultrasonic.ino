#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// RX from duck sensor, TX unused
const uint8_t DUCK_RX_PIN = 2;
const uint8_t DUCK_TX_UNUSED = 3;

SoftwareSerial duckSerial(DUCK_RX_PIN, DUCK_TX_UNUSED);

// State machine for reading duck names
String duckName = "";
bool receivingName = false;
String tempName = "";
unsigned long lastCharTime = 0;
const unsigned long nameTimeout = 100;  // ms
const int NAME_LENGTH = 4;

// WiFi and Web server setup
const char* ssid = "DuckSensor-AP";
const char* password = "password123";
ESP8266WebServer server(80);
String lastReportedDuck = "";

void setup() {
  Serial.begin(115200);          // USB serial monitor
  duckSerial.begin(600);         // Duck sensor: 600 baud

  // WiFi AP setup
  WiFi.softAP(ssid, password);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  readDuckName();  // Non-blocking sensor read

  // Update last reported duck for web page
  if (duckName != "") {
    Serial.println("Duck detected: " + duckName);
    lastReportedDuck = duckName;
    duckName = ""; // Clear after handling
  }

  server.handleClient();
}

void readDuckName() {
  if (duckSerial.available()) {
    char c = duckSerial.read();
    lastCharTime = millis();

    if (c == '#') {
      receivingName = true;
      tempName = "#";
    } else if (receivingName) {
      tempName += c;

      if (tempName.length() >= NAME_LENGTH) {
        duckName = tempName;
        receivingName = false;
        tempName = "";
      }
    }
  }

  // Timeout: reset if transmission stalls
  if (receivingName && (millis() - lastCharTime > nameTimeout)) {
    receivingName = false;
    tempName = "";
  }
}

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>Duck Sensor Web</title>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<style>body{font-family:Arial;}table{border-collapse:collapse;}th,td{padding:8px;border:1px solid #ccc;}</style>";
  html += "</head><body><h1>Duck Sensor Monitor</h1>";
  html += "<table><tr><th>Last Detected Duck</th></tr>";
  html += "<tr><td>" + (lastReportedDuck != "" ? lastReportedDuck : "None") + "</td></tr></table>";
  html += "<p>AP IP: ";
  html += WiFi.softAPIP();
  html += "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}