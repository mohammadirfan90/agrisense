#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

/**
 * @file utils.h
 * @brief Logging and formatting utilities for AgriSense.
 */

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

namespace Utils {

    /**
     * @brief Initialize serial logging interface.
     * @param baudRate Target serial communication speed.
     */
    void initLogging(unsigned long baudRate = 115200);

    /**
     * @brief Write formatted log message to Serial.
     * @param level Importance level of the message.
     * @param sender Identifier of the module emitting the log.
     * @param message Text content of the log.
     */
    void log(LogLevel level, const char* sender, const String& message);

    /**
     * @brief Format uptime in seconds into human-readable HH:MM:SS string.
     * @param uptimeSecs System uptime in seconds.
     * @return Formatted time string.
     */
    String formatUptime(unsigned long uptimeSecs);

} // namespace Utils

#endif // UTILS_H
