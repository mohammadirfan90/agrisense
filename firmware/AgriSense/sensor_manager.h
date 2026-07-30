#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>

/**
 * @file sensor_manager.h
 * @brief Telemetry sampling and signal smoothing manager for AgriSense.
 */

namespace SensorManager {

    /**
     * @brief Configure GPIO inputs, I2C devices, and initialize sensor libraries.
     */
    void initSensors();

    /**
     * @brief Periodic non-blocking sensor sampling loop.
     * @details Evaluates update tasks based on config interval variables.
     */
    void updateSensors();

    /**
     * @brief Get smoothed relative soil moisture percentage.
     * @return Percent value (0.0 to 100.0).
     */
    float getSoilMoisturePct();

    /**
     * @brief Get ambient air temperature.
     * @return Temperature in Celsius.
     */
    float getTemperature();

    /**
     * @brief Get ambient relative air humidity.
     * @return Relative humidity percentage (0.0 to 100.0).
     */
    float getHumidity();

    /**
     * @brief Get light intensity reading.
     * @return Light reading in Lux units.
     */
    float getLightIntensityLux();

    /**
     * @brief Determine if active rainfall is currently detected.
     * @return True if rain is falling.
     */
    bool isRainDetected();

    /**
     * @brief Get human-readable active rain state.
     * @return "DRY", "LIGHT", or "HEAVY".
     */
    String getRainState();

    /**
     * @brief Check if any sensor disconnect fault is active.
     * @return True if a sensor fails to respond.
     */
    bool hasSensorError();

} // namespace SensorManager

#endif // SENSOR_MANAGER_H
