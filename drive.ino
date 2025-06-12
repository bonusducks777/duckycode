#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// WiFi credentials
const char* ssid = "ducky repeater";
const char* password = "chigga123";

// Motor control pins for ESP8266 (D1, D2, D5, D6)
const int IN1 = 5;   // D1 (GPIO5)
const int IN2 = 4;   // D2 (GPIO4)
const int IN3 = 14;  // D5 (GPIO14)
const int IN4 = 12;  // D6 (GPIO12)

// LED pin for WiFi status (built-in LED is usually GPIO2 or GPIO16)
const int LED_PIN = 2;  // D4 (GPIO2), adjust if needed

ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  stopMotors();
  digitalWrite(LED_PIN, HIGH);

  WiFi.softAP(ssid, password);
  delay(100);
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  digitalWrite(LED_PIN, HIGH);

  server.on("/", handleRoot);
  server.on("/forward", handleForward);
  server.on("/backward", handleBackward);
  server.on("/left", handleLeft);
  server.on("/right", handleRight);
  server.on("/stop", handleStop);
  server.on("/fwdleft", handleFwdLeft);
  server.on("/fwdright", handleFwdRight);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  delay(1);
}

void handleRoot() {
  String html = "<html><head>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>ESP8266 Robot Control</title>";
  html += "<style>";
  html += "body{font-family:Arial,sans-serif;text-align:center;margin:0;padding:20px}";
  html += "button{width:80px;height:80px;margin:10px;font-size:20px;border-radius:10px;border:none;background:#4CAF50;color:white;cursor:pointer}";
  html += "button:hover{background:#45a049}";
  html += "button:active{background:#3e8e41}";
  html += ".controls{display:inline-block;margin:20px}";
  html += ".row{display:flex;justify-content:center}";
  html += "#stop{background:#f44336}";
  html += "#stop:hover{background:#d32f2f}";
  html += "</style>";
  html += "</head><body>";
  html += "<h1>ESP8266 Robot Control</h1>";
  html += "<div class='controls'>";
  html += "<div class='row'><button onclick='sendCommand(\"/forward\")'>W</button></div>";
  html += "<div class='row'>";
  html += "<button onclick='sendCommand(\"/left\")'>A</button>";
  html += "<button id='stop' onclick='sendCommand(\"/stop\")'>STOP</button>";
  html += "<button onclick='sendCommand(\"/right\")'>D</button>";
  html += "</div>";
  html += "<div class='row'><button onclick='sendCommand(\"/backward\")'>S</button></div>";
  html += "<div class='row'><button onclick='sendCommand(\"/fwdleft\")'>&#8598;</button><button onclick='sendCommand(\"/forward\")'>W</button><button onclick='sendCommand(\"/fwdright\")'>&#8599;</button></div>";
  html += "</div>";
  html += "<p>Control with WASD keys or buttons</p>";  html += "<script>";
  html += "let keys = {};";
  html += "let lastCmd = '';";
  html += "function sendCommand(cmd) {";
  html += "  if (cmd !== lastCmd) { fetch(cmd).then(() => lastCmd = cmd); }";
  html += "}";
  html += "function updateCommand() {";
  html += "  if (keys['w'] && keys['q']) sendCommand('/fwdleft');";
  html += "  else if (keys['w'] && keys['e']) sendCommand('/fwdright');";
  html += "  else if (keys['w']) sendCommand('/forward');";
  html += "  else if (keys['s']) sendCommand('/backward');";
  html += "  else if (keys['a']) sendCommand('/left');";
  html += "  else if (keys['d']) sendCommand('/right');";
  html += "  else if (keys[' ']) sendCommand('/stop');";
  html += "  else sendCommand('/stop');";
  html += "}";
  html += "document.addEventListener('keydown', function(event) {";
  html += "  if (!keys[event.key.toLowerCase()]) {";
  html += "    keys[event.key.toLowerCase()] = true;";
  html += "    updateCommand();";
  html += "  }";
  html += "});";
  html += "document.addEventListener('keyup', function(event) {";
  html += "  if (keys[event.key.toLowerCase()]) {";
  html += "    keys[event.key.toLowerCase()] = false;";
  html += "    updateCommand();";
  html += "  }";
  html += "});";
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleForward() { moveForward(); server.send(200, "text/plain", "Moving forward"); }
void handleBackward() { moveBackward(); server.send(200, "text/plain", "Moving backward"); }
void handleLeft() {
  Serial.println("Command: Left");
  turnLeft();
  server.send(200, "text/plain", "Turning left");
}
void handleRight() {
  Serial.println("Command: Right");
  turnRight();
  server.send(200, "text/plain", "Turning right");
}
void handleStop() { stopMotors(); server.send(200, "text/plain", "Motors stopped"); }
void handleFwdLeft() { moveFwdLeft(); server.send(200, "text/plain", "Forward left"); }
void handleFwdRight() { moveFwdRight(); server.send(200, "text/plain", "Forward right"); }

void moveForward() { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void moveBackward() { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); }
void turnLeft() { digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void turnRight() { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH); }
void moveFwdLeft() { digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW); }
void moveFwdRight() { digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); }
void stopMotors() { digitalWrite(IN1, LOW); digitalWrite(IN2, LOW); digitalWrite(IN3, LOW); digitalWrite(IN4, LOW); }