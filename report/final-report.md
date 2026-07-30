# 1. Cover Page

```text
================================================================================
                                 PROJECT REPORT
                                       ON
                 AgriSense: Smart Irrigation System Using ESP32
================================================================================

                                  COURSE NAME
                        IoT & Embedded Systems Design Lab

                                 SUBMITTED BY:
                                 Mohammad Irfan

                                  STUDENT IDS:
                                  2026-CSE-041

                                 SUBMITTED TO:
                       Department Evaluation Committee CS

                                  DEPARTMENT:
                   Department of Computer Science and Engineering

                                  UNIVERSITY:
                        Autonomous Engineering Institute

                               SUBMISSION DATE:
                                July 30, 2026
```

---

# 2. Introduction

Traditional farming relies heavily on manual watering, which often results in water waste or plant dehydration. Overwatering washes away nutrients and damages roots, while underwatering dries out plants and stops growth. Because farming uses a large portion of freshwater resources globally, creating an automated system to manage watering based on actual plant needs is essential for saving water.

AgriSense is a low-cost, portable smart irrigation prototype built on the ESP32 microcontroller. The system gathers environmental data such as soil moisture, temperature, humidity, light, and rain. It automatically controls a water pump relay based on soil moisture levels and hosts a simple, local web dashboard over Wi-Fi, allowing users to monitor sensor readings and override controls manually from mobile devices.

---

# 3. Project Overview

### 3.1 Objectives
*   **Environmental Monitoring**: Automatically measure soil moisture, temperature, humidity, light, and rain status at regular, non-blocking intervals.
*   **Smart Irrigation Control**: Automatically switch a water pump relay on or off based on soil moisture level thresholds.
*   **Precipitation Prevention**: Stop irrigation automatically if rain is detected, preventing water waste and plant over-saturation.
*   **Web Dashboard Interface**: Serve a local web page directly from the ESP32 for wireless monitoring and manual pump controls.
*   **System Alert & Safety**: Present status logs on a physical OLED screen during boot and safety lockouts.

### 3.2 Key Features
*   **Dual Operation Modes**: Supports threshold-based automatic control (AUTO) and manual web page overrides (MANUAL).
*   **Wi-Fi Hotspot Fallback**: Connects to home Wi-Fi or automatically broadcasts its own hotspot (`AgriSense_AP`) if connection fails.
*   **Watchdog Run Protection**: Automatically shuts off the pump if it runs continuously for more than 30 seconds to prevent motor burnout.
*   **Data Smoothing Filter**: Employs a 10-sample rolling average filter on analog inputs to filter out electrical sensor noise.
*   **Local Web Hosting**: Serves all HTML/CSS/JS dashboard files directly from the ESP32 flash memory using the LittleFS filesystem.
*   **System Visuals**: Provides immediate status updates via a physical 0.96" OLED screen.

### 3.3 Working Principle
The ESP32 continuously samples the environment. In AUTO mode, if the filtered soil moisture falls below 30% and no rain is detected, the ESP32 drives the relay pin low to turn the pump on. The pump turns off when moisture reaches 70% or if rain is detected. In MANUAL mode, the user toggles the pump via HTTP POST requests from the dashboard. Telemetry and network details are constantly shown on the OLED to alert the user of startup or watchdog timeouts.

---

# 4. Hardware Components

The following table lists the hardware components used in the AgriSense project:

| Component | Quantity | Purpose |
| :--- | :---: | :--- |
| **ESP32 DevKit V1** | 1 | The main microcontroller that processes data, runs the local web server, and controls outputs. |
| **DHT22 Sensor** | 1 | Measures the ambient air temperature and relative humidity. |
| **Capacitive Soil Moisture Sensor** | 1 | Measures the soil moisture level using capacitive changes. |
| **BH1750 Light Sensor** | 1 | Measures the ambient light intensity in Lux. |
| **Rain Sensor Module** | 1 | Detects active rainfall and maps the analog moisture level. |
| **0.96" SSD1306 OLED Display** | 1 | Shows the IP address, sensor readings, and pump state. |
| **1-Channel Relay Module** | 1 | Switches the water pump on and off using the ESP32's 3.3V control signal. |
| **Mini DC Water Pump** | 1 | Pumps water from the reservoir to the soil when the relay closes. |
| **Rechargeable Battery (2S Li-ion)** | 1 | Provides portable power (7.4V DC) to the system. |
| **Battery Holder** | 1 | Houses the battery securely. |
| **LM2596 Buck Converter** | 1 | Steps down the battery voltage to a regulated 5.0V for the ESP32 and relay. |
| **Power Switch (SPST)** | 1 | Connects or disconnects battery power to the system. |
| **Breadboard** | 1 | Used for solderless prototyping of the connections. |
| **Jumper Wires** | 1 | Connects the signal and power pins between modules. |
| **Water Tubing** | 1 | Routes water from the pump output to the soil. |
| **Water Reservoir** | 1 | Holds the water supply used for irrigation. |

