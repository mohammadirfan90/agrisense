# 4. Pin Connections & Assignment Matrix

## 4.1 Pin Assignment Strategy
To prevent boot failures, sensor reading corruptions, and bus contentions, the GPIO layout for the AgriSense ESP32 architecture is designed around the following constraints:

*   **ADC1 Isolation**: All analog sensors are mapped exclusively to ADC1 channels. ADC2 channels (GPIOs 0, 2, 4, 12, 13, 14, 15, 25, 26, 27) are completely avoided because their analog read features are disabled when the ESP32's Wi-Fi network stack is broadcasting.
*   **Boot Strapping Avoidance**: GPIOs 0, 2, 5, 12, and 15 are excluded from output commands. If the relay (often pull-up triggered) or buzzer was wired to these pins, the voltage state during power-on would force the ESP32 into programming mode instead of executing application code.
*   **Default Hardware Bus Mapping**: The I2C bus utilizes the standard native hardware pins (GPIO 21 and GPIO 22) to enable optimal driver execution via the `Wire.h` hardware register interface.

---

## 4.2 Unified Pin Assignment Table

| ESP32 Pin | Function Name | Input/Output | Connected Component | Signal Type | Selection Rationale |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **3.3V** | VCC_3V3 | Power Out | SSD1306, BH1750, DHT22, Rain Sensor Board | DC Power | Dedicated low-noise power rail for sensitive logic electronics. |
| **GND** | GND | Power Out | All ground pins | Ground | Shared system reference ground. Connected in a star topology. |
| **5V (VIN)**| VCC_5V | Power In | LM2596 Output / Relay VCC | DC Power | Main unregulated input supplying power to ESP32 LDO and relay coils. |
| **GPIO 21** | I2C_SDA | Bidirectional | SSD1306 OLED, BH1750 Sensor | Digital (I2C) | Default ESP32 hardware I2C SDA pin. Shared bus. |
| **GPIO 22** | I2C_SCL | Output | SSD1306 OLED, BH1750 Sensor | Digital (I2C) | Default ESP32 hardware I2C SCL pin. Shared bus. |
| **GPIO 32** | ADC1_CH4 | Input | Capacitive Soil Moisture Sensor | Analog (0-3.3V)| Dedicated ADC1 input. Unaffected by active WiFi operations. |
| **GPIO 33** | ADC1_CH5 | Input | Rain Sensor Board (Analog Out) | Analog (0-3.3V)| Dedicated ADC1 input. Unaffected by active WiFi operations. |
| **GPIO 19** | DHT_DATA | Input | DHT22 Temperature/Humidity | Digital (1-Wire)| Safe GPIO with no strapping dependencies or hardware conflicts. |
| **GPIO 16** | PUMP_RELAY | Output | 1-Channel Relay Trigger | Digital (GPIO)| General-purpose output pin (RX2 pin). Floats at boot, safe for actuators. |
| **GPIO 17** | BUZZER_PIN | Output | Active Buzzer Control | Digital (GPIO)| General-purpose output pin (TX2 pin). Floats at boot, safe for alarms. |

---

## 4.3 Pin Map Diagram (ESP32 DevKit V1)

```
                     +-------------------+
                     |       ESP32       |
                     |     DEVKIT V1     |
                     +-------------------+
        3.3V Power --| 3V3           GND |-- System Ground
                     | EN           GPIO23|-- (Unused)
  (Unused Analog) ---| GPIO36(VP)   GPIO22|-- I2C SCL (OLED, BH1750)
  (Unused Analog) ---| GPIO39(VN)   GPIO21|-- I2C SDA (OLED, BH1750)
  (Unused Analog) ---| GPIO34       GPIO19|-- DHT22 Digital Data
  (Unused Analog) ---| GPIO35       GPIO18|-- (Unused SPI CLK)
Soil Moisture ADC ---| GPIO32       GPIO5 |-- (Strapping - Unused)
  Rain Sensor ADC ---| GPIO33       GPIO17|-- Active Buzzer
         (Unused) ---| GPIO25       GPIO16|-- Pump Relay Trigger
         (Unused) ---| GPIO26       GPIO4 |-- (Unused)
         (Unused) ---| GPIO27       GPIO2 |-- (Strapping - Onboard LED)
         (Unused) ---| GPIO14       GPIO15|-- (Strapping - Unused)
         (Unused) ---| GPIO12       GND   |-- System Ground
                     | GND          GPIO13|-- (Unused)
      5V DC Power ---| VIN          RX0   |-- (Unused UART Programming)
                     +-------------------+
```
