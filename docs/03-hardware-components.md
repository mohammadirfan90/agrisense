# 3. Hardware Components & Compatibility Review

## 3.1 Component Rationale & System Interaction

### Main Controller: ESP32 DevKit V1
*   **Why It Is Used**: Features a dual-core Tenesilica Xtensa 32-bit LX6 processor running at 240MHz, 520KB SRAM, 4MB built-in Flash, integrated 2.4GHz Wi-Fi (802.11 b/g/n), Bluetooth, and a wide array of peripherals including I2C, SPI, and multi-channel ADCs.
*   **System Interaction**: Functions as the central controller. It polls all environmental sensors, executes the local decision logic state machine, updates the OLED screen, switches the relay, hosts the SPIFFS/LittleFS file system, and runs the web server to expose JSON REST endpoints.

### DHT22 Temperature & Humidity Sensor
*   **Why It Is Used**: Offers superior accuracy and measurement ranges compared to the DHT11 (Temperature: -40 to 80°C ±0.5°C; Humidity: 0-100% ±2%). Uses a single-wire digital interface.
*   **System Interaction**: Periodically sampled by the firmware to log environmental ambient context. Used in the decision loop to scale the base soil moisture threshold (e.g. raise target soil moisture in high heat environments).

### Capacitive Soil Moisture Sensor (v1.2)
*   **Why It Is Used**: Resists corrosion unlike resistive soil moisture probes, since the metallic electrodes are fully insulated with protective solder mask/FR4 laminate.
*   **System Interaction**: Generates an analog voltage output corresponding to the dielectric permittivity of the soil (which increases with water content). Connected to a dedicated ESP32 ADC1 pin to calculate the soil hydration percentage.

### BH1750 Light Intensity Sensor
*   **Why It Is Used**: Provides direct digital Lux readings (1-65535 lx) via I2C. Does not require external analog calibration and is highly sensitive.
*   **System Interaction**: Connects to the shared I2C bus. Ambient light measurements are monitored to prevent midday watering (which causes rapid evaporation and potential root shock).

### Rain Sensor Module (LM393 Comparator + Foil Probe)
*   **Why It Is Used**: Provides analog rainfall intensity detection and a digital rain-present threshold signal.
*   **System Interaction**: Monitored by the ESP32. When rain is detected, the decision engine triggers a "Rain Lockout" to prevent the water pump from turning on, even if the soil moisture is below the activation threshold.

### 0.96" SSD1306 OLED Display (128x64 pixels)
*   **Why It Is Used**: Compact, self-illuminating display (no backlight needed, low current draw ~20mA) with high contrast. Interfaces over I2C.
*   **System Interaction**: Shares the I2C bus with the BH1750. Renders local state readouts: WiFi connection state, DHCP-allocated IP address, current sensor readings, active alarms, and pump runtime metrics.

### 1-Channel Opto-Isolated Relay Module
*   **Why It Is Used**: Isolates the delicate 3.3V ESP32 microcontroller logic circuit from the high-current 5V inductive pump load.
*   **System Interaction**: Receives a digital control trigger from an ESP32 GPIO. Toggles the high-current power connection to the water pump.

### Mini DC Water Pump (Submersible)
*   **Why It Is Used**: Direct current 3-6V submersible pump, moving up to 120 liters/hour. Compact, low cost, and draws ~300mA running.
*   **System Interaction**: The mechanical actuator of the irrigation loop. Connected to the relay contacts. Submerged in the water reservoir to feed water tubing to the soil.

### Active Buzzer
*   **Why It Is Used**: Generates an immediate, audible warning tone (~2.3kHz) when driven HIGH.
*   **System Interaction**: Driven by an ESP32 GPIO. Sounds alert sequences during startup failures, sensor disconnects, or pump run timeouts.

---

## 3.2 Hardware Compatibility Review

A rigorous architectural analysis was conducted to evaluate the compatibility of the selected components with the ESP32 platform:

| Check # | Compatibility Area | Findings | Resolution / Design Decision |
| :--- | :--- | :--- | :--- |
| **1** | **GPIO Availability** | ESP32 has 25 usable GPIOs. The architecture requires only 7 GPIO pins (1x DHT, 1x Soil ADC, 2x I2C, 1x Rain ADC, 1x Relay, 1x Buzzer). | **Compatible**. The system has ample unused GPIOs available for future expansions. |
| **2** | **ADC Pin Conflict** | ESP32 ADC2 pins cannot be read while Wi-Fi is active. The WiFi stack overrides the ADC2 hardware registers. | **Mitigated**. Analog sensors (Moisture and Rain) are strictly routed to ADC1 pins (GPIO32 and GPIO33). |
| **3** | **Boot Strapping Pins** | GPIOs 0, 2, 5, 12, and 15 affect the ESP32 boot configuration. Attaching pull-ups, pull-downs, or low-impedance relays to these pins can block boot. | **Mitigated**. Outputs (Relay and Buzzer) are routed to safe, neutral GPIOs (GPIO16 and GPIO17) which float at boot. |
| **4** | **I2C Shared Bus** | SSD1306 OLED and BH1750 operate on I2C. The SSD1306 address is `0x3C`; the BH1750 is `0x23`. No address collision exists. | **Compatible**. Both share standard ESP32 I2C pins (SDA=GPIO21, SCL=GPIO22) with a single pair of 4.7kΩ pull-up resistors. |
| **5** | **Relay Logic Shift** | Standard 5V relay coils draw ~80mA. ESP32 GPIO outputs max 3.3V logic up to 40mA. A direct GPIO wire may fail to trigger the optocoupler. | **Mitigated**. Design utilizes a 3.3V logic compatible relay module containing an onboard transistor buffer or optoisolator with active-low trigger. |
| **6** | **Inductive Pump Kickback** | DC Pump is an inductive load. Collapsing magnetic fields upon shutdown induce a massive reverse-voltage spike (up to 100V). | **Mitigated**. A **1N4007 flyback diode** is wired in reverse-parallel directly across the pump motor terminals. |
| **7** | **Battery step-down** | The buck converter (LM2596) requires an input voltage at least 1.5V-2V higher than the desired 5V output. | **Mitigated**. A single 18650 cell (3.7V nominal) is rejected. The design mandates a **2S Li-ion battery pack (7.4V)** or 3S pack. |
| **8** | **ESP32 LDO Budget** | Drawing excessive 3.3V current from the ESP32 onboard regulator will cause thermal shutdown. | **Compatible**. Combined load of display and sensors on 3.3V rail is ~40mA, well below the ESP32 LDO's 500mA rating. |

---

## 3.3 Component Responsibility Matrix

```
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| Component Name   | Primary Purpose              | Inputs                    | Outputs                   | Dependencies           |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| ESP32 DevKit V1  | Central processing & logic   | • 5V DC Vin               | • 3.3V DC (Sensors)       | Stable 5V DC supply    |
|                  | controller.                  | • Raw Sensor Telemetry    | • Relay digital output    |                        |
|                  |                              | • REST HTTP client command| • OLED I2C signals        |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| DHT22 Sensor     | Monitor ambient temperature  | • 3.3V DC power           | • Digital pulse train     | DHT Sensor Library     |
|                  | and relative humidity.       |                           | (1-Wire Protocol)         |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| Capacitive Soil  | Measure dielectric constant | • 3.3V DC power           | • Analog voltage (0-3.3V) | ESP32 ADC1 calibration  |
| Moisture Sensor  | of surrounding soil.         |                           |                           |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| BH1750 Sensor    | Measure ambient light        | • 3.3V DC power           | • I2C digital word        | Wire.h / BH1750 Library|
|                  | intensity in Lux.            | • I2C Clock (SCL)         | (Lux units)               |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| Rain Sensor      | Detect active precipitation  | • 3.3V DC power           | • Analog voltage (0-3.3V) | ADC1 channel read      |
|                  | and storm events.            |                           |                           |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| SSD1306 OLED     | Local visual output for      | • 3.3V DC power           | • Graphic visual frames   | Adafruit SSD1306/GFX   |
|                  | telemetry & network data.    | • I2C Bus signals         |                           | libraries              |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| Relay Module     | Electrical isolation switch  | • 5V DC Power (Coil)      | • SPDT contact closure    | ESP32 GPIO drive (16)  |
|                  | for high current load.       | • Digital control (3.3V)  | (Toggles pump power loop) |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| Mini DC Pump     | Physical fluid displacement  | • 5V DC Power             | • Pressurized water flow  | Relay contact close    |
|                  | (watering actuator).         |                           |                           | Reservoir water supply |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
| Active Buzzer    | Produce audible warning      | • 3.3V/5V DC power        | • 2.3kHz acoustic wave    | ESP32 GPIO drive (17)  |
|                  | tones for alarms.            | • Digital GPIO drive      |                           |                        |
+------------------+------------------------------+---------------------------+---------------------------+------------------------+
```