---

# 5. Software Design

The software consists of modular C++ files written in the Arduino IDE and a light-mode web dashboard. The firmware is organized into manager files (such as `sensor_manager`, `pump_controller`, and `web_server`) to keep the code organized. The web dashboard consists of an HTML page, a CSS stylesheet for layout, and a JavaScript file that polls the ESP32 REST API every 2 seconds.

---

### 5.1 Program 1 – Sensor Initialization
*   **Purpose**: Configures input pins and starts library communications for the DHT22 and I2C sensors.
*   **Explanation**: Sets up pin directions, initializes the I2C bus via Wire, and starts the light sensor and DHT22 libraries.
*   **Code**:
    ```cpp
    void initSensors() {
        dhtSensor.begin();
        pinMode(SOIL_ADC_PIN, INPUT);
        pinMode(RAIN_ADC_PIN, INPUT);
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        lightSensor.begin(BH1750_TO_GROUND);
        lightSensor.start();
    }
    ```

### 5.2 Program 2 – Sensor Data Acquisition
*   **Purpose**: Gathers and filters analog and digital data from environmental sensors.
*   **Explanation**: Measures soil and rain levels, applies a 10-sample rolling average filter to reduce noise, and reads temperature and humidity from the DHT22.
*   **Code**:
    ```cpp
    void updateSensors() {
        unsigned long now = millis();
        if (now - lastSoilSampleTime >= READ_SOIL_INTERVAL_MS) {
            float rawSoil = analogRead(SOIL_ADC_PIN);
            addSoilSample(rawSoil); // Save to rolling buffer
            float avgSoil = computeAverage(soilFilterBuffer, FILTER_SIZE);
            currentSoilMoisturePct = constrain(((SOIL_MOISTURE_DRY - avgSoil) / 
                                     (float)(SOIL_MOISTURE_DRY - SOIL_MOISTURE_WET)) * 100.0f, 0.0f, 100.0f);
            lastSoilSampleTime = now;
        }
    }
    ```

### 5.3 Program 3 – OLED Display
*   **Purpose**: Draws current telemetry values and network connections on the OLED screen.
*   **Explanation**: Refreshes the display frame every second with the IP address, soil moisture, climate data, and pump status.
*   **Code**:
    ```cpp
    void drawMonitoringScreen(float temp, float hum, float moisture, float light, 
                              const String& rain, const String& mode, bool pumpRunning, 
                              const String& ipAddr, int rssi) {
        display.clearDisplay();
        display.printf("IP: %s  RSSI:%d", ipAddr.c_str(), rssi);
        display.drawFastHLine(0, 9, 128, SSD1306_WHITE);
        display.printf("Soil: %.1f%%\nTemp: %.1fC", moisture, temp);
        // ... (Print other telemetry metrics)
        display.display();
    }
    ```

### 5.4 Program 4 – Relay & Pump Control
*   **Purpose**: Toggles the pump relay and monitors running times for system safety.
*   **Explanation**: Activates the relay via GPIO, tracking the start time. If the pump runs past 30 seconds, it shuts off and triggers a lockout.
*   **Code**:
    ```cpp
    void setPumpState(bool turnOn) {
        if (turnOn && isTimeoutActive) return;
        isPumpRunning = turnOn;
        if (turnOn) {
            pumpStartTime = millis();
            pinMode(PUMP_RELAY_PIN, OUTPUT);
            digitalWrite(PUMP_RELAY_PIN, RELAY_TRIGGER_LEVEL);
        } else {
            pinMode(PUMP_RELAY_PIN, INPUT); // Float pin to turn off relay
        }
    }
    ```

