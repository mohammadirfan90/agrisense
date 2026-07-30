# 11. Implementation & Development Guide

This guide details the folder structure, development stages, firmware configurations, and Git strategies for developers building the **AgriSense** system.

---

## 1. Repository Folder Map

```
AgriSense/
│
├── docs/                               # Engineering Specifications
│   ├── 01-project-overview.md          # Overview & functional requirements
│   ├── 02-system-architecture.md       # Sequence, workflows, and state diagrams
│   ├── 03-hardware-components.md       # Hardware rationale & compatibility checks
│   ├── 04-pin-connections.md           # Pin tables & assignment strategies
│   ├── 05-circuit-design.md            # Circuit schematics & wiring logs
│   ├── 06-power-design.md              # Power budgets, battery configurations, grounding
│   ├── 07-data-flow.md                 # Signal mapping equations & JSON schemas
│   ├── 08-api-specification.md         # REST API documentation
│   ├── 09-testing-plan.md              # QA testing levels & safety verification
│   ├── 10-deployment.md                # Development milestones & document list
│   └── 11-implementation-guide.md      # This implementation plan document
│
├── firmware/                           # MCU Firmware Code Base
│   ├── AgriSense/                      # Target ESP32 Arduino project
│   │   ├── AgriSense.ino               # Setup, loop, initialization, and main run orchestrator
│   │   ├── config.h                    # Configurable thresholds, timing constants, Wi-Fi parameters
│   │   ├── pins.h                      # Unified hardware GPIO mapping constants
│   │   ├── sensor_manager.h/.cpp       # Sampling loops, sensor drivers, averaging filters
│   │   ├── pump_controller.h/.cpp      # Relay toggle wrappers, pump timers, watchdog overrides
│   │   ├── display_manager.h/.cpp      # Frame designs, rendering interfaces, status text logic
│   │   ├── web_server.h/.cpp           # Routing configurations, HTTP handlers, static files server
│   │   ├── wifi_manager.h/.cpp         # Connection handlers, AP fallback setup, auto-reconnect loop
│   │   ├── alarm_manager.h/.cpp        # Warning registers, alarm state logic, buzzer tones
│   │   └── utils.h/.cpp                # Logging helpers, string formatting utilities
│   └── libraries/                      # Third-party offline libraries (.zip archives)
│
├── web/                                # Frontend Source Files (stored in LittleFS)
│   ├── index.html                      # semantic structure with panels for gauges, controls, alerts
│   ├── style.css                       # Styles: grid layout, dark mode, responsive styling, glassmorphism cards
│   ├── api.js                          # AJAX wrappers for async fetch calls to REST API endpoints
│   ├── ui.js                           # Dashboard DOM renderers, gauge builders, chart draws
│   ├── script.js                       # Frontend entry point, orchestration logic, updates scheduler
│   └── assets/                         # SVG icons, system logo
│
├── report/                             # Academic & Presentation Deliverables
│   ├── screenshots/                    # Dashboard layout images
│   ├── diagrams/                       # Circuit designs and flowcharts
│   └── final-report.docx               # Academic project final report
│
├── .gitignore                          # Exclude build files, credentials, and VS Code user settings
├── LICENSE                             # MIT License
└── README.md                           # Project directory overview and documentation index
```

---

## 2. Step-by-Step Development Order

```
[Task 1: Basic Boot] -> [Task 2: I2C Scanner] -> [Task 3: Sensor Data] -> [Task 4: Display]
                                                                                |
[Task 8: Async Dashboard] <- [Task 7: HTML/CSS Dashboard] <- [Task 6: Web API] <- [Task 5: WiFi Setup]
    |
[Task 9: Safety Tests]
```

### Task 1: Basic Boot Verification
*   **Objective**: Verify ESP32 compilation, upload path, and bootloader stability.
*   **Dependencies**: Arduino IDE / PlatformIO installed, ESP32 board package loaded.
*   **Expected Output**: ESP32 boots and blinks the onboard LED at 1Hz. Status logs print to the Serial Monitor.
*   **Testing Method**: Compile and upload a basic blink sketch. Watch the built-in LED and monitor Serial at 115200 baud.
*   **Estimated Complexity**: Very Low.

