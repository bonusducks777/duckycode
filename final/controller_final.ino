// Unified Controller Script for Group 32 Rover
// Sends drive commands to the robot server
// Place this file as controller_final.ino in duckycode/final/

#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "Group32-Rover";
const char *password = "Group32Password";

const char *robot_ip = "192.168.4.1"; // Default AP IP for ESP8266/ESP32 softAP

// Pin definitions (customize as needed)
#define UP_PIN 18
#define RIGHT_PIN 21
#define DOWN_PIN 23
#define LEFT_PIN 19
#define STOP_PIN 22

void setup() {
  Serial.begin(115200);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(STOP_PIN, INPUT_PULLUP);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void loop() {
  if (digitalRead(UP_PIN) == LOW) {
    sendDriveCommand("forward");
    delay(300);
  } else if (digitalRead(DOWN_PIN) == LOW) {
    sendDriveCommand("backward");
    delay(300);
  } else if (digitalRead(LEFT_PIN) == LOW) {
    sendDriveCommand("left");
    delay(300);
  } else if (digitalRead(RIGHT_PIN) == LOW) {
    sendDriveCommand("right");
    delay(300);
  } else if (digitalRead(STOP_PIN) == LOW) {
    sendDriveCommand("stop");
    delay(300);
  }
  delay(50);
}

void sendDriveCommand(const char* cmd) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String("http://") + robot_ip + "/drive?cmd=" + cmd;
    http.begin(url);
    int httpCode = http.GET();
    http.end();
    Serial.print("Sent command: ");
    Serial.println(cmd);
  }
}