### 5.5 Program 5 – Wi-Fi & Web Server
*   **Purpose**: Manages the Wi-Fi connection and starts the local HTTP web server.
*   **Explanation**: Connects to the configured router, falling back to AP hotspot mode if connection fails, and mounts the LittleFS filesystem.
*   **Code**:
    ```cpp
    void initWiFi() {
        WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);
        // ... (Check connection; fallback to softAP if timeout)
        LittleFS.begin(true);
        server.begin();
    }
    ```

### 5.6 Program 6 – REST API
*   **Purpose**: Sets up JSON endpoints for remote monitoring and pump control.
*   **Explanation**: Formats sensor readings into a JSON payload for GET requests and parses state updates for pump and mode triggers.
*   **Code**:
    ```cpp
    void handleGetStatus() {
        StaticJsonDocument<512> doc;
        doc["temperature"] = SensorManager::getTemperature();
        doc["soilMoisture"] = SensorManager::getSoilMoisturePct();
        // ... (Populate JSON status variables)
        String res;
        serializeJson(doc, res);
        server.send(200, "application/json", res);
    }
    ```

### 5.7 Program 7 – Final Integrated Firmware
*   **Purpose**: Orchestrates initialization and the main loop.
*   **Explanation**: Initializes logging, display, sensors, and servers during setup, then updates drivers and checks watering thresholds in the loop.
*   **Code**:
    ```cpp
    void loop() {
        SensorManager::updateSensors();
        PumpController::updatePumpWatchdog();
        WebServerManager::updateWebServer();
        // ... (Check automatic watering thresholds)
    }
    ```

---

# 6. System Design

### 6.1 Overall System Description
The AgriSense system uses a clean block layout. The ESP32 reads inputs from the temperature, humidity, light, soil moisture, and rain sensors. Power is supplied by a 7.4V battery, stepped down to 5.0V by a buck converter, and fed to the ESP32 and relay. The sensors are powered by the ESP32's 3.3V output. The pump is switched by the relay, and the web server processes remote user actions.

### 6.2 Circuit Diagram
The circuit diagram below details the electrical connections between the ESP32, sensors, buck converter, and relay loop.

```text
                  +-----------------------------------+
                  |      2S Li-ion Battery (7.4V)     |
                  +-----------------+-----------------+
                                    |
                               Power Switch
                                    |
                  +-----------------+-----------------+
                  |      LM2596 Buck Converter        |
                  |          (5.0V Regulated)         |
                  +--------+-----------------+--------+
                           | OUT+            | OUT-
                           v                 v
                       [ 5V Rail ]      [ GND Rail ]
                           |                 |
            +--------------+                 +-------------------------+
            |                                                          |
            v                                                          v
     +--------------+                 +--------------+                 |
     |  ESP32 VIN   |                 |  Relay VCC   |                 |
     +------+-------+                 +------+-------+                 |
            |                                |                         |
         3.3V LDO                            |                         |
            |                                |                         |
            v [ 3.3V Rail ]                  |                         |
            +------------+                   |                         |
            |            |                   |                         |
            v            v                   |                         |
       [Sensors]    [OLED/BH1750]            |                         |
                                             |                         |
     +---------------------------------------+-------------------------+
     |                                                                 |
     |                     ESP32 MICROCONTROLLER                       |
     |                                                                 |
     |  GPIO 21 (SDA)  <=============>  OLED SDA / BH1750 SDA          |
     |  GPIO 22 (SCL)  <=============>  OLED SCL / BH1750 SCL          |
     |  GPIO 32 (ADC)  <-------------  Soil Moisture Probe (Analog)    |
     |  GPIO 33 (ADC)  <-------------  Rain Sensor Board (Analog)      |
     |  GPIO 19 (Dig)  <-------------  DHT22 Temperature Data          |
     |                                                                 |
     |  GPIO 16 (Out)  ------------->  Relay IN (Trigger Pin)          |
     +-----------------------------------------------------------------+

   [ Relay Loop ]
   5V Rail ----> [ Relay COM Contact ]
                 [ Relay NO  Contact ] ----> [ DC Pump (+) ] ----+
                                                                 |
                                                           Flyback Diode (1N4007)
                                                                 |
   GND Rail <-------------------------------- [ DC Pump (-) ] <---+

================================================================================
                           Figure 1. Circuit Diagram
================================================================================
```

