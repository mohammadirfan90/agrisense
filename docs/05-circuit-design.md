# 5. Circuit Design & Signal Flow

## 5.1 System Wiring Specifications

The physical implementation of the AgriSense system requires logical partitioning of signal lines, high-frequency digital lines, and high-power supply paths.

```
                                          +-------------------------+
                                          |   2S Li-ion Battery     |
                                          |      (7.4V DC)          |
                                          +-------------------------+
                                                       |
                                                  Power Switch
                                                       |
                                          +-------------------------+
                                          |  LM2596 Buck Converter  |
                                          |     (Steps to 5V)       |
                                          +-------------------------+
                                            |                    |
                                         5V Rail              5V Rail
                                            |                    |
                                     +-------------+      +-------------+
                                     |  ESP32 VIN  |      |  Relay VCC  |
                                     +-------------+      +-------------+
                                            |                    |
                                         3.3V LDO                |
                                            |                    |
             +------------------------------+                    |
             |       |        |             |                    |
             |       |        |             |                    |
          DHT22   OLED    BH1750   Moisture & Rain               |
          (VCC)   (VCC)   (VCC)      Sensor VCC                  |
             |       |        |             |                    |
          Digital   I2C      I2C         Analog                  |
            Bus     Bus      Bus          Lines                  |
             |       |        |             |                    |
             +-------+--------+-------------+                    |
                           |                                     |
                    +-------------+                             /  Relay
                    | ESP32 GPIOs |----------------------------|   Contacts
                    +-------------+                             \  (SPDT)
                           |                                     |
                       Buzzer Pin                                |
                           |                                +--------+
                     Active Buzzer                          | DC Pump|
                                                            +--------+
                                                                 |
                                                           Flyback Diode
                                                              (1N4007)
```

### 1. Power Supply Wiring
*   **Battery to Buck Input**: The positive terminal of the 2S Li-ion battery pack (7.4V) passes through the single-pole single-throw (SPST) power switch to the IN+ terminal of the LM2596 buck converter module. The negative battery terminal connects directly to the IN- terminal.
*   **Buck Output to System**: The LM2596 potentiometer is adjusted to output a stable 5.0V. The OUT+ pin is routed to the 5V power bus, which distributes power to the ESP32 `VIN` pin and the Relay VCC terminal. The OUT- pin is connected to the common system Ground (GND) plane.

### 2. Sensor and Display Connections
*   **Digital I2C Bus**: SSD1306 OLED and BH1750 Light Sensor share the I2C bus. 
    *   `SDA` pins of both components connect to ESP32 **GPIO 21**.
    *   `SCL` pins of both components connect to ESP32 **GPIO 22**.
    *   Pull-up resistors (4.7kΩ) are connected between the SDA/SCL lines and the 3.3V power rail to ensure clean square-wave rises.
*   **Analog Sensors**:
    *   **Capacitive Soil Moisture Sensor**: Connects `VCC` to ESP32 **3.3V**, `GND` to common ground, and `AOUT` (Analog Out) to ESP32 **GPIO 32**.
    *   **Rain Sensor Module**: Connects `VCC` to ESP32 **3.3V**, `GND` to common ground, and `AO` (Analog Out) to ESP32 **GPIO 33**. The digital threshold pin (`DO`) is left disconnected to minimize pin usage.
*   **Temperature & Humidity (DHT22)**: Connects `VCC` to ESP32 **3.3V**, `GND` to common ground, and `DATA` to ESP32 **GPIO 19**. A 10kΩ pull-up resistor is placed between `DATA` and `3.3V` to stabilize the custom 1-wire protocol.

### 3. Actuator Control Output
*   **Buzzer**: Connects `VCC` to ESP32 **3.3V**, `GND` to common ground, and the signal input to ESP32 **GPIO 17**.
*   **Relay Module**:
    *   `VCC` connects to the **5V** supply rail (drawn directly from the buck converter to avoid overloading the ESP32 internal regulator).
    *   `GND` connects to common ground.
    *   `IN` (Trigger input) connects to ESP32 **GPIO 16**.
*   **Water Pump Control Loop**:
    *   The positive line of the 5V power supply connects to the **Common (COM)** terminal of the relay module.
    *   The **Normally Open (NO)** terminal of the relay module connects to the positive wire of the Mini DC Water Pump.
    *   The negative wire of the pump connects to the system ground.
    *   When the relay activates, COM connects to NO, closing the loop and powering the pump.
    *   A **1N4007 flyback diode** is soldered in reverse across the pump motor terminals (cathode to positive, anode to negative) to clamp inductive voltage spikes when the relay switches off.

---

## 5.2 Signal Flow Paths

The system utilizes three parallel signal pathways:

```
[Raw Physical Environment]
       |
       +--> (Temp/Humidity Changes) --> DHT22 --> (Digital Pulses) --> ESP32 GPIO 19
       +--> (Moisture Permittivity) --> Soil Probe -> (Analog Volts) -> ESP32 GPIO 32 (ADC1)
       +--> (Precipitation Water)   --> Rain Probe -> (Analog Volts) -> ESP32 GPIO 33 (ADC1)
       +--> (Photon Intensity)      --> BH1750 ---> (I2C Commands) -> ESP32 GPIO 21/22
```

1.  **Analog Sensor Input Flow**: Environmental changes alter the sensor capacitance (soil moisture) or resistance (rain probe). The sensor boards translate this into an analog voltage (0 to 3.3V). The ESP32 internal 12-bit Successive Approximation Register (SAR) ADC converts this voltage into a digital range from 0 to 4095. Software maps this to a 0-100% value.
2.  **I2C Bus Communication Flow**: The ESP32 functions as the I2C Master. It sends start conditions and the device address (e.g., `0x23` for BH1750). The sensor responds as a slave by sending data registers containing light measurement words. The OLED receives display buffers using I2C page-addressing commands.
3.  **Relay Switching Control Flow**: When the firmware triggers irrigation:
    *   GPIO 16 drives the optocoupler inside the relay module.
    *   The internal infrared LED lights up, triggering the photo-transistor.
    *   This switches the 5V coil circuit, creating a magnetic field in the relay core.
    *   The armature is pulled, closing the high-current 5V contact.
    *   Current flows to the pump, which runs until GPIO 16 is driven back to its inactive state.
