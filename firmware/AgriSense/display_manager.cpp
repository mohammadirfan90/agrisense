#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "display_manager.h"
#include "pins.h"
#include "config.h"
#include "utils.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

namespace DisplayManager {

    // Onboard OLED screen interface driver
    static Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
    static bool isOLEDInitialized = false;

    void initDisplay() {
        // Wire.begin must be called prior to init (handled by sensor_manager, but safe to call begin again)
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
            Utils::log(LogLevel::ERROR, "DisplayMgr", "SSD1306 OLED initialization failed!");
            isOLEDInitialized = false;
            return;
        }

        isOLEDInitialized = true;
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.display();
        Utils::log(LogLevel::INFO, "DisplayMgr", "SSD1306 OLED initialized.");
    }

    void drawBootScreen() {
        if (!isOLEDInitialized) return;

        display.clearDisplay();
        
        // Border
        display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
        display.drawRect(2, 2, 124, 60, SSD1306_WHITE);

        // Text
        display.setTextSize(2);
        display.setCursor(12, 12);
        display.print("AgriSense");

        display.setTextSize(1);
        display.setCursor(35, 36);
        display.print("Smart IoT");

        display.setCursor(20, 48);
        display.print("Initializing...");

        display.display();
    }

    void drawMonitoringScreen(float temp, float hum, float moisture, float light, 
                              const String& rain, const String& mode, bool pumpRunning, 
                              const String& ipAddr, int rssi, bool hasError) {
        if (!isOLEDInitialized) return;

        display.clearDisplay();

        // 1. Top Status Bar: IP Address & Wi-Fi RSSI
        display.setTextSize(1);
        display.setCursor(0, 0);
        if (ipAddr.startsWith("192.168.4.")) {
            display.print("AP: ");
        } else {
            display.print("IP: ");
        }
        display.print(ipAddr);

        // Draw basic RSSI signal bar indicator
        display.setCursor(105, 0);
        if (rssi == 0) {
            display.print("AP");
        } else {
            display.printf("%ddB", rssi);
        }
        
        // Draw dividing line
        display.drawFastHLine(0, 9, 128, SSD1306_WHITE);

        // 2. Main Body: Telemetry Metrics
        // Soil Moisture Line
        display.setCursor(0, 13);
        display.print("Soil Moisture: ");
        display.setTextSize(1);
        display.print(moisture, 1);
        display.print("%");

        // Temperature & Humidity Line
        display.setTextSize(1);
        display.setCursor(0, 25);
        display.printf("Temp: %.1fC  Hum: %.1f%%", temp, hum);

        // Light & Rain Line
        display.setCursor(0, 37);
        display.printf("Light: %.0f lx R: ", light);
        display.print(rain);

        // Draw dividing line
        display.drawFastHLine(0, 48, 128, SSD1306_WHITE);

        // 3. Bottom Status Bar: Mode, Pump State, Error Warnings
        display.setCursor(0, 52);
        if (hasError) {
            display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted Warning Text
            display.print("  !! SYSTEM ERROR !!  ");
            display.setTextColor(SSD1306_WHITE);
        } else {
            display.print("Mode: ");
            display.print(mode);
            
            // Draw Pump status box at bottom right
            display.setCursor(80, 52);
            if (pumpRunning) {
                display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted running label
                display.print(" PUMP:ON ");
                display.setTextColor(SSD1306_WHITE);
            } else {
                display.print("PUMP:OFF");
            }
        }

        display.display();
    }

    void drawErrorScreen(const String& errorMsg) {
        if (!isOLEDInitialized) return;

        display.clearDisplay();

        // Border box
        display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
        
        display.setTextSize(1);
        display.setCursor(26, 8);
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted warning header
        display.print(" SYSTEM FAULT ");
        display.setTextColor(SSD1306_WHITE);

        display.setCursor(10, 26);
        display.print("Error: ");
        display.print(errorMsg);

        display.setCursor(10, 44);
        display.print("System is Locked.");
        display.setCursor(10, 53);
        display.print("Checking triggers...");

        display.display();
    }

    void clearDisplay() {
        if (!isOLEDInitialized) return;
        display.clearDisplay();
        display.display();
    }

} // namespace DisplayManager