### Task 2: I2C Bus Diagnostics
*   **Objective**: Scan the physical I2C bus to verify SSD1306 and BH1750 are wired correctly.
*   **Dependencies**: Task 1 complete, physical SDA/SCL wires connected.
*   **Expected Output**: Terminal lists I2C devices at addresses `0x23` (BH1750) and `0x3C` (OLED).
*   **Testing Method**: Flash an I2C scanner sketch and inspect the Serial output.
*   **Estimated Complexity**: Low.

### Task 3: Sensor Calibration & Sampling
*   **Objective**: Collect raw telemetry from all sensors (DHT22, BH1750, Moisture, Rain).
*   **Dependencies**: Task 2 complete, libraries installed.
*   **Expected Output**: Real-time temperature, humidity, light, moisture, and rain readings print to the Serial port.
*   **Testing Method**: Pour water on the rain sensor and place the soil moisture probe in wet and dry conditions. Verify that raw readings map correctly to expected values.
*   **Estimated Complexity**: Medium.

### Task 4: OLED & Buzzer UI Integration
*   **Objective**: Display system telemetry on the OLED screen and trigger buzzer tones.
*   **Dependencies**: Task 3 complete.
*   **Expected Output**: Telemetry displays on the screen. The buzzer sounds warning tones when sensors are disconnected.
*   **Testing Method**: Unplug a sensor during operation and verify that the OLED displays a warning and the buzzer sounds.
*   **Estimated Complexity**: Medium.

### Task 5: Wi-Fi Setup & AP Fallback
*   **Objective**: Connect the ESP32 to a local Wi-Fi router, falling back to a local Access Point if connection fails.
*   **Dependencies**: Task 4 complete.
*   **Expected Output**: The ESP32 connects to Wi-Fi and prints the assigned IP address, or boots its own AP network.
*   **Testing Method**: Attempt boot with correct Wi-Fi credentials, then with incorrect credentials to verify AP fallback.
*   **Estimated Complexity**: Medium.

### Task 6: REST API Endpoint Integration
*   **Objective**: Implement REST API endpoints returning system status as JSON payloads.
*   **Dependencies**: Task 5 complete.
*   **Expected Output**: Calling `/api/status` returns a JSON object containing current system states.
*   **Testing Method**: Use Postman or curl to call API endpoints and verify JSON formatting.
*   **Estimated Complexity**: Medium.

### Task 7: HTML/CSS Web Dashboard Design
*   **Objective**: Construct the dashboard layout and styles.
*   **Dependencies**: None.
*   **Expected Output**: Dashboard renders cleanly in a desktop or mobile browser.
*   **Testing Method**: Open `index.html` locally in a browser and test responsive behavior on various screen sizes.
*   **Estimated Complexity**: Medium.

### Task 8: Client API Integration (Async JS)
*   **Objective**: Connect the Web Dashboard to the ESP32 REST API.
*   **Dependencies**: Task 6 and Task 7 complete.
*   **Expected Output**: Dashboard fetches data from the API and updates display gauges and controls in real time.
*   **Testing Method**: Upload web files to ESP32 Flash storage (LittleFS). Access the dashboard via a browser and test controls.
*   **Estimated Complexity**: Medium-High.

### Task 9: Safety Watchdog & Run Timeout Verification
*   **Objective**: Verify system safety limits (pump runtime watchdogs, rain lockouts).
*   **Dependencies**: Task 8 complete.
*   **Expected Output**: Pump deactivates immediately when rain is detected or when runtime exceeds 30 seconds.
*   **Testing Method**: Run the pump with dry soil sensor readings. Trigger a simulated run timeout or pour water on the rain sensor to verify lockout.
*   **Estimated Complexity**: Medium.

---

## 3. Required Arduino Libraries

The firmware relies on these libraries for hardware communications and data parsing:

