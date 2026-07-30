#include "pump_controller.h"
#include "pins.h"
#include "config.h"
#include "utils.h"
#include "alarm_manager.h"

namespace PumpController {

    // Active operational state variables
    static bool isPumpRunning = false;
    static bool isTimeoutActive = false;
    static unsigned long pumpStartTime = 0;
    static unsigned long lastStopTime = 0;
    static unsigned long lastCommandTime = 0; // State change debounce timer

    // Helper utilities for active relay level trigger logic
    static inline void driveRelayHardware(bool active) {
        if (active) {
            digitalWrite(PUMP_RELAY_PIN, RELAY_TRIGGER_LEVEL);
        } else {
            digitalWrite(PUMP_RELAY_PIN, (RELAY_TRIGGER_LEVEL == HIGH) ? LOW : HIGH);
        }
    }

    void initPump() {
        pinMode(PUMP_RELAY_PIN, INPUT); // Default to High-Impedance floating mode (Relay OFF)
        isPumpRunning = false;
        isTimeoutActive = false;
        pumpStartTime = 0;
        lastStopTime = 0;
        lastCommandTime = 0;
        Utils::log(LogLevel::INFO, "PumpCtrl", "Pump relay driver initialized.");
    }

    void setPumpState(bool turnOn) {
        unsigned long now = millis();

        // 1. Debounce rapid commands
        if (now - lastCommandTime < PUMP_DEBOUNCE_MS) {
            Utils::log(LogLevel::WARNING, "PumpCtrl", "Command ignored: command debounce active.");
            return;
        }

        // 2. Reject commands if watchdog timeout lockout is active
        if (turnOn && isTimeoutActive) {
            Utils::log(LogLevel::ERROR, "PumpCtrl", "Pump command rejected: watchdog timeout active. Reset required.");
            return;
        }

        // 3. Enforce cooldown delay between watering cycles
        if (turnOn && !isPumpRunning && (now - lastStopTime < PUMP_COOLDOWN_TIME_MS)) {
            Utils::log(LogLevel::WARNING, "PumpCtrl", "Pump command rejected: cooldown window active.");
            return;
        }

        lastCommandTime = now;

        if (turnOn) {
            if (!isPumpRunning) {
                isPumpRunning = true;
                pumpStartTime = now;
                pinMode(PUMP_RELAY_PIN, OUTPUT); // Configure as output before driving
                driveRelayHardware(true);
                Utils::log(LogLevel::INFO, "PumpCtrl", "Pump turned ON.");
            }
        } else {
            if (isPumpRunning) {
                isPumpRunning = false;
                lastStopTime = now;
                pinMode(PUMP_RELAY_PIN, INPUT); // Switch to INPUT to cut off leakage current (Relay OFF)
                Utils::log(LogLevel::INFO, "PumpCtrl", "Pump turned OFF.");
            }
        }
    }

    bool getPumpState() {
        return isPumpRunning;
    }

    void updatePumpWatchdog() {
        if (!isPumpRunning) return;

        unsigned long now = millis();
        if (now - pumpStartTime >= PUMP_MAX_RUN_TIME_MS) {
            // Force safe emergency shutdown
            isPumpRunning = false;
            lastStopTime = now;
            pinMode(PUMP_RELAY_PIN, INPUT); // Switch to INPUT to cut off leakage current (Relay OFF)
            isTimeoutActive = true;
            
            Utils::log(LogLevel::ERROR, "PumpCtrl", "CRITICAL: Pump max run time exceeded! Emergency shutdown triggered.");
            AlarmManager::triggerAlarm(AlarmType::PUMP_TIMEOUT);
        }
    }

    bool isPumpTimeoutActive() {
        return isTimeoutActive;
    }

    void resetPumpTimeout() {
        if (isTimeoutActive) {
            isTimeoutActive = false;
            AlarmManager::clearAlarm();
            Utils::log(LogLevel::INFO, "PumpCtrl", "Watchdog timeout reset completed.");
        }
    }

    unsigned long getActiveRunTimeMs() {
        if (!isPumpRunning) return 0;
        return millis() - pumpStartTime;
    }

} // namespace PumpController
