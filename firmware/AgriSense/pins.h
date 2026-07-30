#ifndef PINS_H
#define PINS_H

/**
 * @file pins.h
 * @brief Unified hardware GPIO assignments for AgriSense on ESP32 DevKit V1.
 * @details Prevents pin conflicts (I2C sharing, bootstrapping interference, ADC2-WiFi block).
 */

// =============================================================================
// I2C Bus Settings (Shared)
// =============================================================================
constexpr int I2C_SDA_PIN = 21;  ///< OLED and BH1750 shared data line
constexpr int I2C_SCL_PIN = 22;  ///< OLED and BH1750 shared clock line

// =============================================================================
// Analog Sensors (ADC1 only - safe when Wi-Fi is transmitting)
// =============================================================================
constexpr int SOIL_ADC_PIN = 32; ///< Capacitive Moisture Sensor (ADC1 Channel 4)
constexpr int RAIN_ADC_PIN = 33; ///< Rain Sensor board analog output (ADC1 Channel 5)

// =============================================================================
// Digital Sensors
// =============================================================================
constexpr int DHT_DATA_PIN = 19; ///< DHT22 Single-wire digital data line

// =============================================================================
// Digital Actuators (Outputs)
// =============================================================================
constexpr int PUMP_RELAY_PIN = 16; ///< Relay control pin (RX2 - safe, floats at boot)
constexpr int BUZZER_CTRL_PIN = 17; ///< Active buzzer control pin (TX2 - safe, floats at boot)

#endif // PINS_H