1.  **Adafruit SSD1306** (by Adafruit)
    *   *Purpose*: Hardware driver to control the SSD1306 128x64 OLED display over I2C.
    *   *Official Name*: `Adafruit SSD1306`
    *   *Installation*: Library Manager (Arduino IDE / PlatformIO).
    *   *Dependencies*: `Adafruit GFX Library`, `Adafruit BusIO`, `Wire.h`.
2.  **DHT Sensor Library** (by Adafruit)
    *   *Purpose*: Reads digital pulses from the DHT22 Temperature & Humidity sensor.
    *   *Official Name*: `DHT sensor library`
    *   *Installation*: Library Manager.
    *   *Dependencies*: `Adafruit Unified Sensor`.
3.  **BH1750** (by Stefan)
    *   *Purpose*: Configures and reads Light intensity (Lux) values from the BH1750 sensor over I2C.
    *   *Official Name*: `hp_BH1750`
    *   *Installation*: Library Manager.
    *   *Dependencies*: `Wire.h`.
4.  **ArduinoJson** (by Benoit Blanchon)
    *   *Purpose*: Parses incoming payloads and encodes system status to JSON for REST API endpoints.
    *   *Official Name*: `ArduinoJson`
    *   *Installation*: Library Manager.
    *   *Dependencies*: None.

---

## 4. Web Dashboard Stack Selection

The dashboard uses a vanilla web stack to optimize execution performance and fit within the ESP32's limited storage space:

*   **Semantic HTML5**: Defines the structural layout of the dashboard. Using semantic tags (`<header>`, `<main>`, `<section>`, `<article>`) improves SEO, code readability, and structure.
*   **Vanilla CSS3 (Grid & Flexbox)**: Styles the dashboard using custom variables, gradients, and hover transitions. This design uses responsive breakpoints to fit desktop, tablet, and mobile screens without the overhead of external CSS frameworks.
*   **Vanilla JavaScript (ES6+)**: Handles dashboard logic. It uses the asynchronous Fetch API to poll the REST endpoints, updates the UI DOM, and handles button click events.
*   **SVG Indicators**: Dynamically updates status indicators and circular gauges using SVG vectors rather than heavy external charting libraries.

---

## 5. Configuration File Structure (`config.h`)

Below is the design for the configuration file, containing system settings, thresholds, and timings:

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// ========================================================
// Wi-Fi Connection Parameters
// ========================================================
#define WIFI_STA_SSID     "AgriSense_Router"    // Target Wi-Fi router network
#define WIFI_STA_PASS     "RouterPassword123"   // Target Wi-Fi router password
#define WIFI_AP_SSID      "AgriSense_AP"        // Fallback Access Point name
#define WIFI_AP_PASS      "AgriSense_Secure88"  // AP security password (min 8 chars)
#define WIFI_CONN_TIMEOUT 15000                 // Time to wait before AP fallback (ms)

// ========================================================
// Sensor Calibration Parameters
// ========================================================
#define SOIL_MOISTURE_DRY 3100                  // Raw ADC value in dry air
#define SOIL_MOISTURE_WET 1400                  // Raw ADC value in water
#define RAIN_SENSOR_DRY   3500                  // Raw ADC value showing dry weather
#define RAIN_SENSOR_WET   1500                  // Raw ADC threshold for active rainfall

// ========================================================
// Control Thresholds
// ========================================================
#define DEFAULT_DRY_THRESHOLD 30.0              // Start pump when moisture drops below this %
#define DEFAULT_WET_THRESHOLD 70.0              // Stop pump when moisture reaches this %

// ========================================================
// System Timings (Non-blocking Millis)
// ========================================================
#define READ_SOIL_INTERVAL  1000                // Time between soil moisture reads (ms)
#define READ_DHT_INTERVAL   2000                // Time between DHT22 temperature/humidity reads (ms)
#define READ_LIGHT_INTERVAL 1000                // Time between BH1750 Lux reads (ms)
#define READ_RAIN_INTERVAL  500                 // Time between rain sensor reads (ms)
#define OLED_REFRESH_RATE   1000                // Time between display frame updates (ms)
#define TELEMETRY_INTERVAL  1000                // Time between general state evaluations (ms)

