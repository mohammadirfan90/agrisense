# AgriSense Hardware Assembly & System Setup Guide

This guide details the complete hardware wiring, pin configuration, and software installation steps to assemble and run the **AgriSense Smart Irrigation System**.

---

## 1. Hardware Pin Assignment Matrix
All analog sensors connect exclusively to **ADC1** pins to prevent interference from active Wi-Fi transmissions (which disable ADC2).

| Component | Component Pin | ESP32 Pin | Signal Type | Description / Notes |
| :--- | :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | SDA | **GPIO 21** | Digital (I2C) | Shared I2C Data Line (4.7kΩ pull-up to 3.3V) |
| | SCL | **GPIO 22** | Digital (I2C) | Shared I2C Clock Line (4.7kΩ pull-up to 3.3V) |
| **BH1750 Lux Sensor**| SDA | **GPIO 21** | Digital (I2C) | Shared I2C Data Line |
| | SCL | **GPIO 22** | Digital (I2C) | Shared I2C Clock Line |
| **DHT22 Sensor** | DATA | **GPIO 19** | Digital (1-Wire) | Temp/Humidity Data (10kΩ pull-up to 3.3V) |
| **Soil Moisture Probe**| AOUT (Analog) | **GPIO 32** | Analog (ADC1_4) | Soil Moisture input voltage |
| **Rain Sensor Board** | AO (Analog) | **GPIO 33** | Analog (ADC1_5) | Precipitation level input voltage |
| **Active Buzzer** | Signal (I/O) | **GPIO 17** | Digital Output | Active high alarm control (TX2 pin) |
| **Relay Module** | Trigger (IN) | **GPIO 16** | Digital Output | Active low pump control trigger (RX2 pin) |

---

## 2. Visual Pin Map (ESP32 DevKit V1)

```text
                     +-------------------+
                     |       ESP32       |
                     |     DEVKIT V1     |
                     +-------------------+
         3.3V Power --| 3V3           GND |-- System Ground (GND)
                      | EN           GPIO23|-- (Unused)
                      | GPIO36(VP)   GPIO22|-- I2C SCL (OLED, BH1750)
                      | GPIO39(VN)   GPIO21|-- I2C SDA (OLED, BH1750)
                      | GPIO34       GPIO19|-- DHT22 Data
                      | GPIO35       GPIO18|-- (Unused)
Soil Moisture ADC ----| GPIO32       GPIO5 |-- (Strapping - Unused)
   Rain Sensor ADC ---| GPIO33       GPIO17|-- Active Buzzer Driver
                      | GPIO25       GPIO16|-- Pump Relay Trigger
                      | GPIO26       GPIO4 |-- (Unused)
                      | GPIO27       GPIO2 |-- (Strapping - Onboard LED)
                      | GPIO14       GPIO15|-- (Strapping - Unused)
                      | GND          GND   |-- Ground
                      +-----+-----+-------+
                            | VIN | 
                            +-----+
                               |
                       +5V DC Buck Power
```

---

## 3. Detailed Circuit Connection Guide

### Power Architecture
*   **Source**: A **2S Li-ion battery pack** (7.4V) powers the system.
*   **Step-Down**: Route the battery positive wire through a power switch into the **LM2596 Buck Converter (IN+)**. Set the buck potentiometer to output exactly **5.0V**.
*   **5V Rail**: Connect the buck output **OUT+** to the ESP32 **VIN** pin and the **Relay VCC** pin.
*   **GND Rail**: Connect the buck output **OUT-** to the common ground plane in a star grounding configuration.
*   **3.3V Rail**: Sensors (DHT22, Soil, Rain, BH1750, OLED) receive clean logic power from the ESP32 **3V3** output pin.

### Relay & Pump Control
*   Connect the **COM (Common)** terminal of the relay to the **+5V** power supply.
*   Connect the **NO (Normally Open)** terminal of the relay to the positive terminal of the **Mini DC Water Pump**.
*   Connect the negative terminal of the pump to **GND**.
*   Solder a **1N4007 flyback diode** in reverse-parallel directly across the DC pump motor terminals (stripe/cathode to positive, anode to negative) to suppress inductive voltage spikes.

### Buzzer Connection
*   If using a **bare 2-pin active buzzer** (like TM-1205/WT-1205), switch it via an NPN transistor (e.g. 2N2222) to avoid drawing too much current from the GPIO:
    *   Buzzer positive (`+`) $\rightarrow$ **+5V** (or **+3.3V**).
    *   Buzzer negative (`-`) $\rightarrow$ Transistor **Collector**.
    *   Transistor **Emitter** $\rightarrow$ **GND**.
    *   Transistor **Base** $\rightarrow$ ESP32 **GPIO 17** through a **1kΩ resistor**.
*   If using a **3-pin module** (with onboard transistor), connect VCC to **3V3/5V**, GND to **GND**, and Pin S to **GPIO 17** directly.

---

## 4. Software Installation Steps

### 1. Required Libraries (Install via Arduino Library Manager)
1.  **Adafruit SSD1306** (for OLED)
2.  **DHT sensor library** (by Adafruit, for DHT22)
3.  **hp_BH1750** (by Stefan, for Light Lux Sensor)
4.  **ArduinoJson** (by Benoit Blanchon, for REST API)

### 2. Upload Web Assets (LittleFS)
1.  Verify that your static files (`index.html`, `style.css`, `script.js`) are placed inside `firmware/AgriSense/data`.
2.  Open **Arduino IDE 2.x**.
3.  Press **`Ctrl` + `Shift` + `P`** to open the Command Palette.
4.  Run the command: **`Upload LittleFS to Pico/ESP8266/ESP32`**.

### 3. Flash the Firmware
1.  Under **Tools** $\rightarrow$ **Partition Scheme**, select: `"Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)"`.
2.  Click the **Upload** arrow button in the Arduino IDE toolbar to compile and flash the firmware.
3.  Open the Serial Monitor at **115200** baud to confirm the boot status and read the local IP address!
