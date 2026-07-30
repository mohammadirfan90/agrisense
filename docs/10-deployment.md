# 10. Project Milestones & Documentation Index

## 10.1 Engineering Development Milestones

The development of the AgriSense system is partitioned into five distinct, sequential phases to ensure stable integration:

```
[ Phase 1: Planning ] -> [ Phase 2: Hardware Assembly ] -> [ Phase 3: Drivers ] -> [ Phase 4: Integration ] -> [ Phase 5: QA ]
```

---

### Phase 1: Architecture & Design Verification
*   **Objective**: Establish electrical compatibility, pinout designs, and software layers.
*   **Deliverables**: 
    *   System architecture blueprints, pin maps, and circuit designs.
    *   REST API specifications.
*   **Completion Criteria**: Circuit design, pin maps, and API specifications are reviewed and approved.

### Phase 2: Hardware Assembly & Basic Validation
*   **Objective**: Assemble the physical circuit on a breadboard and verify basic wiring.
*   **Deliverables**:
    *   Assembled physical prototype on a breadboard.
    *   Diagnostic sketch outputs (like I2C bus scanner).
*   **Completion Criteria**: I2C scanner detects both the SSD1306 OLED (`0x3C`) and the BH1750 (`0x23`) addresses without signal dropouts.

### Phase 3: Driver & Local Logic Implementation
*   **Objective**: Write modular drivers for the sensors, OLED, buzzer, and relay.
*   **Deliverables**:
    *   `sensor_manager`, `pump_controller`, `display_manager`, and `alarm_manager` firmware files.
    *   Local offline control logic loops.
*   **Completion Criteria**: The system reads all sensors, applies rolling averages, updates the OLED screen, and toggles the relay based on moisture thresholds in a local offline loop.

### Phase 4: Network & Web Interface Integration
*   **Objective**: Implement Wi-Fi connectivity, HTTP web server endpoints, and the HTML/CSS/JS dashboard.
*   **Deliverables**:
    *   `wifi_manager` and `web_server` firmware files.
    *   Web dashboard files (`index.html`, `style.css`, `script.js`) stored in LittleFS.
*   **Completion Criteria**: A browser can connect to the ESP32 IP address, load the web dashboard, and successfully toggle the pump manually or view live-updated sensor values.

### Phase 5: System Testing & Final Packaging
*   **Objective**: Run failure injection tests, verify system safety features, and mount the components in a permanent housing.
*   **Deliverables**:
    *   Signed-off QA testing checklists.
    *   Completed final project prototype and documentation package.
    *   System testing logs.
*   **Completion Criteria**: The system runs continuously for 24 hours under varying moisture conditions without lockups, crashes, or safety watchdog failures.

---

## 10.2 Documentation Index (`/docs/`)

The following files are located in the `docs/` folder to serve as the system documentation:

1.  **[01-project-overview.md](file:///e:/AgriSense/docs/01-project-overview.md)**: Summarizes the project description, scope, functional (FR), and non-functional (NFR) requirements.
2.  **[02-system-architecture.md](file:///e:/AgriSense/docs/02-system-architecture.md)**: Details the software layers and system workflows, containing system sequence diagrams, state machines, and architecture models.
3.  **[03-hardware-components.md](file:///e:/AgriSense/docs/03-hardware-components.md)**: Analyzes component selections, hardware compatibility checks, and the component responsibility matrix.
4.  **[04-pin-connections.md](file:///e:/AgriSense/docs/04-pin-connections.md)**: Maps out the physical pin connections, including ADC allocation rules, bootstrapping pin avoidance, and I2C sharing.
5.  **[05-circuit-design.md](file:///e:/AgriSense/docs/05-circuit-design.md)**: Explains the electrical schematics, power connections, wiring layouts, and sensor data lines.
6.  **[06-power-design.md](file:///e:/AgriSense/docs/06-power-design.md)**: Details the power architecture, battery sizing calculations, star grounding design, and electrical safety measures.
7.  **[07-data-flow.md](file:///e:/AgriSense/docs/07-data-flow.md)**: Diagrams the telemetry pipeline, calibration equations, data smoothing, and JSON serialization.
8.  **[08-api-specification.md](file:///e:/AgriSense/docs/08-api-specification.md)**: Lists all REST API endpoints, HTTP methods, JSON payloads, and status code behavior.
9.  **[09-testing-plan.md](file:///e:/AgriSense/docs/09-testing-plan.md)**: Documents unit testing, integration tests, and failure injection scenarios.
10. **[10-deployment.md](file:///e:/AgriSense/docs/10-deployment.md)**: Establishes milestones, deliverables, and this documentation index.
