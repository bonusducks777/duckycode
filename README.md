# EEESeaBoat Duck Survey Scripts

This folder contains four ESP8266-based scripts for the ELEC40006 Electronics Design Project. Each script is designed to detect a specific duck characteristic (name or species) and report it via a Wi-Fi hotspot and web server. All scripts provide a simple web interface for real-time monitoring during the EEESeaBoat demo.

## Overview

| Script           | Sensor Type         | What it Detects         | Pin   | Web Output                |
| ---------------- | ------------------ | ----------------------- | ----- | ------------------------ |
| `ultrasonic.ino` | Ultrasonic UART    | Duck name (ASCII)       | D8    | Last detected name       |
| `ir.ino`         | Infrared (digital) | IR frequency (Hz)       | D8    | Frequency metrics        |
| `radio.ino`      | Radio (digital)    | Radio frequency (Hz)    | D8    | Frequency metrics        |
| `magnetic.ino`   | Magnetic switch    | Magnet up/down state    | D8    | Last event & state       |
| `drive.ino`      | Wi-Fi Rover Control| Rover drive commands     | D1/D2/D5/D6 | Drive status & control  |

---

## 1. `ultrasonic.ino` — Duck Name Reader

**Purpose & Approach:**
- Ducks transmit their names using ultrasonic (40kHz) ASK, encoded as UART (600 baud, 8N1, ASCII, starts with `#`).
- The ESP8266 reads this via a demodulator circuit that outputs UART logic levels.
- We use `SoftwareSerial` to receive the data on a digital pin, so the main UART is free for debugging.

**How it works:**
- The code implements a state machine to assemble a 4-character name, starting with `#`.
- It uses a timeout to reset if the transmission stalls, ensuring robustness against noise or partial packets.
- The last detected name is shown on a web page, which auto-refreshes every 2 seconds.

**Key Function:**
```cpp
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
```
- **Why this way?**
  - Using a state machine and timeout ensures we only report valid, complete names and ignore noise or partial data.
  - `SoftwareSerial` allows us to use any digital pin for UART input, freeing up the main serial port for debugging.

---

## 2. `ir.ino` — Infrared Frequency Counter

**Purpose & Approach:**
- Ducks emit IR pulses at a species-specific frequency (135–1000 Hz, 50μs pulse width typical).
- We want to measure this frequency accurately, even with short pulses and possible noise.

**How it works:**
- An interrupt is attached to the IR sensor output pin, triggering on every rising edge.
- The ISR (interrupt service routine) debounces the signal (ignores pulses closer than 350μs) to filter out noise.
- The code counts rising edges per second (average frequency) and also calculates the instantaneous frequency from the last two edges.
- Results are shown on a web page, auto-refreshing every 2 seconds.

**Key Function:**
```cpp
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
```
- **Why this way?**
  - Using interrupts ensures we never miss a pulse, even if the main loop is busy.
  - Debouncing in the ISR is critical for rejecting noise and false triggers, especially with fast, narrow pulses.
  - Both average and instantaneous frequency are reported for robust identification.

**Design Note:**
- Avoid excessive low-pass filtering in hardware, as it can round off the 50μs pulses and make them undetectable. Use a fast comparator or Schmitt trigger if possible.

---

## 3. `radio.ino` — Radio Frequency Counter

**Purpose & Approach:**
- Some ducks emit a radio-frequency digital signal (e.g., 100 Hz, 150 Hz) for species identification.
- The logic is identical to `ir.ino`, but the input is from a radio sensor.

**How it works:**
- Uses the same interrupt/debounce/frequency calculation as `ir.ino`.
- The web page output is the same, making it easy to compare IR and radio results.

**Key Function:**
```cpp
// ...same as IRAM_ATTR handleRisingEdge() above...
```
- **Why this way?**
  - Code reuse and consistency: the same robust logic works for both IR and radio digital signals.

---

## 4. `magnetic.ino` — Magnetic State Detector

