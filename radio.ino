#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "ESP8266-RadioCounter";
const char* password = "password123";

ESP8266WebServer server(80);

const int SIGNAL_PIN = 15; // D8 on NodeMCU/ESP8266 (GPIO15)
volatile unsigned long risingEdgeCount = 0;
volatile unsigned long lastEdgeMicros = 0;
volatile float lastFrequencyHz = 0;

const unsigned long MIN_DEBOUNCE_MICROS = 350; // >300us noise, <2ms (500Hz)

void IRAM_ATTR handleRisingEdge() {
  unsigned long nowMicros = micros();
  if (nowMicros - lastEdgeMicros >= MIN_DEBOUNCE_MICROS) {
    if (lastEdgeMicros != 0) {
      unsigned long periodMicros = nowMicros - lastEdgeMicros;
      if (periodMicros > 0) {
        lastFrequencyHz = 1e6 / (float)periodMicros;
      }
    }
    lastEdgeMicros = nowMicros;
    risingEdgeCount++;
  }
}

void handleRoot() {
  String html = "<!DOCTYPE HTML><html><head>";
  html += "<title>ESP8266 Radio Frequency Counter</title>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<style>body{font-family:Arial;}table{border-collapse:collapse;}th,td{padding:8px;border:1px solid #ccc;}</style>";
  html += "</head><body><h1>ESP8266 Radio Frequency Monitor</h1><table>";
  html += "<tr><th>Metric</th><th>Value</th></tr>";
  html += "<tr><td>Rising Edges / sec</td><td>" + String(risingEdgeCount) + "</td></tr>";
  html += "<tr><td>Instantaneous Frequency</td><td>";
  if (risingEdgeCount == 0) html += "0 Hz (no new edge)";
  else html += String(lastFrequencyHz, 2) + " Hz";
  html += "</td></tr></table>";
  html += "<p>IP: ";
  html += WiFi.softAPIP().toString(); // Fix: convert IPAddress to String
  html += "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  pinMode(SIGNAL_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SIGNAL_PIN), handleRisingEdge, RISING);

  Serial.println();
  Serial.println("ESP8266 Radio Frequency Counter Starting");
  WiFi.softAP(ssid, password);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  static unsigned long lastPrintTime = 0;
  static unsigned long lastEdgeSnapshot = 0;
  unsigned long now = millis();

  if (now - lastPrintTime >= 1000) {
    unsigned long count_snapshot;
    float freq_snapshot;
    unsigned long edgeTime_snapshot;

    noInterrupts();
    count_snapshot = risingEdgeCount;
    freq_snapshot = lastFrequencyHz;
    edgeTime_snapshot = lastEdgeMicros;
    risingEdgeCount = 0;
    interrupts();

    Serial.print("Rising edges/sec: ");
    Serial.print(count_snapshot);
    Serial.print(" | Instantaneous frequency: ");
    if (edgeTime_snapshot == lastEdgeSnapshot || count_snapshot == 0) {
      Serial.println("0 Hz (no new edge)");
    } else {
      Serial.print(freq_snapshot, 2);
      Serial.println(" Hz");
    }
    lastEdgeSnapshot = edgeTime_snapshot;
    lastPrintTime = now;
  }
  server.handleClient();
}
