#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

/**
 * @file display_manager.h
 * @brief SSD1306 OLED layout and frame renderer for AgriSense.
 */

namespace DisplayManager {

    /**
     * @brief Mount SSD1306 and initialize display buffer.
     */
    void initDisplay();

    /**
     * @brief Draw splash screen on startup.
     */
    void drawBootScreen();

    /**
     * @brief Renders the telemetry monitoring layout.
     * @param temp Temperature in Celsius.
     * @param hum Relative humidity %.
     * @param moisture Soil moisture %.
     * @param light Light intensity in Lux.
     * @param rain Rain state text ("DRY", "LIGHT", "HEAVY").
     * @param mode Operational mode ("AUTO" or "MANUAL").
     * @param pumpRunning True if pump relay is powered.
     * @param ipAddr Wi-Fi IP address or AP fallback address.
     * @param rssi Wi-Fi signal strength indicator in dBm.
     * @param hasError Set to true if a warning state is active.
     */
    void drawMonitoringScreen(float temp, float hum, float moisture, float light, 
                              const String& rain, const String& mode, bool pumpRunning, 
                              const String& ipAddr, int rssi, bool hasError);

    /**
     * @brief Display emergency shutdown or sensor error message screen.
     * @param errorMsg Reason for system block.
     */
    void drawErrorScreen(const String& errorMsg);

    /**
     * @brief Clear visual screen buffer and turn off pixels.
     */
    void clearDisplay();

} // namespace DisplayManager

#endif // DISPLAY_MANAGER_H
