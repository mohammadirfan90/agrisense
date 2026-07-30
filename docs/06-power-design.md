# 6. Power Design & Distribution

## 6.1 Power Architecture Overview

The system features a centralized buck-regulated battery power system designed to handle the high current demands of Wi-Fi transmissions and the inductive load of the water pump.

```
       [ 2S Li-ion Battery Pack ] (7.4V Nominal, 8.4V Max)
                   |
             [ Power Switch ] (SPST, 3A Rated)
                   |
      +------------+------------+
      |  LM2596 Buck Converter  | (Step-down to 5.0V DC)
      +------------+------------+
                   |
           [ 5V Power Rail ]
                   |
         +---------+---------+-------------------+
         |                   |                   |
    [ ESP32 VIN ]     [ Relay Coil VCC ]   [ Pump COM Node ]
    (Onboard LDO)            |                   |
         |             Relay Trigger       Relay Contact (NO)
   [ 3.3V Rail ]             |                   |
         |                   |             [ DC Pump VCC ]
   Sensors & OLED            |                   |
         |                   |             1N4007 Diode
         |                   |                   |
  +------+-------------------+-------------------+------+
  |                                                     |
  +------------------[ Star Ground ]--------------------+
```

---

## 6.2 Electrical Power Budget

This budget details current requirements under both typical monitoring and peak activation loads:

| Sub-System / Component | Operating Voltage | Typ. Current (Idle) | Peak Current | Power Source |
| :--- | :--- | :--- | :--- | :--- |
| **ESP32 DevKit V1** | 5.0V (VIN) | 120.0 mA (WiFi on) | 300.0 mA (WiFi Burst) | LM2596 Output |
| **SSD1306 OLED** | 3.3V (LDO) | 15.0 mA | 25.0 mA | ESP32 3.3V Pin |
| **BH1750 Lux Sensor** | 3.3V (LDO) | 1.0 mA | 2.0 mA | ESP32 3.3V Pin |
| **DHT22 Sensor** | 3.3V (LDO) | 1.0 mA | 2.5 mA | ESP32 3.3V Pin |
| **Rain Sensor Board** | 3.3V (LDO) | 10.0 mA | 15.0 mA | ESP32 3.3V Pin |
| **1-Channel Relay Coil**| 5.0V | 0.0 mA (De-energized) | 80.0 mA (Energized) | LM2596 Output |
| **Active Buzzer** | 3.3V (LDO) | 0.0 mA | 30.0 mA | ESP32 3.3V Pin |
| **Mini DC Water Pump** | 5.0V | 0.0 mA (Inactive) | 1000.0 mA (Start/Stall) | LM2596 Output |

### Total Power Calculations:
*   **System Idle (Monitoring, Pump OFF)**:
    $$\text{I}_{\text{idle}} = 120\text{ mA} + 15\text{ mA} + 1\text{ mA} + 1\text{ mA} + 10\text{ mA} = 147\text{ mA}$$
    $$\text{Power}_{\text{idle}} = 5.0\text{V} \times 147\text{ mA} = 0.735\text{ W}$$
*   **Active Irrigation (Pump ON, Buzzer Active, WiFi Transmitting)**:
    $$\text{I}_{\text{peak}} = 300\text{ mA} (ESP32) + 44.5\text{ mA} (3.3V Rail) + 80\text{ mA} (Relay) + 1000\text{ mA} (Pump Start) = 1424.5\text{ mA} (1.42\text{ A})$$
    $$\text{Power}_{\text{peak}} = 5.0\text{V} \times 1.42\text{ A} = 7.10\text{ W}$$

---

## 6.3 Battery Sizing & Battery Life Expectancy

To satisfy the LM2596 buck converter's voltage drop constraints, we use a **2S Li-ion battery configuration**:
*   **Chemistry**: Lithium-Ion (18650 Cells x 2 in series).
*   **Nominal Pack Voltage**: $3.7\text{V} \times 2 = 7.4\text{V}$ (Charge range: $6.0\text{V}$ fully discharged to $8.4\text{V}$ fully charged).
*   **Cell Capacity Rating**: $2500\text{ mAh}$.
*   **Total Available Energy**:
    $$\text{Total Energy} = 7.4\text{V} \times 2500\text{ mAh} = 18.5\text{ Wh}$$

### Battery Run-Time Estimation (Assuming 10 watering cycles of 30 seconds per day):
*   Daily active time: 300 seconds ($0.083\text{ hours}$).
*   Daily idle time: 86,100 seconds ($23.917\text{ hours}$).
*   Daily energy consumption:
    $$\text{Energy}_{\text{idle}} = 0.735\text{ W} \times 23.917\text{ h} = 17.58\text{ Wh}$$
    $$\text{Energy}_{\text{active}} = 7.10\text{ W} \times 0.083\text{ h} = 0.59\text{ Wh}$$
    $$\text{Energy}_{\text{daily\_total}} = 17.58\text{ Wh} + 0.59\text{ Wh} = 18.17\text{ Wh}$$
*   Expected Battery Life on a single charge:
    $$\text{Lifespan} = \frac{18.5\text{ Wh}}{18.17\text{ Wh/day}} \approx 1.02\text{ days}$$
*   **Architectural Note**: To extend battery life, a production version should use the ESP32's **Deep Sleep** mode. The ESP32 would wake up every 15 minutes, read sensors, update the display, water if necessary, send data over Wi-Fi, and go back to sleep. This would reduce the average idle current from 120mA to under 5mA, extending battery life to over 20 days.

---

## 6.4 Grounding & Power Distribution Layout

*   **Star Grounding Protocol**: Analog sensors (Soil Moisture and Rain Sensor) are sensitive to ground noise. The motor pump draws heavy, noisy current. To prevent motor noise from corrupting sensor readings, the system uses a star grounding layout:
    *   Separate ground return paths are run for the DC Pump/Relay coil and the ESP32/Sensors.
    *   These lines connect at a single point: the **OUT-** terminal of the LM2596 buck converter. This prevents motor return currents from creating voltage offsets on the analog sensor ground lines.
*   **Decoupling Capacitors**:
    *   A **100µF electrolytic capacitor** is placed across the 5V power terminals of the relay and pump to act as a local power reservoir during motor startup.
    *   A **0.1µF ceramic capacitor** is placed in parallel to filter high-frequency noise from the DC motor brushes.

---

## 6.5 Power Safety Features

*   **Reverse-Polarity Protection**: A Schottky diode (e.g. 1N5822, 3A rated) is wired in series with the battery switch to prevent circuit damage if the cells are inserted backwards.
*   **Over-Current Fusing**: A **2A fast-blow fuse** is placed on the battery positive line to protect the cells from discharging too quickly in the event of a short circuit.
