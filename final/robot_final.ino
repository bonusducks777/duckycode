// Unified Robot Script for Group 32 Rover
// Combines drive, all sensors, and web server with integrated HTML UI
// Place this file as robot_final.ino in duckycode/final/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

// WiFi credentials
const char* ssid = "Group32-Rover";
const char* password = "Group32Password";

ESP8266WebServer server(80);

// Motor control pins
const int IN1 = 5;   // D1 (GPIO5)
const int IN2 = 4;   // D2 (GPIO4)
const int IN3 = 14;  // D5 (GPIO14)
const int IN4 = 12;  // D6 (GPIO12)

// Sensor pins
const int IR_PIN = 15;      // D8 (GPIO15)
const int RF_PIN = 15;      // D8 (GPIO15) (shared for demo)
const int MAGNET_PIN = 15;  // D8 (GPIO15) (shared for demo)
const int ULTRASONIC_RX = 15; // D8 (GPIO15) (shared for demo)

// Sensor state
volatile unsigned long irRisingEdgeCount = 0;
volatile float irLastFrequencyHz = 0;
volatile unsigned long rfRisingEdgeCount = 0;
volatile float rfLastFrequencyHz = 0;
bool magnetUp = false;
String duckName = "";

// --- Motor control ---
void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}
void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}
void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// --- Sensor ISRs (simplified for demo) ---
void IRAM_ATTR handleIrRisingEdge() {
  irRisingEdgeCount++;
  // Frequency calculation omitted for brevity
}
void IRAM_ATTR handleRfRisingEdge() {
  rfRisingEdgeCount++;
  // Frequency calculation omitted for brevity
}

// --- Web server handlers ---
void handleRoot() {
  // Serve the HTML UI
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "index.html not found");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}
void handleSensorData() {
  // Serve all sensor data as JSON
  String json = "{";
  json += "\"ir\":" + String(irLastFrequencyHz, 2) + ",";
  json += "\"rf\":" + String(rfLastFrequencyHz, 2) + ",";
  json += "\"magnet\":\"" + String(magnetUp ? "Up" : "Down") + "\",";
  json += "\"duckName\":\"" + duckName + "\"";
  json += "}";
  server.send(200, "application/json", json);
}
void handleDrive() {
  String cmd = server.arg("cmd");
  if (cmd == "forward") forward();
  else if (cmd == "backward") backward();
  else if (cmd == "left") left();
  else if (cmd == "right") right();
  else stopMotors();
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT); pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  stopMotors();

  pinMode(IR_PIN, INPUT);
  pinMode(RF_PIN, INPUT);
  pinMode(MAGNET_PIN, INPUT);
  pinMode(ULTRASONIC_RX, INPUT);

  // Attach interrupts for IR and RF (demo: both on D8)
  attachInterrupt(digitalPinToInterrupt(IR_PIN), handleIrRisingEdge, RISING);
  attachInterrupt(digitalPinToInterrupt(RF_PIN), handleRfRisingEdge, RISING);

  // Magnet sensor (demo: analogRead in loop)

  WiFi.softAP(ssid, password);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  server.on("/", handleRoot);
  server.on("/sensors", handleSensorData);
  server.on("/drive", handleDrive);
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/analysis.js", SPIFFS, "/analysis.js");
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Magnet sensor
  int sensorValue = analogRead(MAGNET_PIN);
  float voltage = sensorValue * (3.3 / 1023.0);
  magnetUp = (voltage > 2.5);

  // Duck name (demo: not implemented)
  // duckName = ...

  server.handleClient();
}
