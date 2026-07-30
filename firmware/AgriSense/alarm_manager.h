#ifndef ALARM_MANAGER_H
#define ALARM_MANAGER_H

#include <Arduino.h>
#include "pins.h"
#include "config.h"

/**
 * @file alarm_manager.h
 * @brief Non-blocking warning tones and alarm state controller for AgriSense.
 */

enum class AlarmType {
    NONE,
    STARTUP,
    SENSOR_FAULT,
    PUMP_TIMEOUT,
    LOW_BATTERY
};

namespace AlarmManager {

    /**
     * @brief Initialize physical buzzer pin interface.
     */
    void initAlarms();

    /**
     * @brief Trigger specific alarm pattern.
     * @param type Target alarm pattern to start.
     */
    void triggerAlarm(AlarmType type);

    /**
     * @brief Clear active alarm and turn off buzzer.
     */
    void clearAlarm();

    /**
     * @brief Non-blocking update cycle for driving buzzer state changes.
     * @details Must be called in main loop.
     */
    void updateAlarms();

    /**
     * @brief Check if any alarm is currently active.
     * @return True if system warning buzzer is working.
     */
    bool isAlarmActive();

    /**
     * @brief Get active alarm type.
     * @return AlarmType state.
     */
    AlarmType getActiveAlarmType();

} // namespace AlarmManager

#endif // ALARM_MANAGER_H
