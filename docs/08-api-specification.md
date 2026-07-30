# 8. REST API Specification

## 8.1 API Design Principles
The communication interface between the web dashboard and the ESP32 firmware relies on a REST API.
*   **Protocol**: HTTP/1.1
*   **Content Type**: Always `application/json` for requests and responses.
*   **State Control**: Writing endpoints (POST) require an explicit action payload to prevent accidental double-execution.

---

## 8.2 Endpoint Definitions

### 1. Retrieve Current System Status
*   **Endpoint**: `GET /api/status`
*   **Purpose**: Returns all current sensor readings, operational mode, pump status, system uptime, and active alarms.
*   **Request Headers**: `Accept: application/json`
*   **Request Body**: None (Empty)
*   **Success Response**:
    *   **Status Code**: `200 OK`
    *   **Body**:
        ```json
        {
          "uptime_s": 1205,
          "mode": "AUTO",
          "pump": "OFF",
          "sensors": {
            "moisture": 32.4,
            "temp": 24.5,
            "humidity": 62.1,
            "lux": 150.0,
            "rain": "DRY"
          },
          "alarms": {
            "sensor_fault": false,
            "pump_timeout": false,
            "low_battery": false
          }
        }
        ```

### 2. Set Pump Operation Mode
*   **Endpoint**: `POST /api/mode`
*   **Purpose**: Sets the system mode to automatic closed-loop threshold control or manual dashboard override control.
*   **Request Body**:
    ```json
    {
      "mode": "MANUAL"
    }
    ```
    *(Allowed values: `"AUTO"`, `"MANUAL"`)*
*   **Success Response**:
    *   **Status Code**: `200 OK`
    *   **Body**:
        ```json
        {
          "success": true,
          "message": "System mode updated to MANUAL",
          "current_mode": "MANUAL"
        }
        ```
*   **Error Responses**:
    *   **Status Code**: `400 Bad Request` (Invalid JSON or invalid mode string)
        ```json
        {
          "success": false,
          "error": "Invalid mode parameter. Expected 'AUTO' or 'MANUAL'"
        }
        ```

### 3. Switch Pump Relay State
*   **Endpoint**: `POST /api/pump`
*   **Purpose**: Manually toggles the physical pump relay ON or OFF.
*   **Constraint**: This command is rejected if the system is currently in `AUTO` mode, or if a safety lockout (like a rain lockout or run timeout) is active.
*   **Request Body**:
    ```json
    {
      "state": "ON"
    }
    ```
    *(Allowed values: `"ON"`, `"OFF"`)*
*   **Success Response**:
    *   **Status Code**: `200 OK`
    *   **Body**:
        ```json
        {
          "success": true,
          "message": "Pump activated successfully",
          "pump_state": "ON"
        }
        ```
*   **Error Responses**:
    *   **Status Code**: `403 Forbidden` (Command sent while system is in AUTO mode or when safety lockout is active)
        ```json
        {
          "success": false,
          "error": "Command rejected: manual override is disabled in AUTO mode."
        }
        ```
    *   **Status Code**: `400 Bad Request` (Invalid JSON or invalid state string)
        ```json
        {
          "success": false,
          "error": "Invalid state parameter. Expected 'ON' or 'OFF'"
        }
        ```

---

## 8.3 HTTP Status Code Mapping

| Status Code | Reason for Use | Scenario in AgriSense |
| :--- | :--- | :--- |
| `200 OK` | Request completed successfully. | Returned for valid status reads and successful pump/mode state changes. |
| `400 Bad Request` | Request payload parsing failed or contains malformed keys. | The dashboard sent an invalid JSON body or unsupported status commands. |
| `403 Forbidden` | Command parsed but rejected due to logical constraints. | Attempted to turn on the pump manually while in `AUTO` mode, or while a rain lockout was active. |
| `404 Not Found` | The requested API route does not exist. | Client attempted to access `/api/config` or other undefined paths. |
| `500 Internal Error` | Firmware encountered an internal processing fault. | The LittleFS flash system is unmounted, or an I2C device timed out. |
| `503 Service Unavailable` | ESP32 cannot handle the request at this time. | The WebServer queue is full, or the system is currently restarting. |
