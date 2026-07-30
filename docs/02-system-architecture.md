# 2. System Architecture & Workflows

## 2.1 Software Architecture Layers
The AgriSense firmware is structured in five decoupled software layers to ensure maintainability, testability, and isolated component responsibilities.

```
+-------------------------------------------------------------+
|                     Dashboard Layer (UI)                    |
|                HTML5 / CSS3 / Vanilla JavaScript            |
+-------------------------------------------------------------+
                              | REST API (JSON)
+-------------------------------------------------------------+
|                  Communication Layer                        |
|       WebServer / WiFiManager / REST API Endpoints          |
+-------------------------------------------------------------+
                              | Reads/Writes State
+-------------------------------------------------------------+
|                   Business Logic Layer                      |
|         Decision Engine / AlarmManager / State Machine       |
+-------------------------------------------------------------+
                              | Commands Drivers
+-------------------------------------------------------------+
|                     Driver / Hardware Layer                 |
| SensorManager / PumpController / DisplayManager (I2C/GPIO)  |
+-------------------------------------------------------------+
                              | Unified Pins & Defines
+-------------------------------------------------------------+
|                  Configuration Layer                        |
|                     config.h / pins.h                       |
+-------------------------------------------------------------+
```

### Layer Responsibilities
1.  **Configuration Layer (`config.h`, `pins.h`)**: Centralized compilation constants. Declares physical ESP32 pin bindings, sensor calibration values, auto-mode moisture thresholds, default WiFi credentials, and safety timers.
2.  **Driver Layer (`sensor_manager`, `pump_controller`, `display_manager`)**: Abstracted hardware interaction. Wraps sensor libraries (DHT, BH1750), handles non-blocking analog reading smoothing (rolling averages), operates physical relay GPIOs, and manages SSD1306 frame rendering.
3.  **Business Logic Layer (`alarm_manager`, State Machine)**: Decides state transitions. Validates sensor ranges, compares current soil moisture against thresholds, monitors pump running durations, tracks system safety states, and triggers alarm behaviors.
4.  **Communication Layer (`wifi_manager`, `web_server`)**: Manages external network interfaces. Establishes station connection or boots local Access Point fallback, processes incoming REST HTTP requests, parses incoming control variables, and sends JSON response bodies.
5.  **Dashboard Layer (`index.html`, `style.css`, `script.js`)**: Executes in client browser. Performs AJAX polling of ESP32 REST API, renders live gauges, provides interactive toggles, updates network status indicators, and logs local alarm events.

---

## 2.2 System Architecture Diagram
The flow of telemetry, network commands, and output control across the hardware boundary is visualised below:

```mermaid
graph TD
    subgraph BrowserClient["Browser Client (Mobile/PC)"]
        UI["Web Dashboard UI"]
        AJAX["REST Client (JS Fetch)"]
    end

    subgraph Network["Network Layer"]
        WiFi["Wi-Fi connection (Station / AP)"]
    end

    subgraph ESP32["ESP32 Microcontroller"]
        WebServer["REST HTTP Server"]
        Config["Configuration & Pins"]
        
        subgraph Logic["Core Firmware Engine"]
            SM["State Machine"]
            DE["Decision Engine"]
            AM["Alarm Manager"]
        end

        subgraph Drivers["Driver Layer"]
            Sensors["Sensor Manager (DHT22, Soil, Rain, BH1750)"]
            Pump["Pump Controller (Relay Drive)"]
            OLED["Display Manager (SSD1306)"]
        end
    end

    subgraph HardwareOutputs["Physical Hardware Outputs"]
        Relay["1-Channel Opto-isolated Relay"]
        Motor["Mini DC Water Pump"]
        Buzzer["Active Buzzer"]
        Screen["0.96 OLED Display"]
    end

    subgraph HardwareInputs["Physical Sensors"]
        DHT["DHT22 (Temp & Hum)"]
        Soil["Capacitive Soil Probe"]
        Rain["Rain Sensor Board"]
        Light["BH1750 Lux Sensor"]
    end

    %% Wiring connections
    UI <=> AJAX
    AJAX <=>|HTTP Requests / JSON| WiFi
    WiFi <=>|TCP Port 80| WebServer
    WebServer <=>|Read Status / Write Config| SM
    SM === DE
    DE === AM
    DE -->|Control Signals| Pump
    DE -->|Render Frames| OLED
    Sensors -->|Raw Telemetry| DE
    
    %% Drivers to physical components
    Pump -->|GPIO Control| Relay
    Relay -->|DC Power Switching| Motor
    AM -->|GPIO PWM/High| Buzzer
    OLED -->|I2C SDA/SCL| Screen
    
    %% Physical sensors to drivers
    DHT -->|Digital Input| Sensors
    Soil -->|Analog ADC1| Sensors
    Rain -->|Analog ADC1| Sensors
    Light -->|I2C SDA/SCL| Sensors
```

