#ifndef PUMP_CONTROLLER_H
#define PUMP_CONTROLLER_H

#include <Arduino.h>

/**
 * @file pump_controller.h
 * @brief Safety-monitored pump relay controller for AgriSense.
 */

namespace PumpController {

    /**
     * @brief Configure GPIO direction for pump relay control.
     */
    void initPump();

    /**
     * @brief Trigger pump relay state change.
     * @param turnOn Set to true to switch pump ON, false to switch OFF.
     * @details Checks safety rules (timeout, cooldown limits) before running.
     */
    void setPumpState(bool turnOn);

    /**
     * @brief Read active state of pump relay.
     * @return True if pump is powered.
     */
    bool getPumpState();

    /**
     * @brief Non-blocking monitoring function for pump watchdog evaluation.
     * @details Must be called in core loop. Shuts down pump if runtime limits are exceeded.
     */
    void updatePumpWatchdog();

    /**
     * @brief Check if pump has been locked out due to a running timeout error.
     * @return True if timeout is active.
     */
    bool isPumpTimeoutActive();

    /**
     * @brief Clear active pump runtime timeout lockout.
     */
    void resetPumpTimeout();

    /**
     * @brief Get elapsed runtime of active watering cycle.
     * @return Run time in milliseconds.
     */
    unsigned long getActiveRunTimeMs();

} // namespace PumpController

#endif // PUMP_CONTROLLER_H