**Purpose & Approach:**
- Some ducks are identified by the state of a magnetic field sensor (e.g., reed switch or Hall effect sensor).
- We want to detect and report both the current state (UP/DOWN) and the last event.

**How it works:**
- An interrupt is attached to the sensor pin, triggering on any change (rising or falling edge).
- The ISR updates the state and logs the event (UP or DOWN) with a timestamp.
- The web page shows both the last event and the current state, auto-refreshing every 2 seconds.

**Key Function:**
```cpp
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
```
- **Why this way?**
  - Using an interrupt ensures immediate response to changes, even if the main loop is busy.
  - Tracking both the last event and current state helps with debugging and ensures the web page always shows meaningful information.

---

## 5. `drive.ino` — ESP8266 Wi-Fi Rover Drivebase Control

**Purpose:**
Provides a web-based interface for real-time control of the rover's drive motors via Wi-Fi (ESP8266 in AP mode). Supports forward, backward, left, right, forward-left, and forward-right maneuvers, with continuous movement as long as a key is held.

**Key Features:**
- Runs a web server at http://192.168.4.1 for direct control (no external Wi-Fi needed).
- Responsive WASD and arrow-style UI, plus keyboard shortcuts (W/A/S/D/Q/E/Space).
- Supports combined keypresses (e.g., W+Q for forward-left) for smooth steering.
- Motors run continuously in the last commanded direction until a new command or stop is sent.
- Uses only safe ESP8266 output pins (D1/D2/D5/D6) for motor driver compatibility.
- Built-in LED indicates Wi-Fi status.

**How it works:**
- Each movement command (forward, left, etc.) keeps the motors running until a new command or stop is sent.
- The web UI tracks keydown/keyup events and sends commands only when the set of pressed keys changes, ensuring smooth, uninterrupted driving.
- When all movement keys are released, the robot stops.

**Wiring Notes:**
- IN1 = D1 (GPIO5), IN2 = D2 (GPIO4), IN3 = D5 (GPIO14), IN4 = D6 (GPIO12).
- Built-in LED is on D4 (GPIO2) by default.
- Avoid using ESP8266 boot-sensitive pins for motor control.

**Usage:**
1. Flash to ESP8266 and power up.
2. Connect to Wi-Fi SSID `ducky repeater` (password: `ducks123`).
3. Open http://192.168.4.1 in a browser.
4. Use the on-screen buttons or WASD/Q/E/Space keys for control.

**Integration:**
- Designed to be modular and ready for future integration with sensor scripts and unified rover control.

---

## How to Use Each Script

1. **Upload the script to your ESP8266 (NodeMCU, Wemos D1 Mini, etc.).**
2. **Connect the appropriate sensor to D8 (GPIO15) and GND.**
3. **Power the ESP8266 and wait for it to start.**
4. **Connect your phone or laptop to the Wi-Fi hotspot created by the ESP8266 (see SSID in each script).**
5. **Open a browser and go to `http://192.168.4.1` to view the live sensor data.**

---

## Integration Note

These scripts are modular building blocks for the EEESeaBoat rover. In the final system, their logic will be integrated into a comprehensive control program that:
- Combines all sensor processing (ultrasonic, infrared, radio, magnetic) into a single codebase.
- Handles controller and drivebase logic for rover movement and remote operation.
- Hosts a unified web server that not only displays live sensor data but also saves and stores results for later analysis.
- Presents all information in a user-friendly interface for efficient field operation and demonstration.

This modular approach allows for easier testing and debugging of each sensor subsystem before full integration. The scripts here represent an essential step in the development process toward a robust, all-in-one EEESeaBoat control and survey platform.

---

## Design Rationale

- ESP8266 modules allow each sensor to be independently monitored and debugged via Wi-Fi, without a complex wired interface or display.
- The web server approach is platform-agnostic: any phone, tablet, or laptop can view the results in real time.
- Interrupt-driven logic ensures reliable detection of fast or brief events, even if the main loop is busy with other tasks.
- The code is modular and easy to adapt for future sensors or changes in the duck communication protocol.
