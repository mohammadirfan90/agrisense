#include <WiFi.h>
#include "wifi_manager.h"
#include "config.h"
#include "utils.h"

namespace WiFiManager {

    // Active operational state variables
    static bool isAPActive = false;
    static unsigned long lastReconnectTime = 0;
    constexpr unsigned long RECONNECT_INTERVAL_MS = 10000; // Try reconnecting every 10s

    void initWiFi() {
        Utils::log(LogLevel::INFO, "WiFiMgr", "Attempting connection to router: " + String(WIFI_STA_SSID));
        
        WiFi.mode(WIFI_MODE_STA);
        WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);

        unsigned long startAttemptTime = millis();
        // Wait for connection with timeout (blocking is allowed in setup phase)
        while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_CONN_TIMEOUT_MS) {
            delay(500);
            Serial.print(".");
        }

        if (WiFi.status() == WL_CONNECTED) {
            isAPActive = false;
            Serial.println();
            Utils::log(LogLevel::INFO, "WiFiMgr", "Connected to Wi-Fi. IP Address: " + WiFi.localIP().toString());
        } else {
            Serial.println();
            Utils::log(LogLevel::WARNING, "WiFiMgr", "Wi-Fi connection timed out. Activating Access Point fallback...");
            
            WiFi.mode(WIFI_MODE_AP);
            bool apCreated = WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
            if (apCreated) {
                isAPActive = true;
                Utils::log(LogLevel::INFO, "WiFiMgr", "Access Point broadcasting. AP SSID: " + String(WIFI_AP_SSID));
                Utils::log(LogLevel::INFO, "WiFiMgr", "AP Gateway IP: " + WiFi.softAPIP().toString());
            } else {
                Utils::log(LogLevel::ERROR, "WiFiMgr", "Failed to start SoftAP.");
            }
        }
    }

    void updateWiFi() {
        if (isAPActive) return; // Do not check connection in AP Mode

        unsigned long now = millis();
        if (WiFi.status() != WL_CONNECTED) {
            if (now - lastReconnectTime >= RECONNECT_INTERVAL_MS) {
                lastReconnectTime = now;
                Utils::log(LogLevel::WARNING, "WiFiMgr", "Wi-Fi connection lost. Attempting reconnection...");
                WiFi.disconnect();
                WiFi.begin(WIFI_STA_SSID, WIFI_STA_PASS);
            }
        }
    }

    bool isWiFiConnected() {
        return (WiFi.status() == WL_CONNECTED);
    }

    bool isAPModeActive() {
        return isAPActive;
    }

    String getIPAddress() {
        if (isAPActive) {
            return WiFi.softAPIP().toString();
        }
        return WiFi.localIP().toString();
    }

    int getRSSI() {
        if (isAPActive) {
            return 0; // RSSI is 0 in local AP broadcast mode
        }
        return WiFi.RSSI();
    }

} // namespace WiFiManager
