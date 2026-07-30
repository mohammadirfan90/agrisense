# 1. Cover Page

```text
================================================================================
                                 PROJECT REPORT
                                       ON
                 AgriSense: Smart Irrigation System Using ESP32
================================================================================

                                  COURSE NAME
                             [Insert Course Name]

                                 SUBMITTED BY:
                             [Insert Student Names]

                                  STUDENT IDS:
                              [Insert Student IDs]

                                 SUBMITTED TO:
                             [Insert Instructor Name]

                                  DEPARTMENT:
                       [Insert Department of Engineering]

                                  UNIVERSITY:
                             [Insert University Name]

                               SUBMISSION DATE:
                              [Insert Date Here]
```

---

# 2. Introduction

Traditional agricultural methods heavily rely on manual monitoring of environmental parameters, which often leads to inefficient water consumption and compromised crop yields. Over-watering causes root rot, soil erosion, and nutrient leaching, while under-watering results in plant dehydration, wilting, and reduced growth. Given that agriculture accounts for a substantial percentage of global freshwater usage, optimizing water consumption is a vital ecological and financial necessity.

To address these challenges, embedded Internet of Things (IoT) solutions offer a robust, automated framework. Microcontrollers integrated with environmental sensors can continuously analyze microclimates and soil characteristics in real time. By automating water delivery according to real-time telemetry rather than fixed schedules, these systems prevent water waste and reduce manual labor.

**AgriSense** is developed as a localized, low-cost embedded IoT prototype. It utilizes the ESP32 microcontroller to monitor soil moisture, air temperature, relative humidity, light intensity, and precipitation. By processing these environmental variables, it executes closed-loop control over a local water pump relay and hosts a local web dashboard to enable remote tracking and manual override.

---

# 3. Project Overview

### 3.1 Objectives
*   **Automated Sensing**: Collect microclimate telemetry (soil moisture, temperature, humidity, light, and rain status) using low-cost sensor modules at non-blocking intervals.
*   **Autonomous Water Regulation**: Implement a closed-loop rule engine to toggle a water pump relay automatically when soil moisture drops below a threshold, unless active rainfall is detected.
*   **Local Web Hosting & UI**: Host an HTTP web server directly on the ESP32 to serve a responsive, zero-dependency light-mode dashboard and run a REST API for remote control.
*   **Physical Feedback & Safety**: Provide real-time data display on a physical OLED screen, trigger acoustic alarm sequences via a buzzer during hardware faults, and enforce safety watchdogs to prevent pump burnout.

### 3.2 Key Features
*   **Dynamic Dual-Mode Control**: Supports an automatic threshold-based irrigation mode (AUTO) and a manual dashboard override mode (MANUAL).
*   **Fail-Safe Lockouts**: Features a 30-second maximum pump run watchdog, sensor disconnect detection, and emergency rain shutoff.
*   **Local Connectivity**: Connects to an existing Wi-Fi router (Station mode) or automatically broadcasts its own Wi-Fi Access Point (AP mode) if the router is unavailable.
*   **High Performance & Low Latency**: Written using non-blocking C++ loop structures (utilizing `millis()` instead of `delay()`) to ensure the HTTP server and OLED refresh do not block sensor sampling.

### 3.3 Working Principle
The system continuously samples the physical environment. Sensor data is processed by the ESP32 CPU using calibration constraints and rolling average filters to eliminate electronic noise. Under the **AUTO** mode, if the filtered soil moisture falls below the lower threshold (default 30%) and the rain sensor is dry, the ESP32 drives the relay control pin high (active-low triggered) to start the water pump. When the moisture reaches the wet target (default 70%), or if the rain sensor detects water drops, the relay is immediately deactivated. 

Under the **MANUAL** mode, the user can toggle the pump relay using HTTP POST requests from the dashboard. The system status, Wi-Fi details, and telemetry are constantly written to the SSD1306 OLED display and exposed via JSON REST endpoints. If a sensor fails to respond or the pump runs continuously for over 30 seconds without moisture changes, the buzzer sounds, the OLED displays an error screen, and the pump shuts down.

---

# 4. Hardware Components

The following table lists the hardware components utilized in the AgriSense project:

| Component | Quantity | Purpose |
| :--- | :---: | :--- |
| **ESP32 DevKit V1** | 1 | Serves as the central MCU. Collects sensor data, runs control logic, updates OLED, and hosts the local HTTP web server. |
| **DHT22 Sensor** | 1 | Measures ambient temperature and relative humidity of the air. |
| **Capacitive Soil Moisture Sensor** | 1 | Measures relative soil moisture percentage via capacitive dielectric permittivity. Resistant to corrosion. |
| **BH1750 Light Sensor** | 1 | Measures ambient light intensity in Lux over I2C to determine daylight status. |
| **Rain Sensor Module** | 1 | Detects the presence and intensity of active rainfall (Dry, Light, Heavy). |
| **0.96" SSD1306 OLED Display** | 1 | Displays local network IP address, Wi-Fi signal strength, sensor values, and active alarms. |
| **1-Channel Relay Module** | 1 | Switches the high-current 5V DC line feeding the water pump using a 3.3V logic trigger. |
| **Mini DC Water Pump** | 1 | Delivers water from the reservoir to the soil when the relay contacts close. |
| **Active Buzzer** | 1 | Sounds audible alarm beeps during system boot, sensor faults, or pump run timeouts. |
| **Rechargeable Battery (2S Li-ion)** | 1 | Supplies raw input voltage (7.4V DC) for portable operation. |
| **Battery Holder** | 1 | Houses the battery pack securely and provides terminal wire connections. |
| **LM2596 Buck Converter** | 1 | Steps down the battery voltage (7.4V) to a regulated 5.0V DC to power the ESP32 and relay. |
| **Power Switch (SPST)** | 1 | Connects or disconnects the battery from the buck converter to power the system on/off. |
| **Breadboard** | 1 | Holds the ESP32 and components for clean, solderless prototyping. |
| **Jumper Wires** | 1 | Connects signal, power, and ground pins between modules. |
| **Water Tubing** | 1 | Directs the water output from the pump nozzle to the plant soil. |
| **Water Reservoir** | 1 | Holds the water supply utilized for irrigation. |

---

# 5. Software Design