// ========================================================
// Safety Limits & Watchdog Constants
// ========================================================
#define PUMP_MAX_RUN_TIME  30000                // Max continuous run time before auto deactivation (ms)
#define PUMP_COOLDOWN_TIME 10000                // Min duration between sequential pump starts (ms)
#define RELAY_TRIGGER_LEVEL LOW                 // LOW = Active Low relay modules, HIGH = Active High
#define BUZZER_ALARM_FREQ   2300                // Acoustic warning tone pitch (Hz)
#define PUMP_DEBOUNCE_MS   500                  // Signal bounce prevention (ms)

#endif // CONFIG_H
```

---

## 6. Pin Constants Structure (`pins.h`)

This file defines the physical pin connections on the ESP32:

```cpp
#ifndef PINS_H
#define PINS_H

// ========================================================
// Shared I2C Bus Configuration
// ========================================================
#define I2C_SDA_PIN 21   // Shared SDA line for OLED and BH1750
#define I2C_SCL_PIN 22   // Shared SCL line for OLED and BH1750

// ========================================================
// Analog Inputs (ADC1 Only)
// ========================================================
#define SOIL_ADC_PIN 32  // Analog input for Soil Moisture (ADC1 Channel 4)
#define RAIN_ADC_PIN 33  // Analog input for Rain Sensor (ADC1 Channel 5)

// ========================================================
// Digital Inputs
// ========================================================
#define DHT_DATA_PIN 19  // Digital input pin for DHT22

// ========================================================
// Digital Actuator Outputs
// ========================================================
#define PUMP_RELAY_PIN 16 // Output driver pin for Relay Module (RX2 Pin)
#define BUZZER_CTRL_PIN 17 // Output driver pin for Buzzer Alert (TX2 Pin)

