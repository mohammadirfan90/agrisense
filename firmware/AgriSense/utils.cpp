#include "utils.h"

namespace Utils {

    void initLogging(unsigned long baudRate) {
        Serial.begin(baudRate);
        delay(10); // Short stabilization wait
        Serial.println("\n=============================================");
        Serial.println("AgriSense Firmware Logging Active");
        Serial.println("=============================================");
    }

    void log(LogLevel level, const char* sender, const String& message) {
        String levelStr;
        switch (level) {
            case LogLevel::DEBUG:
                levelStr = "[DEBUG]";
                break;
            case LogLevel::INFO:
                levelStr = "[INFO ]";
                break;
            case LogLevel::WARNING:
                levelStr = "[WARN ]";
                break;
            case LogLevel::ERROR:
                levelStr = "[ERROR]";
                break;
        }

        unsigned long ms = millis();
        // Print syntax: [TIME_MS] [LEVEL] [SENDER] message
        Serial.printf("[%lu ms] %s [%s] %s\n", ms, levelStr.c_str(), sender, message.c_str());
    }

    String formatUptime(unsigned long uptimeSecs) {
        unsigned long hours = uptimeSecs / 3600;
        unsigned long mins = (uptimeSecs % 3600) / 60;
        unsigned long secs = uptimeSecs % 60;

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%02lu:%02lu:%02lu", hours, mins, secs);
        return String(buffer);
    }

} // namespace Utils
