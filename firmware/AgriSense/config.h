#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/**
 * @file config.h
 * @brief Configuration constants, calibration values, and thresholds for AgriSense.
 */

// =============================================================================
// Wi-Fi Connection Parameters
// =============================================================================
constexpr char WIFI_STA_SSID[] = "Irfan";    ///< Local Wi-Fi Router SSID
constexpr char WIFI_STA_PASS[] = "01400748802";   ///< Local Wi-Fi Router Password
constexpr char WIFI_AP_SSID[]  = "AgriSense_AP";        ///< Fallback AP Access Point SSID
constexpr char WIFI_AP_PASS[]  = "12345678";          ///< Fallback AP Access Point Password (min 8 chars)
constexpr unsigned long WIFI_CONN_TIMEOUT_MS = 15000;   ///< Wi-Fi connection timeout before AP fallback (ms)

// =============================================================================
// Sensor Calibration Constants (Ajust relative to raw ADC reads)
// =============================================================================
constexpr int SOIL_MOISTURE_DRY = 3100;                  ///< RAW ADC value of soil moisture probe in dry air
constexpr int SOIL_MOISTURE_WET = 1400;                  ///< RAW ADC value of soil moisture probe in water
constexpr int RAIN_SENSOR_DRY   = 3500;                  ///< RAW ADC value threshold representing dry weather
constexpr int RAIN_SENSOR_WET   = 1500;                  ///< RAW ADC value threshold representing active rain

// =============================================================================
// Control Thresholds
// =============================================================================
constexpr float DEFAULT_DRY_THRESHOLD = 30.0f;          ///< Soil Moisture % to trigger pump (AUTO Mode)
constexpr float DEFAULT_WET_THRESHOLD = 70.0f;          ///< Soil Moisture % target to turn off pump (AUTO Mode)

// =============================================================================
// System Timings (Non-blocking Millis intervals)
// =============================================================================
constexpr unsigned long READ_SOIL_INTERVAL_MS  = 1000;  ///< Period between soil moisture samples (ms)
constexpr unsigned long READ_DHT_INTERVAL_MS   = 2000;  ///< Period between DHT22 samples (ms)
constexpr unsigned long READ_LIGHT_INTERVAL_MS = 1000;  ///< Period between BH1750 samples (ms)
constexpr unsigned long READ_RAIN_INTERVAL_MS  = 500;   ///< Period between rain sensor samples (ms)
constexpr unsigned long OLED_REFRESH_RATE_MS   = 1000;  ///< Period between screen updates (ms)
constexpr unsigned long TELEMETRY_INTERVAL_MS  = 1000;  ///< Period between core decision engine checks (ms)

// =============================================================================
// Safety Limits & Alarm Constants
// =============================================================================
constexpr unsigned long PUMP_MAX_RUN_TIME_MS  = 30000;  ///< Max allowed pump running time before shutdown (ms)
constexpr unsigned long PUMP_COOLDOWN_TIME_MS = 10000;  ///< Cooldown time required between active pump cycles (ms)
constexpr int RELAY_TRIGGER_LEVEL             = LOW;    ///< LOW for Active-Low relays, HIGH for Active-High
constexpr int BUZZER_ALARM_FREQ_HZ            = 2300;   ///< Active buzzer sound frequency (Hz)
constexpr unsigned long PUMP_DEBOUNCE_MS      = 500;    ///< Toggle debounce timing (ms)

#endif // CONFIG_H