#endif // PINS_H
```

---

## 7. Firmware Module Designs

### 1. Main Application (`AgriSense.ino`)
*   **Responsibility**: Coordinates system startup and runs the main non-blocking execution loop.
*   **Public Functions**:
    *   `void setup()`: Initializes Serial, mounts LittleFS, runs setup on modules, and connects to Wi-Fi.
    *   `void loop()`: Calls execution routines on active modules at non-blocking intervals using `millis()`.
*   **Private/Internal States**:
    *   `unsigned long lastDHTReadTime`, `lastSoilReadTime`, `lastDisplayTime`, `lastTelemetryTime`.
*   **Dependencies**: All sub-modules, `config.h`, `pins.h`.
*   **Data Flow**: Reads timing logs, parses states, and passes telemetry down to modules.

### 2. Sensor Manager (`sensor_manager.h/.cpp`)
*   **Responsibility**: Samples analog and digital sensors, applies rolling average filters, and checks for hardware faults.
*   **Public Functions**:
    *   `void initSensors()`: Mounts DHT22, sets up I2C pins, and performs self-checks.
    *   `void sampleSensors()`: Reads sensors at set intervals.
    *   `float getMoisturePct()`, `float getTemperature()`, `float getHumidity()`, `float getLux()`: Return current filtered values.
    *   `String getRainState()`: Returns `"DRY"`, `"LIGHT"`, or `"HEAVY"`.
    *   `bool hasSensorError()`: Returns true if a sensor fails to respond.
*   **Private Responsibilities**:
    *   Maintains arrays for rolling averages.
    *   Calculates calibrations from RAW ADC inputs.
*   **Dependencies**: `DHT.h`, `Wire.h`, `hp_BH1750.h`, `config.h`, `pins.h`.
*   **Data Flow**: Passes raw analog/I2C signals to calibration functions, filters results, and stores them in public structures.

### 3. Pump Controller (`pump_controller.h/.cpp`)
*   **Responsibility**: Toggles the pump relay and monitors safety limits to prevent dry running or flooding.
*   **Public Functions**:
    *   `void initPump()`: Sets pin directions and initializes the relay to an inactive state.
    *   `void setPumpState(bool turnOn)`: Switches the relay. Rejects commands if a safety lockout is active.
    *   `bool getPumpState()`: Returns current relay status.
    *   `void evaluatePumpWatchdog()`: Shuts down pump if it runs past `PUMP_MAX_RUN_TIME`.
    *   `bool isLockoutActive()`: Returns true if a runtime lockout is active.
*   **Private Responsibilities**:
    *   Tracks pump start time to enforce max run duration.
    *   Checks cooldown durations before allowing reactivation.
*   **Dependencies**: `pins.h`, `config.h`.
*   **Data Flow**: Receives commands from the decision engine or REST API and updates the relay pin state.

### 4. Display Manager (`display_manager.h/.cpp`)
*   **Responsibility**: Formats and draws system statuses and sensor values on the OLED display.
*   **Public Functions**:
    *   `void initDisplay()`: Connects to the SSD1306 and draws the boot screen.
    *   `void updateScreen(float moisture, float temp, float lux, String mode, bool pump, String alert)`: Renders current telemetry.
    *   `void drawErrorScreen(String errorMsg)`: Shows critical faults.
*   **Private Responsibilities**:
    *   Draws screen boundaries, telemetry text, status labels, and indicators.
*   **Dependencies**: `Adafruit_SSD1306.h`, `Adafruit_GFX.h`, `pins.h`.
*   **Data Flow**: Receives system telemetry from the main loop and draws it to the screen.

### 5. Wi-Fi Manager (`wifi_manager.h/.cpp`)
*   **Responsibility**: Connects to the local network, sets up the fallback Access Point, and handles auto-reconnects.
*   **Public Functions**:
    *   `void initWiFi()`: Reads configuration settings and attempts router connection.
    *   `void handleWiFiReconnect()`: Checks connection status and triggers reconnects when needed.
    *   `String getLocalIP()`: Returns the assigned IP address.
    *   `bool isConnected()`: Returns connection status.
*   **Private Responsibilities**:
    *   Configures static IP routes and establishes AP credentials.
*   **Dependencies**: `WiFi.h`, `config.h`.
*   **Data Flow**: Manages the Wi-Fi hardware stack.

### 6. Web Server & API Controller (`web_server.h/.cpp`)
*   **Responsibility**: Serves UI files from LittleFS and handles incoming JSON requests.
*   **Public Functions**:
    *   `void initWebServer()`: Binds path routes and starts the server on port 80.
    *   `void processIncomingClients()`: Parses incoming connection requests.
*   **Private Responsibilities**:
    *   `void handleGetStatus()`, `void handlePostMode()`, `void handlePostPump()`: Router endpoints.
*   **Dependencies**: `WebServer.h`, `LittleFS.h`, `ArduinoJson.h`, `config.h`.
*   **Data Flow**: Converts requests to internal state changes, and serializes telemetry into JSON responses.

### 7. Alarm Manager (`alarm_manager.h/.cpp`)
*   **Responsibility**: Monitors safety limits and sounds alarms via the buzzer.
*   **Public Functions**:
    *   `void initAlarm()`: Configures buzzer pin outputs.
    *   `void evaluateAlarmStates(bool sensorError, bool pumpTimeout, bool lowBattery)`: Checks system states to determine if alarms should sound.
    *   `void soundAlarmTone(int patternType)`: Sounds warning sequences.
*   **Private Responsibilities**:
    *   Drives PWM/High signals to the buzzer.
*   **Dependencies**: `pins.h`, `config.h`.
*   **Data Flow**: Reads alarm triggers and generates hardware sound signals.

---

## 8. Web Dashboard Modules Design

To keep the web code clean and maintainable, the frontend files are organized into distinct files:

```
[index.html] (Semantic markup & UI structure)
      |
      |-- Imports --> [style.css] (Layout, glassmorphism, responsive grids)
      |
      +-- Loads ----> [script.js] (Core controller, timing loop, startup routines)
                           |
                           +-- Calls --> [api.js] (REST HTTP Fetch/AJAX requests)
                           +-- Calls --> [ui.js] (DOM changes, circular gauge updates)