### 6.3 Flowchart
The flowchart below maps the logic flow of the firmware, showing the setup phase, sensor updates, auto-watering decisions, and safety watchdogs.

```text
  [ Power On ]
        |
        v
  [ Init Logging, Display, Sensors & Relay ]
        |
        v
  [ Connect Wi-Fi (AP Mode Fallback if timed out) ]
        |
        v
  [ Mount LittleFS & Start HTTP Web Server on Port 80 ]
        |
        v
  +---> [ Core Execution Loop ]
  |     |
  |     v
  |  [ Update Sensors, Wi-Fi status, Watchdog, & Web Client ]
  |     |
  |     v
  |  [ Is Telemetry Interval Elapsed (1 second)? ]
  |     |
  |     +---> (No)  ---+
  |     | (Yes)        |
  |     v              v
  |  [ Read active sensor values & check safety errors ]
  |     |
  |     v
  |  [ Sensor Fault Active? ]
  |     |
  |     +---> (Yes) ---> [ Stop Pump, Show Error ] -------+
  |     | (No)                                           |
  |     v                                                |
  |  [ Pump Run Watchdog Triggered (>30s)? ]             |
  |     |                                                |
  |     +---> (Yes) ---> [ Stop Pump, Watchdog Error ] --+
  |     | (No)                                           |
  |     v                                                |
  |  [ Operating Mode == AUTO? ]                         |
  |     |                                                |
  |     +---> (No)  ---> [ Read Manual Web Commands ] --+
  |     | (Yes)                                          |
  |     v                                                |
  |  [ Pump ON? ]                                        |
  |     |                                                |
  |     +---> (Yes) ---> [ Moisture >= 70% OR Rain? ]    |
  |     |                      |                         |
  |     |                      +---> (Yes) -> [ Pump OFF]|
  |     |                      +---> (No)  -> [ Keep ON ]|
  |     |                                                |
  |     +---> (No)  ---> [ Moisture < 30% AND Dry? ]     |
  |                            |                         |
  |                            +---> (Yes) -> [ Pump ON ]|
  |                            +---> (No)  -> [ Keep OFF]|
  |                                                      |
  |     +------------------------------------------------+
  |     |
  |     v
  |  [ Is OLED Refresh Interval Elapsed (1 second)? ]
  |     |
  |     +---> (Yes) ---> [ Refresh OLED Telemetry Screen ]
  |     +---> (No)  ---> [ Continue Loop ]
  |     |
  +-----+

================================================================================
                           Figure 2. System Flowchart
================================================================================
```

### 6.4 System Workflow
1.  **Startup**: The system powers on, runs self-checks, mounts the LittleFS partition, and connects to Wi-Fi.
2.  **Monitoring**: The ESP32 continuously updates sensor readings in the background using non-blocking timers.
3.  **Irrigation (AUTO)**: The system checks if soil moisture is below 30%. If dry and there is no rain, the pump starts. When the soil moisture reaches 70% or rain is detected, the pump stops.
4.  **Watchdog Check**: If the pump is running, the timer checks its duration. If it runs for 30 seconds, the pump is shut down and the OLED shows an error.
5.  **Web Request Handling**: The server responds to status requests from the web browser dashboard and processes manual command inputs.

---

# 7. Implementation

1.  **Breadboard Assembly**: Mounted the ESP32 DevKit V1 and connected the OLED and BH1750 sensors to the shared I2C bus pins.
2.  **Sensor and Actuator Wiring**: Connected the DHT22, soil moisture probe, and rain sensor to the ESP32. Wired the relay module trigger to its control pin.
3.  **Power Step-Down Calibration**: Wired the battery output to the input of the LM2596 buck converter. Calibrated the converter output to exactly 5.0V before connecting it to the ESP32 and relay.
4.  **Hardware Testing**: Flashed basic test sketches to scan the I2C bus and verify that the sensors and OLED screen were responding correctly.
5.  **Moisture Calibration**: Placed the soil moisture probe in dry air and water, recorded the raw analog values, and updated the calibration boundaries in the configuration file.
6.  **Software Development**: Programmed the C++ drivers and combined them in the main sketch. Built the HTML, CSS, and JavaScript web dashboard.
7.  **Uploading and Flashing**: Flashed the dashboard files to the LittleFS partition using the upload tool and uploaded the main firmware to the ESP32.
8.  **Final Testing**: Assembled the components in a temporary setup, connected the pump and tubing, and verified automatic and manual watering routines.

