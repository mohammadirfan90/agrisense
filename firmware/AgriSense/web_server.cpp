#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "web_server.h"
#include "sensor_manager.h"
#include "pump_controller.h"
#include "wifi_manager.h"
#include "alarm_manager.h"
#include "utils.h"

namespace WebServerManager {

    // Onboard HTTP web server on Port 80
    static WebServer server(80);
    static SystemMode currentMode = SystemMode::AUTO;

    // Helper: Determine standard Content-Type from file extensions
    static String getContentType(const String& filename) {
        if (filename.endsWith(".html")) return "text/html";
        if (filename.endsWith(".css"))  return "text/css";
        if (filename.endsWith(".js"))   return "application/javascript";
        if (filename.endsWith(".png"))  return "image/png";
        if (filename.endsWith(".ico"))  return "image/x-icon";
        if (filename.endsWith(".svg"))  return "image/svg+xml";
        return "text/plain";
    }

    // Helper: Stream files from LittleFS to client browser
    static bool handleFileRead(String path) {
        if (path.endsWith("/")) {
            path += "index.html";
        }
        
        String contentType = getContentType(path);
        if (LittleFS.exists(path)) {
            File file = LittleFS.open(path, "r");
            server.streamFile(file, contentType);
            file.close();
            return true;
        }
        
        return false;
    }

    // Handler: Serve static files or return 404
    static void handleNotFound() {
        if (handleFileRead(server.uri())) {
            return;
        }
        
        // Return JSON error if file is missing
        StaticJsonDocument<128> doc;
        doc["success"] = false;
        doc["error"] = "File not found or route undefined.";
        
        String jsonResponse;
        serializeJson(doc, jsonResponse);
        server.send(404, "application/json", jsonResponse);
    }

    // Handler: GET /api/status
    static void handleGetStatus() {
        // Enforce CORS so local browser testing is allowed
        server.sendHeader("Access-Control-Allow-Origin", "*");
        
        StaticJsonDocument<512> doc;
        doc["temperature"] = SensorManager::getTemperature();
        doc["humidity"] = SensorManager::getHumidity();
        doc["soilMoisture"] = SensorManager::getSoilMoisturePct();
        doc["lightIntensity"] = SensorManager::getLightIntensityLux();
        doc["rainDetected"] = SensorManager::isRainDetected();
        doc["pumpRunning"] = PumpController::getPumpState();
        doc["mode"] = getSystemModeString();
        doc["uptime"] = millis() / 1000;
        doc["wifiRSSI"] = WiFiManager::getRSSI();
        doc["sensorFault"] = SensorManager::hasSensorError();
        doc["pumpTimeout"] = PumpController::isPumpTimeoutActive();

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        server.send(200, "application/json", jsonResponse);
    }

    // Handler: POST /api/pump/on
    static void handlePostPumpOn() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        
        StaticJsonDocument<128> doc;
        if (currentMode == SystemMode::AUTO) {
            doc["success"] = false;
            doc["error"] = "Pump command rejected: Manual override disabled in AUTO mode.";
            String res;
            serializeJson(doc, res);
            server.send(403, "application/json", res);
            return;
        }

        if (PumpController::isPumpTimeoutActive()) {
            doc["success"] = false;
            doc["error"] = "Pump command rejected: Safety timeout lockout is active.";
            String res;
            serializeJson(doc, res);
            server.send(403, "application/json", res);
            return;
        }

        PumpController::setPumpState(true);
        doc["success"] = true;
        doc["message"] = "Pump relay turned ON.";
        
        String res;
        serializeJson(doc, res);
        server.send(200, "application/json", res);
    }

    // Handler: POST /api/pump/off
    static void handlePostPumpOff() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        
        StaticJsonDocument<128> doc;
        if (currentMode == SystemMode::AUTO) {
            doc["success"] = false;
            doc["error"] = "Pump command rejected: Manual override disabled in AUTO mode.";
            String res;
            serializeJson(doc, res);
            server.send(403, "application/json", res);
            return;
        }

        PumpController::setPumpState(false);
        doc["success"] = true;
        doc["message"] = "Pump relay turned OFF.";
        
        String res;
        serializeJson(doc, res);
        server.send(200, "application/json", res);
    }

    // Handler: POST /api/mode/auto
    static void handlePostModeAuto() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        
        setSystemMode(SystemMode::AUTO);
        
        StaticJsonDocument<128> doc;
        doc["success"] = true;
        doc["mode"] = "AUTO";
        doc["message"] = "System operational mode switched to AUTO.";
        
        String res;
        serializeJson(doc, res);
        server.send(200, "application/json", res);
    }

    // Handler: POST /api/mode/manual
    static void handlePostModeManual() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        
        setSystemMode(SystemMode::MANUAL);
        
        StaticJsonDocument<128> doc;
        doc["success"] = true;
        doc["mode"] = "MANUAL";
        doc["message"] = "System operational mode switched to MANUAL.";
        
        String res;
        serializeJson(doc, res);
        server.send(200, "application/json", res);
    }

    // Handler: OPTIONS (for preflight CORS approval from browsers)
    static void handleOptions() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    }

    void initWebServer() {
        // Init LittleFS
        if (!LittleFS.begin(true)) {
            Utils::log(LogLevel::ERROR, "WebServer", "Failed to mount LittleFS partitions.");
        } else {
            Utils::log(LogLevel::INFO, "WebServer", "LittleFS partitions mounted.");
        }

        // Binds REST Endpoints
        server.on("/api/status", HTTP_GET, handleGetStatus);
        server.on("/api/pump/on", HTTP_POST, handlePostPumpOn);
        server.on("/api/pump/off", HTTP_POST, handlePostPumpOff);
        server.on("/api/mode/auto", HTTP_POST, handlePostModeAuto);
        server.on("/api/mode/manual", HTTP_POST, handlePostModeManual);

        // Preflight CORS bindings
        server.on("/api/pump/on", HTTP_OPTIONS, handleOptions);
        server.on("/api/pump/off", HTTP_OPTIONS, handleOptions);
        server.on("/api/mode/auto", HTTP_OPTIONS, handleOptions);
        server.on("/api/mode/manual", HTTP_OPTIONS, handleOptions);

        // Static files hook
        server.onNotFound(handleNotFound);

        server.begin();
        Utils::log(LogLevel::INFO, "WebServer", "HTTP REST WebServer active on port 80.");
    }

    void updateWebServer() {
        server.handleClient();
    }

    SystemMode getSystemMode() {
        return currentMode;
    }

    void setSystemMode(SystemMode mode) {
        if (currentMode != mode) {
            currentMode = mode;
            Utils::log(LogLevel::INFO, "WebServer", "System mode updated: " + getSystemModeString());
        }
    }

    String getSystemModeString() {
        return (currentMode == SystemMode::AUTO) ? "AUTO" : "MANUAL";
    }

} // namespace WebServerManager
