#include <Wire.h>
#include <DHT.h>
#include <hp_BH1750.h>
#include "sensor_manager.h"
#include "pins.h"
#include "config.h"
#include "utils.h"

namespace SensorManager {

    // Onboard hardware drivers
    static DHT dhtSensor(DHT_DATA_PIN, DHT22);
    static hp_BH1750 lightSensor;

    // Smoothed sensor states
    static float currentTemperature = 0.0f;
    static float currentHumidity = 0.0f;
    static float currentSoilMoisturePct = 0.0f;
    static float currentLux = 0.0f;
    static String currentRainState = "DRY";
    static bool rainDetectedStatus = false;
    static bool isSensorFaultActive = false;

    // Rolling average filters (10-sample windows)
    constexpr int FILTER_SIZE = 10;
    static float soilFilterBuffer[FILTER_SIZE];
    static float rainFilterBuffer[FILTER_SIZE];
    static int soilFilterIndex = 0;
    static int rainFilterIndex = 0;

    // Timing tracking states
    static unsigned long lastDHTSampleTime = 0;
    static unsigned long lastSoilSampleTime = 0;
    static unsigned long lastLightSampleTime = 0;
    static unsigned long lastRainSampleTime = 0;

    // Helper functions
    static void addSoilSample(float rawADC);
    static void addRainSample(float rawADC);
    static float computeAverage(const float* buffer, int size);

    void initSensors() {
        // Init digital pins
        dhtSensor.begin();
        pinMode(SOIL_ADC_PIN, INPUT);
        pinMode(RAIN_ADC_PIN, INPUT);

        // Init I2C bus (safe if multiple calls, Wire.begin is safe)
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
        
        // Init Light Sensor
        bool bhReady = lightSensor.begin(BH1750_TO_GROUND);
        if (!bhReady) {
            Utils::log(LogLevel::ERROR, "SensorMgr", "BH1750 not found on I2C.");
            isSensorFaultActive = true;
        } else {
            lightSensor.start();
        }

        // Pre-fill averaging filters to avoid initial zeros
        float initialSoil = analogRead(SOIL_ADC_PIN);
        float initialRain = analogRead(RAIN_ADC_PIN);
        for (int i = 0; i < FILTER_SIZE; i++) {
            soilFilterBuffer[i] = initialSoil;
            rainFilterBuffer[i] = initialRain;
        }
        
        // Populate initial readings
        addSoilSample(initialSoil);
        addRainSample(initialRain);
        
        // Safe check DHT22
        float testTemp = dhtSensor.readTemperature();
        float testHumid = dhtSensor.readHumidity();
        if (isnan(testTemp) || isnan(testHumid)) {
            Utils::log(LogLevel::WARNING, "SensorMgr", "DHT22 initial read failed.");
            isSensorFaultActive = true;
        } else {
            currentTemperature = testTemp;
            currentHumidity = testHumid;
        }

        Utils::log(LogLevel::INFO, "SensorMgr", "Sensors initialized.");
    }

    void updateSensors() {
        unsigned long now = millis();

        // 1. Sample Rain Sensor (Every 500ms)
        if (now - lastRainSampleTime >= READ_RAIN_INTERVAL_MS) {
            float rawRain = analogRead(RAIN_ADC_PIN);
            addRainSample(rawRain);
            lastRainSampleTime = now;

            float avgRain = computeAverage(rainFilterBuffer, FILTER_SIZE);
            if (avgRain <= RAIN_SENSOR_WET) {
                currentRainState = "HEAVY";
                rainDetectedStatus = true;
            } else if (avgRain < RAIN_SENSOR_DRY) {
                currentRainState = "LIGHT";
                rainDetectedStatus = true;
            } else {
                currentRainState = "DRY";
                rainDetectedStatus = false;
            }
        }

        // 2. Sample Soil Moisture (Every 1000ms)
        if (now - lastSoilSampleTime >= READ_SOIL_INTERVAL_MS) {
            float rawSoil = analogRead(SOIL_ADC_PIN);
            addSoilSample(rawSoil);
            lastSoilSampleTime = now;

            float avgSoilADC = computeAverage(soilFilterBuffer, FILTER_SIZE);
            
            // Map moisture calibrated range
            float moisture = ((float)(SOIL_MOISTURE_DRY - avgSoilADC) / (float)(SOIL_MOISTURE_DRY - SOIL_MOISTURE_WET)) * 100.0f;
            currentSoilMoisturePct = constrain(moisture, 0.0f, 100.0f);
        }

        // 3. Sample Light Intensity (Every 1000ms)
        if (now - lastLightSampleTime >= READ_LIGHT_INTERVAL_MS) {
            lightSensor.start(); // Trigger measurement
            float lux = lightSensor.getLux();
            if (lux >= 0.0f) {
                currentLux = lux;
            }
            lastLightSampleTime = now;
        }

        // 4. Sample Temperature & Humidity (Every 2000ms)
        if (now - lastDHTSampleTime >= READ_DHT_INTERVAL_MS) {
            float temp = dhtSensor.readTemperature();
            float humid = dhtSensor.readHumidity();

            if (isnan(temp) || isnan(humid)) {
                Utils::log(LogLevel::WARNING, "SensorMgr", "DHT22 read failure.");
                isSensorFaultActive = true;
            } else {
                currentTemperature = temp;
                currentHumidity = humid;
                isSensorFaultActive = false; // Reset if reading succeeds
            }
            lastDHTSampleTime = now;
        }
    }

    float getSoilMoisturePct() { return currentSoilMoisturePct; }
    float getTemperature() { return currentTemperature; }
    float getHumidity() { return currentHumidity; }
    float getLightIntensityLux() { return currentLux; }
    bool isRainDetected() { return rainDetectedStatus; }
    String getRainState() { return currentRainState; }
    bool hasSensorError() { return isSensorFaultActive; }

    // Private helper implementations
    static void addSoilSample(float rawADC) {
        soilFilterBuffer[soilFilterIndex] = rawADC;
        soilFilterIndex = (soilFilterIndex + 1) % FILTER_SIZE;
    }

    static void addRainSample(float rawADC) {
        rainFilterBuffer[rainFilterIndex] = rawADC;
        rainFilterIndex = (rainFilterIndex + 1) % FILTER_SIZE;
    }

    static float computeAverage(const float* buffer, int size) {
        float sum = 0.0f;
        for (int i = 0; i < size; i++) {
            sum += buffer[i];
        }
        return sum / (float)size;
    }

} // namespace SensorManager
