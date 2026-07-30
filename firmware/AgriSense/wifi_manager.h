#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>

/**
 * @file wifi_manager.h
 * @brief Wi-Fi Connection Manager and AP Fallback controller for AgriSense.
 */

namespace WiFiManager {

    /**
     * @brief Configure Wi-Fi station mode or spin up fallback Access Point.
     * @details Blends blocking startup checks up to timeout configuration limits.
     */
    void initWiFi();

    /**
     * @brief Periodic non-blocking connection monitor.
     * @details Runs in core loop. Detects drops and handles reconnections in station mode.
     */
    void updateWiFi();

    /**
     * @brief Check if Wi-Fi station mode is connected.
     * @return True if connected to an external router.
     */
    bool isWiFiConnected();

    /**
     * @brief Check if fallback AP broadcast is active.
     * @return True if ESP32 is running its own local network.
     */
    bool isAPModeActive();

    /**
     * @brief Get active network IP address.
     * @return IP address string (e.g. 192.168.1.X or 192.168.4.1).
     */
    String getIPAddress();

    /**
     * @brief Read Wi-Fi received signal strength.
     * @return RSSI in dBm, or 0 if in AP mode.
     */
    int getRSSI();

} // namespace WiFiManager

#endif // WIFI_MANAGER_H
