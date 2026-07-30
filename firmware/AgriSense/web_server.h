#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>

/**
 * @file web_server.h
 * @brief Local Web Server and REST API controllers for AgriSense.
 */

enum class SystemMode {
    AUTO,
    MANUAL
};

namespace WebServerManager {

    /**
     * @brief Mount LittleFS partition, route URL endpoints, and start HTTP Server.
     */
    void initWebServer();

    /**
     * @brief Handle incoming client network calls.
     * @details Runs in core loop.
     */
    void updateWebServer();

    /**
     * @brief Retrieve current system mode (AUTO or MANUAL).
     * @return SystemMode enum value.
     */
    SystemMode getSystemMode();

    /**
     * @brief Modify current system mode.
     * @param mode Target SystemMode to set.
     */
    void setSystemMode(SystemMode mode);

    /**
     * @brief Get string representation of current system mode.
     * @return "AUTO" or "MANUAL".
     */
    String getSystemModeString();

} // namespace WebServerManager

#endif // WEB_SERVER_H
