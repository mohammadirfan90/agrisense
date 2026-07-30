# AgriSense - Smart Irrigation System Using ESP32

AgriSense is an autonomous, smart agricultural monitoring and irrigation control system built on the ESP32 platform. The system leverages local sensors (soil moisture, temperature, humidity, light, and rain detection) to automate irrigation and hosts a responsive web dashboard directly on the ESP32 for remote monitoring and manual override control.

---

## 📂 Repository Directory Map

```
AgriSense/
│
├── docs/                               # System Architecture & Blueprints
│   ├── 01-project-overview.md          # Scope, FR, and NFR specifications
│   ├── 02-system-architecture.md       # Firmware layers, workflows, and state machines
│   ├── 03-hardware-components.md       # Component selection and compatibility reviews
│   ├── 04-pin-connections.md           # Pin mapping and signal routing
│   ├── 05-circuit-design.md            # Schematics and electrical signal flow
│   ├── 06-power-design.md              # Power budgets, battery configurations, star grounding
│   ├── 07-data-flow.md                 # Data pipeline, filtering, and JSON formats
│   ├── 08-api-specification.md         # REST HTTP endpoint specs
│   ├── 09-testing-plan.md              # QA, verification, and failure injection plans
│   ├── 10-deployment.md                # Development milestones and documentation index
│   ├── 11-implementation-guide.md      # Detailed implementation guide and configuration blueprints
│   └── images/                         # Architectural diagrams and schematics
│
├── firmware/                           # ESP32 Core Source Files
│   ├── AgriSense/                      # Arduino / C++ sketch and managers
│   │   ├── AgriSense.ino               # Main entry point and core loop
│   │   ├── config.h                    # System settings and threshold defines
│   │   ├── pins.h                      # Unified pin definitions
│   │   ├── sensor_manager.h/.cpp       # Sensor read and calibration logic
│   │   ├── pump_controller.h/.cpp      # Relay switching and runtime watchdogs
│   │   ├── display_manager.h/.cpp      # SSD1306 OLED rendering functions
│   │   ├── web_server.h/.cpp           # HTTP route handlers and JSON API
│   │   ├── wifi_manager.h/.cpp         # Wi-Fi connection and AP fallback handlers
│   │   └── alarm_manager.h/.cpp        # Warning codes and buzzer tones
│   └── libraries/                      # Dependency archives
│
├── web/                                # Frontend Source Files (stored in LittleFS)
│   ├── index.html                      # Semantic dashboard layout
│   ├── style.css                       # Premium responsive styles
│   ├── script.js                       # AJAX HTTP polling and UI updates
│   └── assets/                         # Icons and logos
│
├── report/                             # Final Academic Presentation Deliverables
│   ├── screenshots/                    # Dashboard UI images
│   ├── diagrams/                       # Circuit schematics
│   ├── final-report.md                 # Academic report in Markdown
│   └── final-report.docx               # Final written report file
│
├── LICENSE                             # MIT License
├── README.md                           # This readme file
└── SETUP_GUIDE.md                      # Hardware wiring and flashing guide
```

---

## 🚀 Setup & Deployment
For step-by-step instructions on physical breadboard assembly, power configurations, active buzzer transistor circuits, and uploading LittleFS static assets or C++ firmware, please refer to the **[SETUP_GUIDE.md](file:///e:/AgriSense/SETUP_GUIDE.md)** in the root directory.

## 📝 Project Report
A complete, structured final academic project report has been compiled and is available in Markdown format at **[report/final-report.md](file:///e:/AgriSense/report/final-report.md)**.

---

## 🛠 Architectural Specifications Index

For detailed hardware, software, power, and circuit design documentation, refer to the following specification files:

*   **Requirements & Objectives**: See [Project Overview & Requirements](file:///e:/AgriSense/docs/01-project-overview.md).
*   **System Architecture & Sequences**: See [System Architecture & Workflows](file:///e:/AgriSense/docs/02-system-architecture.md).
*   **Hardware Compatibility & Rationale**: See [Hardware Components & Compatibility Review](file:///e:/AgriSense/docs/03-hardware-components.md).
*   **Pin Assignments & Routing**: See [Pin Connections & Assignment Matrix](file:///e:/AgriSense/docs/04-pin-connections.md).
*   **Circuit Connections**: See [Circuit Design & Signal Flow](file:///e:/AgriSense/docs/05-circuit-design.md).
*   **Power Calculations & Safety**: See [Power Design & Distribution](file:///e:/AgriSense/docs/06-power-design.md).
*   **Data Conversion Pipeline**: See [Data Flow & Processing Pipeline](file:///e:/AgriSense/docs/07-data-flow.md).
*   **REST API Handshakes**: See [REST API Specification](file:///e:/AgriSense/docs/08-api-specification.md).
*   **Testing & QA Protocols**: See [Testing & Quality Assurance Plan](file:///e:/AgriSense/docs/09-testing-plan.md).
*   **Milestones & Roadmap**: See [Project Milestones & Documentation Index](file:///e:/AgriSense/docs/10-deployment.md).
*   **Implementation Guide & Config Blueprints**: See [Implementation & Development Guide](file:///e:/AgriSense/docs/11-implementation-guide.md).
