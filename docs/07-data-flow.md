# 7. Data Flow & Processing Pipeline

## 7.1 Data Flow Diagram
The flow of telemetry from physical sensors through the ESP32 processing stages to the browser dashboard is shown below:

```mermaid
graph TD
    subgraph PhysicalEnvironment["Physical Environment"]
        Water["Soil Moisture permittivity"]
        Heat["Ambient Temp/Humidity"]
        Photons["Ambient Light Lux"]
        Precip["Rain Droplets"]
    end

    subgraph SensorModules["Sensor Modules"]
        SoilProbe["Capacitive Soil Probe v1.2"]
        DHT["DHT22 Sensor"]
        BH["BH1750 Lux Sensor"]
        RainProbe["Rain Sensor Board"]
    end

    subgraph ESP32Firmware["ESP32 Core Firmware"]
        ADC["ADC1 Hardware read (12-bit)"]
        OneWire["1-Wire Pulse Reader"]
        I2C["I2C Bus Master driver"]
        
        subgraph Processing["Data Processing Module"]
            Cal["Calibration & Mapping (0-100%)"]
            Smooth["Smoothing (10-sample Rolling Average)"]
            Val["Validation (Out-of-bounds Check)"]
        end

        subgraph LocalState["Unified State Struct"]
            StateTelemetry["Sensor Values Array"]
            SystemMode["Mode (AUTO / MANUAL)"]
            PumpState["Pump (ON / OFF)"]
            AlarmState["Alarms Register"]
        end

        API["REST API Router (JSON Encoder)"]
    end

    subgraph Network["Network Layer"]
        TCP["HTTP TCP Server (Port 80)"]
    end

    subgraph ClientBrowser["Client Web Browser"]
        Fetch["Fetch API (JS async poll)"]
        JSONParser["JSON Response Parser"]
        UI_Update["DOM UI Renderer (Dashboard Gauges)"]
    end

    %% Environment to sensors
    Water --> SoilProbe
    Heat --> DHT
    Photons --> BH
    Precip --> RainProbe

    %% Sensors to ESP32
    SoilProbe -->|Analog Voltage 0-3.3V| ADC
    RainProbe -->|Analog Voltage 0-3.3V| ADC
    DHT -->|Digital Pulse Train| OneWire
    BH -->|I2C Registers| I2C

    %% Internal ESP32 routing
    ADC --> Cal
    OneWire --> Cal
    I2C --> Cal
    
    Cal --> Smooth
    Smooth --> Val
    Val -->|Validated Struct| StateTelemetry
    
    StateTelemetry --> LocalState
    SystemMode --> LocalState
    PumpState --> LocalState
    AlarmState --> LocalState

    LocalState --> API
    API -->|Encode JSON Payload| TCP
    
    %% API to client
    TCP -->|HTTP Payload Response| Fetch
    Fetch --> JSONParser
    JSONParser -->|Update State Object| UI_Update
```

---

## 7.2 Telemetry Conversion Pipeline

### 1. Soil Moisture Mapping
The capacitive soil moisture probe outputs an analog voltage between 1.2V (fully saturated soil) and 2.8V (completely dry air).
*   **ADC Conversion**: The ESP32 converts this voltage into a 12-bit digital value (0 to 4095).
    *   $\text{ADC}_{\text{dry}} \approx 3100$ (corresponds to ~2.5V in dry air)
    *   $\text{ADC}_{\text{wet}} \approx 1400$ (corresponds to ~1.1V fully submerged)
*   **Linear Mapping Formula**:
    $$\text{Moisture \%} = \text{constrain}\left( \frac{\text{ADC}_{\text{dry}} - \text{ADC}_{\text{raw}}}{\text{ADC}_{\text{dry}} - \text{ADC}_{\text{wet}}} \times 100,\ 0,\ 100 \right)$$

### 2. Rain Sensor Calibration
The rain sensor probe uses a voltage comparator circuit. 
*   **ADC Range Mapping**:
    *   $\text{ADC} > 3500$: Dry (No rainfall)
    *   $1500 < \text{ADC} \le 3500$: Moderate Rain
    *   $\text{ADC} \le 1500$: Heavy Rain / Water immersion

### 3. Data Smoothing & Noise Reduction
To prevent transient voltage dips (caused by motor activations) from triggering false sensor readings, all analog inputs pass through a **rolling average filter**:
*   An array of size 10 stores the last 10 samples.
*   Every 100ms, the oldest sample is replaced with a new reading.
*   The system uses the computed average of this array for all decision logic checks.

---

## 7.3 Data Serialization (REST JSON Schema)
When requested, the communication layer encodes the active system states into a minified JSON response body:

```json
{
  "timestamp": 178263729,
  "system": {
    "mode": "AUTO",
    "pump": "OFF",
    "uptime_s": 3624,
    "wifi_rssi": -65
  },
  "sensors": {
    "moisture_pct": 42.5,
    "temperature_c": 26.8,
    "humidity_pct": 55.2,
    "light_lux": 420,
    "rain_state": "DRY"
  },
  "alarms": {
    "sensor_fault": false,
    "pump_timeout": false,
    "low_battery": false
  }
}
```
This payload is read by the dashboard's JavaScript engine to update the user interface.
