# 9. Testing & Quality Assurance Plan

## 9.1 Testing Strategy Overview
To ensure system stability, the testing plan spans five progressive levels from isolated code units to physical hardware-in-the-loop validation:

```
[ Unit Testing ] -> [ Driver Validation ] -> [ Integration ] -> [ System Safety ] -> [ Acceptance ]
```

---

## 9.2 Verification Testing Levels

### 1. Software Unit Testing
*   **Target**: Logic classes and algorithms that do not have physical pin dependencies (e.g. Rolling Average filter, state transition logic, JSON string generator).
*   **Methodology**: Run tests using a desktop framework (like platformio-test or Native C++ desktop compilation) to verify code execution without flashing to the ESP32.
*   **Verification Criteria**:
    *   Rolling average computes mathematical mean correctly for known arrays.
    *   Analog mapping formulas constrain outputs strictly between 0% and 100%.

### 2. Driver & Hardware Validation
*   **Target**: Physical communication interfaces (I2C, 1-Wire, ADC).
*   **Methodology**:
    *   **I2C Scanner Sweep**: Compile and run a basic diagnostic sketch to verify that the SSD1306 OLED (address `0x3C`) and BH1750 (address `0x23`) are detected on the bus.
    *   **Serial Plotter Calibration**: Read raw analog values from the capacitive soil probe in dry air and fully submerged in water. Update the calibration bounds (`ADC_DRY`, `ADC_WET`) in `config.h` based on these measurements.
    *   **DHT22 Signal Test**: Verify that the sensor is reading correctly and not returning `NaN` values.

### 3. Integration Testing
*   **Target**: Coordination between the firmware and Web dashboard.
*   **Methodology**:
    *   **API Verification**: Use tools like Postman, curl, or standard web browser debuggers to call `/api/status` and verify that the JSON schema matches the specifications in the API document.
    *   **State Control Test**: Call `/api/mode` to switch the system between `AUTO` and `MANUAL` modes, verifying that the OLED screen and dashboard reflect this mode change within 200ms.
    *   **Manual Relay Switch Test**: Send a POST request to `/api/pump` in `MANUAL` mode and verify that the relay switches on and off.

### 4. System-Level Safety Checks (Failure Injection)
*   **Target**: Safety controls and error recovery.
*   **Methodology**:
    *   **Sensor Disconnect Test**: Physically disconnect the DHT22 or capacitive moisture probe data lines during operation. Verify that the buzzer sounds, the OLED shows a sensor fault warning, and the relay turns off the pump immediately.
    *   **WiFi Recovery Test**: Turn off the target Wi-Fi access point during operation. Verify that the ESP32 automatically switches to Local Access Point (AP) mode within 15 seconds to allow manual local control.
    *   **Pump Run Timeout Test**: Place the capacitive moisture probe in dry air, put the pump in a container without water, and set the system to `AUTO` mode. Verify that the pump automatically shuts off after 30 seconds of continuous run, sounds the buzzer, and enters the `ERROR` state.
    *   **Rain Lockout Test**: Set the system to `AUTO` mode and place the soil moisture probe in dry soil (triggering irrigation). Pour water onto the rain sensor probe. Verify that the pump immediately shuts off and stays off.

### 5. User Acceptance & Performance Verification
*   **Target**: User Interface responsiveness and overall system performance.
*   **Methodology**:
    *   **Responsive UI Sweep**: Load the dashboard on multiple devices (iOS mobile, Android tablet, Chrome desktop, Safari). Verify that the dashboard layouts scale correctly, text remains readable, and controls are easily clickable.
    *   **Low-Battery Performance Check**: Gradually lower the input voltage from the buck converter to simulate battery discharge. Verify that the ESP32 continues to operate down to 4.5V, and that the buzzer sounds a low-battery warning.