---

## 2.3 System Workflow
The sequential execution sequence after power delivery is illustrated below:

```mermaid
sequenceDiagram
    autonumber
    participant HW as Hardware / Sensors
    participant Boot as ESP32 Bootloader
    participant Core as Core Firmware Engine
    participant Comm as Wifi & WebServer
    participant Client as Browser Dashboard

    HW->>Boot: Power Applied
    Boot->>Core: Launch Setup Loop
    activate Core
    Core->>HW: Init I2C, OLED, GPIO Pins
    Core->>HW: Probe Sensors (DHT22, BH1750, Moisture)
    alt Sensors Fail
        Core->>HW: Sound Buzzer, Show Error on OLED
        Core->>Core: Enter ERROR State
    else Sensors OK
        Core->>Comm: Init WiFi Station Mode
        activate Comm
        alt WiFi STA Connection Fails (Timeout 15s)
            Comm->>Core: Fallback requested
            Core->>Comm: Launch WiFi Access Point (AP)
            Comm->>HW: Update OLED: "AP Mode: AgriSense_AP"
        else WiFi Connected
            Comm->>HW: Update OLED with Station IP Address
        end
        Core->>Comm: Start Web Server (Port 80)
    end
    deactivate Core

    Note over Core, Comm: Run loop starts (non-blocking)

    loop Every 10ms (Core Loop)
        Core->>HW: Check Watchdog Timer (Reset WDT)
        alt Every 500ms
            Core->>HW: Sample Rain Sensor (ADC1)
        end
        alt Every 1000ms
            Core->>HW: Sample Soil Moisture (ADC1) & BH1750 Lux (I2C)
            Core->>Core: Compute Rolling Average (Smoothing)
            Core->>HW: Update SSD1306 Display Frame
        end
        alt Every 2000ms
            Core->>HW: Sample DHT22 (Temp/Humid)
        end
        
        Core->>Core: Run Decision Engine (Evaluate Thresholds)
        alt Mode == AUTO
            alt Moisture < Threshold AND Rain == DRY
                Core->>HW: Drive Relay HIGH (Pump ON)
                Core->>Core: Start Pump Watchdog Timer
            else Moisture >= Target OR Rain == WET
                Core->>HW: Drive Relay LOW (Pump OFF)
                Core->>Core: Reset Pump Watchdog Timer
            end
        end

        Comm->>Client: Handle REST requests (GET /api/status)
        Client->>Comm: AJAX UI Poll
        Comm-->>Client: Return JSON Telemetry
    end
```

---

## 2.4 System State Machine
The system operates within defined states to ensure predictable behavior, especially during hardware faults or manual intervention:

```mermaid
stateDiagram-v2
    [*] --> BOOT : Power Applied
    
    state BOOT {
        [*] --> HW_INIT : Mount Flash (LittleFS)
        HW_INIT --> SENSOR_TEST : Verify I2C Bus & Pin Directions
        SENSOR_TEST --> WIFI_CONNECT : Hardware OK
        SENSOR_TEST --> ERROR_STATE : Sensor Missing / I2C Bus Locked
    }

    state WIFI_CONNECT {
        [*] --> STA_MODE : Attempt Station Connection
        STA_MODE --> SERVER_START : Connected
        STA_MODE --> AP_MODE : Timeout (15s)
        AP_MODE --> SERVER_START : AP Broadcast Active
    }

    SERVER_START --> MONITORING : Server Running
    
    state MONITORING {
        [*] --> IDLE
        IDLE --> AUTO_IRRIGATION : Moisture < Threshold (AUTO Mode)
        IDLE --> MANUAL_CONTROL : API Command Received (MANUAL Mode)
        IDLE --> ERROR_STATE : Sensor Disconnect / Out of Range
    }

    state AUTO_IRRIGATION {
        [*] --> RUNNING : Pump Activated
        RUNNING --> IDLE : Moisture >= Wet Target
        RUNNING --> IDLE : Rain Detected (Rain Lockout)
        RUNNING --> ERROR_STATE : Continuous Run Timeout (>30s)
    }

    state MANUAL_CONTROL {
        [*] --> MANUAL_RUN : Pump Forced ON
        MANUAL_RUN --> IDLE : Pump Forced OFF (API)
        MANUAL_RUN --> IDLE : Mode Switched to AUTO
        MANUAL_RUN --> ERROR_STATE : Manual Run Limit Exceeded (>30s)
    }

    state ERROR_STATE {
        [*] --> SAFE_SHUTDOWN : Deactivate Pump Relay
        SAFE_SHUTDOWN --> BUZZER_WARN : Sound Warning Tone
        BUZZER_WARN --> RECOVERY_ATTEMPT : Wait 10s
        RECOVERY_ATTEMPT --> BOOT : Software Restart (ESP.restart)
    }
    
    ERROR_STATE --> [*] : Complete Shutdown
```
