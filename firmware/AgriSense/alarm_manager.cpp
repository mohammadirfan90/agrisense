#include "alarm_manager.h"
#include "utils.h"

namespace AlarmManager {

    // Internal states
    static AlarmType activeAlarm = AlarmType::NONE;
    static unsigned long lastToggleTime = 0;
    static bool isBuzzerOn = false;
    static int patternIndex = 0;

    void initAlarms() {
        pinMode(BUZZER_CTRL_PIN, OUTPUT);
        digitalWrite(BUZZER_CTRL_PIN, LOW);
        activeAlarm = AlarmType::NONE;
        isBuzzerOn = false;
        lastToggleTime = 0;
        patternIndex = 0;
        Utils::log(LogLevel::INFO, "AlarmMgr", "Buzzer initialized.");
    }

    void triggerAlarm(AlarmType type) {
        if (activeAlarm == type) return; // Already running
        
        activeAlarm = type;
        lastToggleTime = millis();
        patternIndex = 0;
        
        // Start pattern immediately
        if (activeAlarm != AlarmType::NONE) {
            digitalWrite(BUZZER_CTRL_PIN, HIGH);
            isBuzzerOn = true;
            Utils::log(LogLevel::WARNING, "AlarmMgr", "Alarm triggered.");
        } else {
            digitalWrite(BUZZER_CTRL_PIN, LOW);
            isBuzzerOn = false;
        }
    }

    void clearAlarm() {
        if (activeAlarm == AlarmType::NONE) return;
        
        activeAlarm = AlarmType::NONE;
        digitalWrite(BUZZER_CTRL_PIN, LOW);
        isBuzzerOn = false;
        Utils::log(LogLevel::INFO, "AlarmMgr", "Alarms cleared.");
    }

    bool isAlarmActive() {
        return activeAlarm != AlarmType::NONE;
    }

    AlarmType getActiveAlarmType() {
        return activeAlarm;
    }

    void updateAlarms() {
        if (activeAlarm == AlarmType::NONE) {
            if (isBuzzerOn) {
                digitalWrite(BUZZER_CTRL_PIN, LOW);
                isBuzzerOn = false;
            }
            return;
        }

        unsigned long now = millis();
        unsigned long elapsed = now - lastToggleTime;

        switch (activeAlarm) {
            case AlarmType::STARTUP:
                // Pattern: Beep once for 150ms then stop
                if (patternIndex == 0 && elapsed >= 150) {
                    digitalWrite(BUZZER_CTRL_PIN, LOW);
                    isBuzzerOn = false;
                    lastToggleTime = now;
                    patternIndex = 1;
                } else if (patternIndex == 1) {
                    activeAlarm = AlarmType::NONE; // Finished startup beep
                }
                break;

            case AlarmType::SENSOR_FAULT:
                // Pattern: Fast beep (150ms ON, 150ms OFF)
                if (elapsed >= 150) {
                    isBuzzerOn = !isBuzzerOn;
                    digitalWrite(BUZZER_CTRL_PIN, isBuzzerOn ? HIGH : LOW);
                    lastToggleTime = now;
                }
                break;

            case AlarmType::PUMP_TIMEOUT:
                // Pattern: Slow heavy beep (1000ms ON, 500ms OFF)
                if (isBuzzerOn && elapsed >= 1000) {
                    digitalWrite(BUZZER_CTRL_PIN, LOW);
                    isBuzzerOn = false;
                    lastToggleTime = now;
                } else if (!isBuzzerOn && elapsed >= 500) {
                    digitalWrite(BUZZER_CTRL_PIN, HIGH);
                    isBuzzerOn = true;
                    lastToggleTime = now;
                }
                break;

            case AlarmType::LOW_BATTERY:
                // Pattern: Double beep (100ms ON, 100ms OFF, 100ms ON, 1000ms OFF)
                // patternIndex tracks step:
                // 0: First Beep ON (100ms)
                // 1: First Beep OFF (100ms)
                // 2: Second Beep ON (100ms)
                // 3: Second Beep OFF (1000ms)
                if (patternIndex == 0 && elapsed >= 100) {
                    digitalWrite(BUZZER_CTRL_PIN, LOW);
                    isBuzzerOn = false;
                    lastToggleTime = now;
                    patternIndex = 1;
                } else if (patternIndex == 1 && elapsed >= 100) {
                    digitalWrite(BUZZER_CTRL_PIN, HIGH);
                    isBuzzerOn = true;
                    lastToggleTime = now;
                    patternIndex = 2;
                } else if (patternIndex == 2 && elapsed >= 100) {
                    digitalWrite(BUZZER_CTRL_PIN, LOW);
                    isBuzzerOn = false;
                    lastToggleTime = now;
                    patternIndex = 3;
                } else if (patternIndex == 3 && elapsed >= 1000) {
                    digitalWrite(BUZZER_CTRL_PIN, HIGH);
                    isBuzzerOn = true;
                    lastToggleTime = now;
                    patternIndex = 0; // Loop pattern
                }
                break;

            default:
                break;
        }
    }

} // namespace AlarmManager