---

# 8. Results and Testing

Testing was performed at individual module and overall integration levels to verify system performance. During unit testing, sensor readings were verified via the Serial Monitor to ensure accuracy. The OLED displayed appropriate metrics under varying conditions. The relay successfully switched power to the pump when commanded.

Integrated system tests confirmed the automatic control logic: the pump turned on when moisture fell below 30% and turned off when it reached 70%. When rain was introduced, the rain sensor immediately cut off the pump, demonstrating correct override behavior. The web dashboard accurately updated its status readouts and successfully executed manual pump toggling in Manual mode.

The summary of the tests and results is presented in the table below:

| Test | Expected Result | Status |
| :--- | :--- | :---: |
| **Sensor Reading Test** | ESP32 successfully reads values from soil, DHT22, light, and rain sensors. | **PASSED** |
| **OLED Display Test** | OLED displays IP address, sensor readings, and pump state, updating every 1 second. | **PASSED** |
| **Relay Test** | Driving the relay control pin LOW triggers the relay switch. | **PASSED** |
| **Pump Test** | Water pump turns on when the relay is active and pumps water through the tubing. | **PASSED** |
| **Web Dashboard Test** | Dashboard loads on local devices, shows live sensor data, and allows pump toggle in Manual mode. | **PASSED** |
| **Automatic Irrigation Test** | Pump turns on below 30% soil moisture and turns off once moisture exceeds 70% or rain is detected. | **PASSED** |

---

# 9. Challenges

*   **Soil Moisture Sensor Calibration**: Mapping the analog voltage of the capacitive soil moisture sensor required manual measurement in dry air and wet soil to find the correct calibration limits and prevent inaccurate moisture percentages.
*   **Relay Active LOW Logic**: The relay module used an active-low trigger (driving the pin LOW turned the switch ON). The control code had to be inverted to prevent the pump from running unexpectedly on boot.
*   **Common Ground Wiring**: Mixing analog sensors with digital modules caused sensor noise. Connecting all ground connections to a single common ground bus (star grounding) resolved this issue.
*   **Power Supply Arrangement**: Attempting to power the pump directly from the ESP32's regulator caused brownouts. The circuit was redesigned to draw power directly from the LM2596 buck converter.
*   **OLED Flickering**: The high current draw when the pump turned on caused the OLED screen to flicker and reset. This was resolved by placing a flyback diode across the pump terminals.

---

# 10. Future Improvements

*   **Mobile Application**: Develop a native mobile app companion using Flutter or React Native to simplify local device connection and network discovery.
*   **Cloud Data Logging**: Integrate an external IoT cloud platform (such as ThingSpeak or Adafruit IO) to log long-term sensor trends over the internet.
*   **Weather API Integration**: Connect to an online weather service over Wi-Fi to delay scheduled watering cycles if rain is forecast.
*   **Solar-Powered Operation**: Add a solar panel and a battery charging circuit (such as a TP4056 or CN3791) to enable self-sufficient outdoor operation.
*   **Multiple Irrigation Zones**: Expand the system to control multiple relay valves independently to water different garden beds according to different plant needs.

---

# 11. Conclusion

The AgriSense Smart Irrigation System successfully demonstrates an automated, low-cost solution for monitoring soil conditions and controlling watering. By combining the ESP32 microcontroller with sensors, a relay, and a DC pump, the system successfully executes automatic watering thresholds based on soil moisture and rain detection. The local web dashboard served from LittleFS provides a clean, responsive interface for wireless telemetry visualization and manual override. Built on non-blocking structures with safety protections like the pump watchdog and Wi-Fi AP fallback, AgriSense provides a reliable prototype for residential gardens and greenhouses.
