/**
 * @file AgriSense.ino
 * @brief Main firmware entry point and task scheduler for AgriSense Smart Irrigation.
 * @details Manages non-blocking schedules using millis() and executes the closed-loop decision engine.
 */

#include "config.h"
#include "pins.h"
#include "utils.h"
#include "alarm_manager.h"
#include "sensor_manager.h"
#include "pump_controller.h"
#include "display_manager.h"
#include "wifi_manager.h"
#include "web_server.h"

// Non-blocking timer tracking states
static unsigned long lastDecisionTime = 0;
static unsigned long lastOLEDUpdateTime = 0;

void setup() {
    // 1. Initialize core system utilities and hardware pins
    Utils::initLogging(115200);
    AlarmManager::initAlarms();
    DisplayManager::initDisplay();

    // 2. Present startup visual and sound indicators
    DisplayManager::drawBootScreen();
    AlarmManager::triggerAlarm(AlarmType::STARTUP);

    // 3. Initialize background drivers
    SensorManager::initSensors();
    PumpController::initPump();

    // 4. Initialize network services
    WiFiManager::initWiFi();
    WebServerManager::initWebServer();

    Utils::log(LogLevel::INFO, "Main", "Setup complete. Entering run loop.");
}

void loop() {
    // =========================================================================
    // Core Non-blocking Driver Task Updates (Run every loop iteration)
    // =========================================================================
    AlarmManager::updateAlarms();
    SensorManager::updateSensors();
    WiFiManager::updateWiFi();
    PumpController::updatePumpWatchdog();
    WebServerManager::updateWebServer();

    unsigned long now = millis();

    // =========================================================================
    // 1. Closed-Loop Decision Engine (Run at TELEMETRY_INTERVAL_MS)
    // =========================================================================
    if (now - lastDecisionTime >= TELEMETRY_INTERVAL_MS) {
        lastDecisionTime = now;

        // Read active operational variables
        SystemMode mode = WebServerManager::getSystemMode();
        bool sensorFault = SensorManager::hasSensorError();
        bool pumpTimeout = PumpController::isPumpTimeoutActive();
        bool rainActive = SensorManager::isRainDetected();
        float moisture = SensorManager::getSoilMoisturePct();

        // Safety Guard A: Trigger shutdown if sensor disconnects
        if (sensorFault) {
            PumpController::setPumpState(false);
            AlarmManager::triggerAlarm(AlarmType::SENSOR_FAULT);
            Utils::log(LogLevel::ERROR, "Main", "Safety Alert: Sensor fault active. Irrigation blocked.");
        }
        // Safety Guard B: Shut down buzzer if sensor fault recovers
        else if (AlarmManager::getActiveAlarmType() == AlarmType::SENSOR_FAULT) {
            AlarmManager::clearAlarm();
        }

        // Evaluate Automatic Irrigation rules
        if (mode == SystemMode::AUTO && !sensorFault && !pumpTimeout) {
            bool pumpState = PumpController::getPumpState();

            if (!pumpState) {
                // Trigger condition: Moisture below dry threshold AND no rain detected
                if (moisture < DEFAULT_DRY_THRESHOLD && !rainActive) {
                    Utils::log(LogLevel::INFO, "Main", "AUTO Mode: Moisture dry. Starting pump.");
                    PumpController::setPumpState(true);
                }
            } else {
                // Deactivation conditions: Wet target reached OR rain detected
                if (moisture >= DEFAULT_WET_THRESHOLD) {
                    Utils::log(LogLevel::INFO, "Main", "AUTO Mode: Moisture target reached. Stopping pump.");
                    PumpController::setPumpState(false);
                } else if (rainActive) {
                    Utils::log(LogLevel::WARNING, "Main", "AUTO Mode: Precipitation detected. Emergency rain stop triggered.");
                    PumpController::setPumpState(false);
                }
            }
        }
    }

    // =========================================================================
    // 2. Physical User Interface Frame Update (Run at OLED_REFRESH_RATE_MS)
    // =========================================================================
    if (now - lastOLEDUpdateTime >= OLED_REFRESH_RATE_MS) {
        lastOLEDUpdateTime = now;

        bool sensorFault = SensorManager::hasSensorError();
        bool pumpTimeout = PumpController::isPumpTimeoutActive();
        bool pumpRunning = PumpController::getPumpState();
        
        String ipAddr = WiFiManager::getIPAddress();
        int rssi = WiFiManager::getRSSI();

        if (sensorFault) {
            DisplayManager::drawErrorScreen("SENSOR FAULT");
        } else if (pumpTimeout) {
            DisplayManager::drawErrorScreen("PUMP TIMEOUT");
        } else {
            // Render active monitoring values
            DisplayManager::drawMonitoringScreen(
                SensorManager::getTemperature(),
                SensorManager::getHumidity(),
                SensorManager::getSoilMoisturePct(),
                SensorManager::getLightIntensityLux(),
                SensorManager::getRainState(),
                WebServerManager::getSystemModeString(),
                pumpRunning,
                ipAddr,
                rssi,
                false
            );
        }
    }
}
