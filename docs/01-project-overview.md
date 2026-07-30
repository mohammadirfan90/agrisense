# 1. Project Overview

## 1.1 Project Summary
**AgriSense** is a smart, localized agricultural monitoring and irrigation control system built on the ESP32 microcontroller. The system automates plant watering based on environmental inputs (soil moisture, temperature, humidity, ambient light, and rain detection). It hosts its own local web server directly on the ESP32, exposing a REST API and serving a responsive vanilla web dashboard. This allows real-time monitoring and manual override of irrigation controls by any local device connected to the same Wi-Fi network without requiring external cloud databases or internet access.

## 1.2 Objectives
*   **Autonomous Monitoring**: Sample soil moisture, ambient light, rainfall, temperature, and humidity at regular non-blocking intervals.
*   **Smart Irrigation Control**: Implement a closed-loop decision engine that activates a water pump when soil moisture falls below threshold levels, unless rainfall is active.
*   **Local Web Hosting**: Serve a responsive, low-latency web dashboard directly from the ESP32 flash memory using SPIFFS/LittleFS.
*   **REST API Communication**: Establish JSON-based HTTP REST endpoints for state tracking and remote pump activation.
*   **Local Visualization**: Provide immediate feedback of system health, IP address, and sensor readings on a physical SSD1306 OLED display.
*   **Fail-Safe Architecture**: Guard against mechanical/software failures with safety lockouts (pump run timeouts, rain lockouts, sensor validation).

## 1.3 Scope
*   **In Scope**:
    *   ESP32 DevKit V1 firmware architecture utilizing non-blocking `millis()` loop logic.
    *   I2C multi-device bus integration (SSD1306 OLED + BH1750 Light Sensor).
    *   Analog sensor calibration (Soil moisture, Rain sensor).
    *   Single-channel low-side relay switching circuit with inductive noise suppression.
    *   Local Wi-Fi Access Point (AP) and Station (STA) network configurations.
    *   SPIFFS/LittleFS-hosted web dashboard constructed with semantic HTML5, vanilla CSS, and vanilla JS.
    *   JSON API endpoints for dashboard data exchange.
*   **Out of Scope**:
    *   External cloud database integration (e.g., AWS IoT, Firebase).
    *   Public internet routing (WAN access) and external DNS management.
    *   Mobile applications (Android/iOS native wrapper apps).
    *   Multi-zone irrigation routing (system supports one pump zone).

## 1.4 Expected Outcomes
*   A fully self-contained smart irrigation hardware prototype with a neat, low-noise power layout.
*   A responsive dashboard interface that operates smoothly on mobile, tablet, and desktop viewports.
*   A modular, documented codebase that can be compiled in the Arduino IDE.
*   Complete engineering blueprints detailing pin routing, electrical isolation, and API specs.

---

# 2. Functional Requirements

## 2.1 Environmental Monitoring
*   **FR-1.1**: The system must sample soil moisture levels at a minimum interval of 1 second.
*   **FR-1.2**: The system must measure temperature and humidity using the DHT22 sensor every 2 seconds.
*   **FR-1.3**: The system must measure light intensity in Lux via the BH1750 sensor every 1 second.
*   **FR-1.4**: The system must detect rainfall status (dry, light rain, heavy rain) every 500 milliseconds.

## 2.2 Irrigation Control
*   **FR-2.1**: The system must run in either **AUTO** or **MANUAL** mode.
*   **FR-2.2 (AUTO)**: The system must activate the water pump via the relay when soil moisture drops below the configurable threshold value (default: 30%) and no rain is detected.
*   **FR-2.3 (AUTO)**: The system must deactivate the pump when soil moisture reaches the target wet threshold (default: 70%).
*   **FR-2.4 (AUTO)**: The system must lock out pump execution if rain is detected, regardless of soil moisture levels.
*   **FR-2.5 (MANUAL)**: The user must be able to toggle the pump ON and OFF remotely via the dashboard.
*   **FR-2.6 (MANUAL)**: Manual activation must override automatic thresholds but remain constrained by safety watchdog timers.

## 2.3 Local User Interface
*   **FR-3.1**: The OLED display must show the current Wi-Fi SSID, local IP address, sensor status, active alarms, and pump state.
*   **FR-3.2**: The display must refresh at least once every 1 second without halting sensor reads.
*   **FR-3.3**: The active buzzer must sound an audible alarm if a hardware fault is detected (e.g., sensor disconnected, pump timeout).

## 2.4 Web Server & Dashboard
*   **FR-4.1**: The ESP32 must run an HTTP server listening on port 80.
*   **FR-4.2**: The server must serve static UI files (`index.html`, `style.css`, `script.js`) from local flash storage (LittleFS).
*   **FR-4.3**: The server must provide JSON endpoints for reading sensor values and setting operational states.
*   **FR-4.4**: The web dashboard must show live-updated sensor values, system mode (AUTO/MANUAL), pump status, and current warning logs.
*   **FR-4.5**: The dashboard must fit desktop, tablet, and mobile screens.

---

# 3. Non-Functional Requirements

## 3.1 Performance
*   **NFR-1.1**: HTTP REST API response latency must remain under 200 milliseconds under typical single-client conditions.
*   **NFR-1.2**: The core firmware loop must execute in under 10 milliseconds to avoid blocking network stacks or watchdog resets.

## 3.2 Reliability
*   **NFR-2.1**: The system must implement an automatic ESP32 Hardware Watchdog Timer (WDT) to recover from firmware lockups.
*   **NFR-2.2**: The Wi-Fi manager must auto-detect connection drops and reconnect within 10 seconds, maintaining local offline irrigation routines in the interim.
*   **NFR-2.3**: If the pump runs continuously for more than a configured maximum duration (default: 30 seconds) without soil moisture increasing, it must shut down and enter an ERROR state to prevent flooding or motor burnout.

## 3.3 Maintainability
*   **NFR-3.1**: Firmware modules must be logically isolated into distinct driver and logic files (`sensor_manager`, `pump_controller`, etc.) with strict header declarations.
*   **NFR-3.2**: Code must compile without custom patches to standard Arduino libraries.

## 3.4 Power Efficiency
*   **NFR-4.1**: The system must run on battery power. The design must minimize continuous current draw; sensors should be powered down or placed in low-power modes when not sampling if possible.
*   **NFR-4.2**: The LM2596 buck converter must operate at an efficiency level above 80% to conserve battery reserve.

## 3.5 Scalability
*   **NFR-5.1**: The I2C bus wiring layout must support additional I2C sensors (e.g., barometric pressure, UV sensor) without requiring GPIO modifications.
*   **NFR-5.2**: The software configuration header must support adding additional threshold parameters easily.

## 3.6 Security
*   **NFR-6.1**: The Wi-Fi Access Point mode must require WPA2-PSK encryption with a strong preset password.
*   **NFR-6.2**: REST API routes modifying system state (e.g. pump toggles) must validate control token arguments to block basic cross-site request hijacking.