```

1.  **`index.html` (Semantic Layout)**:
    *   Defines dashboard structure. Features cards for Soil Moisture, Air Temperature, Humidity, and Light.
    *   Includes control buttons for Mode Selection (AUTO/MANUAL) and Pump Activation (ON/OFF).
    *   Imports stylesheet and script modules.
2.  **`style.css` (Styles & Visual Design)**:
    *   Implements dark mode palette and glassmorphism styling.
    *   Uses Flexbox and CSS Grid to dynamically adjust layout for mobile, tablet, and desktop viewports.
    *   Styles status indicators, buttons, cards, and gauges.
3.  **`api.js` (Asynchronous REST Connector)**:
    *   Handles Fetch API calls to target endpoints (`GET /api/status`, `POST /api/mode`, `POST /api/pump`).
    *   Returns parsed JSON objects or handles network connection errors.
4.  **`ui.js` (DOM Renderer)**:
    *   Updates text fields, gauge displays, and toggle buttons based on system telemetry.
    *   Displays alert messages and handles transitions between AUTO and MANUAL mode interfaces.
5.  **`script.js` (Core Application Engine)**:
    *   Initializes the dashboard, starts the update loop, and processes user commands.
    *   Polls the REST API every 1000ms to fetch and update the display with the latest telemetry.

---

## 9. Development Checklist

```
Setup Phase
[ ] Install ESP32 Board package in IDE
[ ] Verify Serial Monitor communication
[ ] Run basic Blink test

Driver Verification
[ ] Run I2C Scanner to verify OLED & BH1750 addresses
[ ] Test DHT22 sensor readings
[ ] Calibrate Capacitive Soil Probe (record dry & wet limits)
[ ] Calibrate Rain Sensor probe
[ ] Test Buzzer warnings
[ ] Test Relay switching states

Firmware Development
[ ] Implement configuration headers (config.h, pins.h)
[ ] Build Sensor Manager module
[ ] Build Pump Controller with safety watchdogs
[ ] Build Display Manager frame update functions
[ ] Setup WiFi connection & AP fallback modes
[ ] Implement Web Server and REST API JSON serialization
[ ] Integrate Alarm Manager warnings

Web Dashboard Development
[ ] Build responsive HTML5 layout (index.html)
[ ] Implement CSS styles (style.css)
[ ] Implement JavaScript API fetches (api.js)
[ ] Build UI DOM update functions (ui.js)
[ ] Upload dashboard files to LittleFS flash partition

System Testing
[ ] Verify dashboard loads over Wi-Fi
[ ] Test manual pump controls from mobile and desktop
[ ] Test automatic pump activations based on moisture thresholds
[ ] Run failure injection tests (sensor disconnect, Wi-Fi reconnect)
[ ] Verify pump run timeout and rain lockout safety features
[ ] Run continuous loop stability check (24 hours)
```

---

## 10. Version Control & Git Strategy

*   **Branch Structure**:
    *   `main`: Holds stable production releases (e.g. `v1.0.0`).
    *   `develop`: The primary integration branch for development.
    *   `feature/*`: Feature branches for individual tasks (e.g. `feature/firmware-dht22`, `feature/web-api`).
    *   `bugfix/*`: Bugfix branches to resolve issues found during testing.
*   **Commit Message Convention**: Commit messages must follow the **Conventional Commits** standard:
    *   `feat(scope): [description]` (e.g. `feat(firmware): add sensor calibration routines`)
    *   `fix(scope): [description]` (e.g. `fix(web): resolve layout scaling issue on mobile`)
    *   `docs(scope): [description]` (e.g. `docs(api): document HTTP endpoints`)
*   **Semantic Versioning**: Standard SemVer (`MAJOR.MINOR.PATCH`) is used to version releases:
    *   `v0.1.0`: Hardware assembled, basic verification tests complete.
    *   `v0.2.0`: Local firmware control loops complete.
    *   `v0.3.0`: WiFi, web server, and REST API endpoints functional.
    *   `v1.0.0`: Web dashboard integrated, system safety tested, final release.
