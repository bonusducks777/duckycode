#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "MagnetSensor-AP";
const char* password = "password123";

ESP8266WebServer server(80);

const int MAGNET_PIN = 15; // Changed from 0 (D3) to 15 (D8)
volatile bool magnetUp = false;
volatile unsigned long lastChangeTime = 0;

String lastEvent = "None";

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>Magnetic Field Sensor</title>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<style>body{font-family:Arial;}table{border-collapse:collapse;}th,td{padding:8px;border:1px solid #ccc;}</style>";
  html += "</head><body><h1>Magnetic Field Monitor</h1>";
  html += "<table><tr><th>Last Event</th><th>Current State</th></tr>";
  html += "<tr><td>" + lastEvent + "</td><td>" + (magnetUp ? "UP" : "DOWN") + "</td></tr></table>";
  html += "<p>AP IP: ";
  html += WiFi.softAPIP().toString(); // Fix: convert IPAddress to String
  html += "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(MAGNET_PIN, INPUT);

  WiFi.softAP(ssid, password);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Read analog value from the magnetic sensor
  int sensorValue = analogRead(MAGNET_PIN); // 0-1023 for 0-3.3V
  float voltage = sensorValue * (3.3 / 1023.0);

  // Decide pole based on 2.5V threshold
  bool newMagnetUp = (voltage > 2.5);
  if (newMagnetUp != magnetUp) {
    magnetUp = newMagnetUp;
    lastChangeTime = millis();
    lastEvent = magnetUp ? "UP" : "DOWN";
  }

  server.handleClient();
}