The software architecture of AgriSense consists of modular C++ drivers and a lightweight web dashboard. The firmware is structured using object-oriented principles, dividing responsibilities into distinct files: [config.h](file:///e:/AgriSense/firmware/AgriSense/config.h) for thresholds, [pins.h](file:///e:/AgriSense/firmware/AgriSense/pins.h) for pin definitions, and manager modules for hardware. The web dashboard uses semantic HTML5, CSS Grid, and vanilla JavaScript to parse JSON and issue controls.

---

### 5.1 Program 1 – Sensor Initialization
*   **Purpose**: Configures the I/O direction of sensor pins, begins I2C bus communications, initializes the DHT22 and BH1750 drivers, and pre-fills the analog rolling-average buffers to avoid boot startup issues.
*   **Code**:
    ```cpp
    // [Placeholder: Insert Sensor Initialization Source Code Here]
    ```

### 5.2 Program 2 – Sensor Data Acquisition
*   **Purpose**: Executes non-blocking periodic reads of the soil moisture, air DHT22, rain, and light sensors, applies a 10-sample rolling average filter to the analog inputs, and sets a fault flag if digital communications fail.
*   **Code**:
    ```cpp
    // [Placeholder: Insert Sensor Data Acquisition Source Code Here]
    ```

### 5.3 Program 3 – OLED Display
*   **Purpose**: Draws structural UI screens on the 128x64 SSD1306 display via I2C, updating local network parameters (IP, RSSI), sensor values, and warning screens when safety lockouts trigger.
*   **Code**:
    ```cpp
    // [Placeholder: Insert OLED Display Source Code Here]
    ```

### 5.4 Program 4 – Relay & Pump Control
*   **Purpose**: Manages pump relay states, enforces command debouncing and a 10-second cooldown window between activations, and runs a software watchdog that cuts off power if the pump operates continuously for more than 30 seconds.
*   **Code**:
    ```cpp
    // [Placeholder: Insert Relay and Pump Control Source Code Here]
    ```

### 5.5 Program 5 – Wi-Fi & Web Server
*   **Purpose**: Establishes Station mode connection to a local router or configures a fallback Access Point hotspot if the router times out, mounts the LittleFS partition, and binds HTTP server routing handlers on Port 80.
*   **Code**:
    ```cpp
    // [Placeholder: Insert Wi-Fi and Web Server Source Code Here]
    ```

### 5.6 Program 6 – REST API
*   **Purpose**: Handles incoming client requests, serializes telemetry and alarm states to JSON format for GET requests, and parses state changes (Auto/Manual mode, Pump ON/OFF) for POST requests.
*   **Code**:
    ```cpp
    // [Placeholder: Insert REST API Request Handlers Source Code Here]
    ```

### 5.7 Program 7 – Final Integrated Firmware
*   **Purpose**: Combines all manager modules, runs initialization routines in the setup function, and schedules updates dynamically inside the non-blocking execution loop.
*   **Code**:
    ```cpp
    // [Placeholder: Insert Final Integrated Firmware AgriSense.ino Here]
    ```

---

# 6. System Design

### 6.1 Overall System Description
AgriSense uses a modular architecture. The ESP32 MCU reads analog data from the soil moisture and rain sensor boards, I2C digital packets from the BH1750 lux and SSD1306 OLED, and 1-wire pulses from the DHT22. Power is distributed from a 7.4V Battery, stepped down to a regulated 5V via a buck converter to feed the ESP32 VIN and Relay VCC. The ESP32's onboard LDO regulator supplies 3.3V to the sensors. Under automatic control, decisions are determined based on environmental boundaries, while manual command routes are managed by the HTTP WebServer.

### 6.2 Circuit Diagram
The circuit diagram below details the electrical wiring, including sensor pin layouts, power lines, NPN transistor driver wiring for the active buzzer, and flyback diode protection for the pump relay contacts.

```text
================================================================================
                    [Placeholder: Insert Circuit Diagram Here]
================================================================================
```

### 6.3 Flowchart
The flowchart below traces the execution logic of the main firmware loop, mapping sensor sampling, decision rules, watchdog monitoring, and HTTP request routing.

```text
================================================================================
                      [Placeholder: Insert Flowchart Here]
================================================================================
```

### 6.4 System Workflow
1.  **Boot Phase**: The system initializes logging, configures hardware pins, mounts the LittleFS filesystem, initializes the sensors, and attempts to connect to Wi-Fi. The buzzer sounds a startup beep, and the OLED displays a boot screen.
2.  **Sensing Loop**: Every loop iteration, the background sensor filters, Wi-Fi status, and pump watchdog timers are updated. Sensors are sampled at non-blocking intervals (e.g. 500ms for rain, 1000ms for moisture, 2000ms for DHT22).
3.  **Irrigation Logic (AUTO)**: If soil moisture is below the dry threshold (30%) and no rain is detected, the relay switches on. If moisture reaches 70% or rain is detected, the relay switches off.
4.  **Watchdog Monitoring**: If the pump operates continuously for 30 seconds, it shuts down, enters a safety timeout state, displays a "PUMP TIMEOUT" error on the OLED, and triggers the buzzer.
5.  **Web Request Handling**: In the background, the server listens for HTTP requests. If a client connects, it serves the static dashboard files from LittleFS or executes JSON API state requests.

---

# 7. Implementation

The AgriSense system was built and integrated in the following chronological sequence:

1.  **Hardware Breadboard Assembly**: Wired the ESP32 DevKit V1 on a breadboard. Connected the SSD1306 OLED and BH1750 to the shared I2C bus (GPIO 21 and 22), the DHT22 to GPIO 19, and the analog probes to GPIO 32 and 33.
2.  **Actuator Driver Integration**: Wired the relay module trigger to GPIO 16 and the active buzzer module to GPIO 17. Connected the water pump to the relay output contacts with a 1N4007 flyback diode across the pump terminals.
3.  **Power Calibration**: Adjusted the LM2596 buck converter output to exactly 5.0V using a multimeter. Wired the 5V line to the ESP32 VIN and relay VCC, and connected all grounds in a common ground bus.
4.  **Basic Firmware Diagnostics**: Flashed an I2C scanner sketch to verify that the OLED (address `0x3C`) and BH1750 (address `0x23`) were detected on the I2C bus. Verified serial printing at 115200 baud.
5.  **Sensor Calibration**: Collected raw ADC telemetry from the soil moisture sensor in dry air and fully submerged in water to configure `SOIL_MOISTURE_DRY` and `SOIL_MOISTURE_WET` values. Calibrated the rain sensor module wet/dry thresholds.
6.  **Modular Software Coding**: Implemented separate manager source files (`sensor_manager`, `pump_controller`, `display_manager`, `wifi_manager`, `web_server`, `alarm_manager`). Combined them inside the main C++ orchestrator loop.
7.  **Web Dashboard Frontend Design**: Developed the web files (`index.html`, `style.css`, `script.js`). Coded the light-mode UI using solid backgrounds and rounded borders. Wrote a browser simulation engine inside the JavaScript file for offline mock testing.
8.  **LittleFS Integration & Flashing**: Configured the partition scheme in the IDE. Uploaded the web files to the LittleFS filesystem partition on the ESP32, and flashed the compiled C++ firmware binary.
9.  **Deployment & Field Testing**: Placed the sensor probes into soil, ran integration tests via Wi-Fi Station and AP connections, and verified automatic and manual watering routines.

---

# 8. Results and Testing

A series of functional tests were performed to verify system performance and safety features:

| Test | Expected Result | Status |
| :--- | :--- | :---: |
| **Sensor Read Test** | Sensors sample telemetry. Temperature, humidity, light, rain state, and soil moisture print to the Serial Monitor. | **PASSED** |
| **OLED Display Test** | Displays telemetry metrics and Wi-Fi IP address clearly. Automatically updates every 1 second. | **PASSED** |
| **Relay Toggle Test** | Writing GPIO 16 LOW triggers the relay coil and activates the internal optocoupler. | **PASSED** |
| **Pump Water Test** | Water pump operates when relay is closed, delivering water through the vinyl tubing. | **PASSED** |
| **Web Dashboard Test** | Dashboard loads over the local IP. Shows live data updates, changes mode, and allows pump control. | **PASSED** |
| **AUTO Irrigation Test** | Pump starts when moisture drops below 30%, and shuts off when moisture reaches 70%. | **PASSED** |
| **Rain Lockout Test** | Pouring water on the rain sensor while irrigation is active instantly stops the pump. | **PASSED** |
| **Pump Timeout Test** | Pump shuts down and locks out if it runs for 30s without moisture changes. Buzzer sounds alarm. | **PASSED** |
| **Sensor Fault Alarm Test** | Disconnecting the DHT22 triggers the buzzer warning pattern and displays an error screen. | **PASSED** |
| **Wi-Fi AP Fallback Test** | Disconnecting the router forces the ESP32 to start `AgriSense_AP` within 15 seconds. | **PASSED** |

---

# 9. Challenges and Future Improvements

### 9.1 Technical Challenges
*   **Analog Pin Constraints**: Using Wi-Fi disables ESP32 ADC2 channels. This required rerouting analog inputs strictly to ADC1 channels (GPIO 32 and 33) to prevent sensor read failures.
*   **Inductive Noise Interference**: Toggling the inductive motor of the water pump created electromagnetic interference, causing ESP32 reset loops. This was resolved by installing a 1N4007 flyback diode across the pump terminals and decoupling capacitors on the power rails.
*   **Legacy SPIFFS Deprecation**: The legacy SPIFFS filesystem is deprecated in modern Arduino libraries. We migrated to LittleFS, requiring a custom partition scheme and updating file system macros in the code.
*   **Arduino IDE 2.x File Uploads**: Legacy Java-based upload plugins do not work in the new Arduino IDE 2.x. This was resolved by configuring the new Command Palette LittleFS extension to flash web directories.
*   **Power Management**: The active buzzer, sensors, and OLED drew high current during system boot, causing voltage brownouts. We resolved this by routing power through the LM2596 buck converter instead of the ESP32 regulator.

### 9.2 Future Improvements
*   **Deep Sleep Optimization**: Implement ESP32 deep sleep cycles between sampling intervals to reduce battery consumption, using external interrupts to wake up during rain events.
*   **Multi-Zone Support**: Expand the relay driver board and valve system to support multi-zone irrigation, allowing customized thresholds for different plant species.
*   **OTA (Over-The-Air) Updates**: Implement OTA firmware updating to allow updating the C++ sketch and LittleFS assets remotely over Wi-Fi.
*   **Real-Time Clock (RTC)**: Add an external I2C RTC module (such as the DS3231) to log precise time-stamped events and enforce time-of-day irrigation schedules (e.g. watering only during early morning or evening).
*   **Cloud Logging Integration**: Add optional cloud data streaming (MQTT/HTTPS) to external databases (like Firebase or AWS IoT) to track seasonal soil trend logs remotely.
*   **Soil Nutrient Sensing**: Integrate NPK (Nitrogen, Phosphorus, Potassium) soil sensors to analyze soil health and provide fertilizing recommendations on the dashboard.

---

# 10. Conclusion

The **AgriSense Smart Irrigation System** successfully demonstrates a localized, low-cost embedded IoT prototype for precision agriculture. By utilizing an ESP32 microcontroller, the system successfully automates soil moisture tracking and coordinates irrigation using non-blocking programming structures. 

The implementation of safety watches (such as rain lockouts, sensor disconnect triggers, and pump run watchdogs) ensures high reliability and protects the physical hardware. Serving a responsive web dashboard directly from the ESP32's internal LittleFS storage provides convenient monitoring and control capabilities without requiring external databases or internet access. 

Overall, AgriSense offers an efficient and practical solution for home gardens, small-scale greenhouses, and water-scarce agricultural zones.
