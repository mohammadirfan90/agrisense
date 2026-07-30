// API Endpoints configuration
// If running locally on file:// protocol, default to mock/simulation mode.
const isLocalFile = window.location.protocol === 'file:';
const baseUrl = isLocalFile ? 'http://192.168.4.1' : ''; // Fallback to default ESP32 AP IP if opened locally

// Active State
let currentData = {
    temperature: 24.5,
    humidity: 58.2,
    soilMoisture: 35.0,
    lightIntensity: 250.0,
    rainDetected: false,
    pumpRunning: false,
    mode: "AUTO",
    uptime: 0,
    wifiRSSI: -60,
    sensorFault: false,
    pumpTimeout: false
};

let pollInterval = null;
let isSimulating = false;
let consecutiveFailures = 0;

// Initialize
document.addEventListener('DOMContentLoaded', () => {
    // Check if we need to run in simulation or fetch from device
    startPolling();
    
    // Add page visibility listener to save power when tab is inactive
    document.addEventListener('visibilitychange', () => {
        if (document.hidden) {
            stopPolling();
        } else {
            startPolling();
        }
    });
});

function startPolling() {
    if (pollInterval) clearInterval(pollInterval);
    refreshData();
    pollInterval = setInterval(refreshData, 2000);
}

function stopPolling() {
    if (pollInterval) {
        clearInterval(pollInterval);
        pollInterval = null;
    }
}

// Fetch system telemetry
async function refreshData() {
    if (isSimulating) {
        runSimulationStep();
        updateUI(currentData);
        return;
    }

    try {
        const response = await fetch(`${baseUrl}/api/status`, {
            method: 'GET',
            headers: { 'Accept': 'application/json' },
            mode: 'cors'
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const data = await response.json();
        currentData = data;
        consecutiveFailures = 0;
        updateUI(currentData);
        updateConnectionBadge(true);
    } catch (err) {
        consecutiveFailures++;
        console.warn("API request failed:", err);
        
        // If we fail 3 times and are on local file system, fall back to simulation
        if (consecutiveFailures >= 3 && isLocalFile) {
            console.log("Switching to offline simulation mode...");
            isSimulating = true;
            updateConnectionBadge(false, "Simulating");
        } else {
            updateConnectionBadge(false, "Offline");
        }
    }
}

// Set System Mode (AUTO vs MANUAL)
async function setMode(mode) {
    const endpoint = mode === 'AUTO' ? '/api/mode/auto' : '/api/mode/manual';
    
    // Optimistic UI updates
    currentData.mode = mode;
    if (mode === 'AUTO') {
        // In AUTO, pump state is managed by the ESP32, but let's disable manual buttons immediately
        const pumpBtn = document.getElementById('pump-toggle-btn');
        pumpBtn.classList.add('disabled-control');
        pumpBtn.disabled = true;
    }
    updateUI(currentData);

    if (isSimulating) {
        console.log(`[Sim] Mode updated to ${mode}`);
        return;
    }

    try {
        const response = await fetch(`${baseUrl}${endpoint}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            mode: 'cors'
        });
        const resData = await response.json();
        if (resData.success) {
            refreshData();
        } else {
            alert(`Error: ${resData.error}`);
        }
    } catch (err) {
        console.error("Failed to set mode:", err);
    }
}

// Toggle Pump (Only valid in MANUAL mode)
async function togglePump() {
    if (currentData.mode === 'AUTO') return;
    
    const nextState = !currentData.pumpRunning;
    const endpoint = nextState ? '/api/pump/on' : '/api/pump/off';
    
    // Optimistic UI update
    currentData.pumpRunning = nextState;
    updateUI(currentData);

    if (isSimulating) {
        console.log(`[Sim] Pump toggled to ${nextState ? 'ON' : 'OFF'}`);
        return;
    }

    try {
        const response = await fetch(`${baseUrl}${endpoint}`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            mode: 'cors'
        });
        const resData = await response.json();
        if (!resData.success) {
            alert(`Command Rejected: ${resData.error}`);
            // Revert state
            currentData.pumpRunning = !nextState;
            updateUI(currentData);
        } else {
            refreshData();
        }
    } catch (err) {
        console.error("Failed to toggle pump:", err);
        // Revert state
        currentData.pumpRunning = !nextState;
        updateUI(currentData);
    }
}

// Update DOM elements based on system state
function updateUI(data) {
    // 1. Controls
    const autoBtn = document.getElementById('mode-auto-btn');
    const manualBtn = document.getElementById('mode-manual-btn');
    const pumpBtn = document.getElementById('pump-toggle-btn');
    const pumpText = document.getElementById('pump-btn-text');
    const pumpHint = document.getElementById('pump-hint');
    const systemStatus = document.getElementById('system-status-indicator');

    // Update classes and text based on pump state
    if (data.pumpRunning) {
        pumpBtn.classList.remove('off');
        pumpBtn.classList.add('on');
        pumpText.innerText = data.mode === 'AUTO' ? "Pump ON (Auto)" : "Pump ON";
    } else {
        pumpBtn.classList.remove('on');
        pumpBtn.classList.add('off');
        pumpText.innerText = data.mode === 'AUTO' ? "Pump OFF (Auto)" : "Pump OFF";
    }

    if (data.mode === 'AUTO') {
        autoBtn.classList.add('active');
        manualBtn.classList.remove('active');
        pumpBtn.disabled = true;
        pumpBtn.classList.add('disabled-control');
        pumpHint.innerText = "Automatic thresholds are controlling the pump.";
        systemStatus.innerText = "Monitoring Environment";
    } else {
        autoBtn.classList.remove('active');
        manualBtn.classList.add('active');
        pumpBtn.disabled = false;
        pumpBtn.classList.remove('disabled-control');
        pumpHint.innerText = "Tap to toggle the physical pump relay.";
        systemStatus.innerText = "Manual Override Mode";
    }

    // Disable pump control completely if a safety error is active
    if (data.sensorFault || data.pumpTimeout) {
        pumpBtn.classList.remove('on', 'off');
        pumpBtn.classList.add('disabled-control');
        pumpBtn.disabled = true;
        pumpText.innerText = "Pump Locked Out";
    }

    // 2. Alert Banner
    const alertBanner = document.getElementById('alert-banner');
    const alertTitle = document.getElementById('alert-title');
    const alertMsg = document.getElementById('alert-msg');
    const safetyStatusVal = document.getElementById('safety-status');

    if (data.sensorFault) {
        alertBanner.classList.remove('hidden');
        alertTitle.innerText = "Sensor Error Active";
        alertMsg.innerText = "DHT22 or Soil Moisture probe is disconnected. Irrigation is halted to prevent hardware burnout.";
        safetyStatusVal.innerText = "Error Lockout";
        safetyStatusVal.className = "info-value text-red";
    } else if (data.pumpTimeout) {
        alertBanner.classList.remove('hidden');
        alertTitle.innerText = "Pump Safety Watchdog Triggered";
        alertMsg.innerText = "Pump has run continuously for over 30s without moisture changes. System entered safety lockout.";
        safetyStatusVal.innerText = "Timeout Lockout";
        safetyStatusVal.className = "info-value text-red";
    } else {
        alertBanner.classList.add('hidden');
        safetyStatusVal.innerText = "Active Guard";
        safetyStatusVal.className = "info-value text-green-bold";
    }

    // 3. Soil Moisture Card
    const moistureVal = document.getElementById('moisture-val');
    const moistureStatus = document.getElementById('moisture-status');
    const moistureRing = document.getElementById('moisture-ring');

    if (data.sensorFault) {
        moistureVal.innerText = "--";
        moistureStatus.innerText = "Sensor Error";
        moistureStatus.className = "status-pill text-red bg-red-light";
        setRingProgress(0);
    } else {
        const moist = Math.round(data.soilMoisture * 10) / 10;
        moistureVal.innerText = moist.toFixed(1);
        setRingProgress(data.soilMoisture);

        if (data.soilMoisture < 30) {
            moistureStatus.innerText = "Dry / Watering Alert";
            moistureStatus.className = "status-pill text-orange bg-orange-light";
            moistureRing.className.baseVal = "progress-ring-bar text-orange";
        } else if (data.soilMoisture > 70) {
            moistureStatus.innerText = "Saturated / Wet";
            moistureStatus.className = "status-pill text-blue bg-blue-light";
            moistureRing.className.baseVal = "progress-ring-bar text-blue";
        } else {
            moistureStatus.innerText = "Optimal Soil Health";
            moistureStatus.className = "status-pill text-green bg-green-light";
            moistureRing.className.baseVal = "progress-ring-bar text-green";
        }
    }

    // 4. Air Climate Card
    const tempVal = document.getElementById('temp-val');
    const humidVal = document.getElementById('humid-val');
    const climateStatus = document.getElementById('climate-status');

    if (data.sensorFault) {
        tempVal.innerText = "--";
        humidVal.innerText = "--";
        climateStatus.innerText = "Sensor Error";
        climateStatus.className = "status-pill text-red bg-red-light";
    } else {
        tempVal.innerText = data.temperature.toFixed(1);
        humidVal.innerText = data.humidity.toFixed(1);

        if (data.temperature > 35) {
            climateStatus.innerText = "High Ambient Heat";
            climateStatus.className = "status-pill text-orange bg-orange-light";
        } else if (data.temperature < 5) {
            climateStatus.innerText = "Frost Warning";
            climateStatus.className = "status-pill text-blue bg-blue-light";
        } else {
            climateStatus.innerText = "Moderate Climate";
            climateStatus.className = "status-pill text-green bg-green-light";
        }
    }

    // 5. Sunlight (Lux) Card
    const lightVal = document.getElementById('light-val');
    const lightStatus = document.getElementById('light-status');
    const lightIcon = document.getElementById('light-icon');

    lightVal.innerText = Math.round(data.lightIntensity);
    if (data.lightIntensity < 50) {
        lightStatus.innerText = "Night / Dark";
        lightStatus.className = "status-pill text-teal bg-teal-light";
        lightIcon.style.transform = "scale(0.85)";
        lightIcon.style.opacity = "0.5";
    } else if (data.lightIntensity > 5000) {
        lightStatus.innerText = "Direct Sunlight";
        lightStatus.className = "status-pill text-orange bg-orange-light";
        lightIcon.style.transform = "scale(1.15)";
        lightIcon.style.opacity = "1";
    } else {
        lightStatus.innerText = "Bright Daylight";
        lightStatus.className = "status-pill text-yellow bg-yellow-light";
        lightIcon.style.transform = "scale(1)";
        lightIcon.style.opacity = "0.8";
    }

    // 6. Precipitation (Rain) Card
    const rainVal = document.getElementById('rain-val');
    const rainStatus = document.getElementById('rain-status');
    
    if (data.rainDetected) {
        rainVal.innerText = "RAIN ACTIVE";
        rainVal.classList.add('text-blue');
        rainStatus.innerText = "Irrigation Blocked";
        rainStatus.className = "status-pill text-red bg-red-light";
    } else {
        rainVal.innerText = "DRY";
        rainVal.classList.remove('text-blue');
        rainStatus.innerText = "No Precipitation";
        rainStatus.className = "status-pill text-teal bg-teal-light";
    }

    // 7. Device Information & Footer
    document.getElementById('uptime-val').innerText = formatUptime(data.uptime);
    document.getElementById('rssi-val').innerText = formatRSSI(data.wifiRSSI);
}

// Control SVG dash offsets for moisture progress ring
function setRingProgress(percent) {
    const ring = document.getElementById('moisture-ring');
    const radius = ring.r.baseVal.value;
    const circumference = 2 * Math.PI * radius;
    
    // Clamp between 0 and 100
    const clampedPercent = Math.min(Math.max(percent, 0), 100);
    const offset = circumference - (clampedPercent / 100) * circumference;
    
    ring.style.strokeDasharray = `${circumference} ${circumference}`;
    ring.style.strokeDashoffset = offset;
}

// Convert seconds into human-readable uptime string
function formatUptime(seconds) {
    if (seconds <= 0 || isNaN(seconds)) return "0s";
    
    const d = Math.floor(seconds / (3600 * 24));
    const h = Math.floor((seconds % (3600 * 24)) / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = seconds % 60;
    
    let result = [];
    if (d > 0) result.push(`${d}d`);
    if (h > 0) result.push(`${h}h`);
    if (m > 0) result.push(`${m}m`);
    if (s > 0 || result.length === 0) result.push(`${s}s`);
    
    return result.join(' ');
}

// Clean formatting of RSSI values
function formatRSSI(rssi) {
    if (rssi === 0 || !rssi) return "Disconnected";
    if (rssi > -50) return "Excellent (" + rssi + " dBm)";
    if (rssi > -65) return "Good (" + rssi + " dBm)";
    if (rssi > -80) return "Fair (" + rssi + " dBm)";
    return "Weak (" + rssi + " dBm)";
}

// Toggle Connection Status UI indicator badge
function updateConnectionBadge(online, label) {
    const badge = document.getElementById('connection-badge');
    badge.className = "badge"; // Reset

    if (online) {
        badge.classList.add('badge-online');
        badge.querySelector('.badge-label').innerText = label || "Connected";
    } else {
        if (label === "Simulating") {
            badge.classList.add('badge-connecting');
            badge.querySelector('.badge-label').innerText = "Simulation Mode";
        } else {
            badge.classList.add('badge-offline');
            badge.querySelector('.badge-label').innerText = "Disconnected";
        }
    }
}

// --- Browser Simulation Engine ---
// Simulates the physical hardware feedback loop when no ESP32 is present
let simTime = 0;
let pumpSecs = 0;

function runSimulationStep() {
    simTime += 2;
    currentData.uptime = simTime;

    // Simulate natural daylight lux cycling
    const luxCycle = Math.sin(simTime / 120) * 8000 + 4000;
    currentData.lightIntensity = Math.max(10, luxCycle);

    // Simulate minor temperature & humidity shifts
    currentData.temperature = 22.0 + Math.sin(simTime / 150) * 3;
    currentData.humidity = 60.0 - Math.sin(simTime / 150) * 10;

    // Simulate rain toggle every 4 minutes (240s)
    if (simTime % 240 < 40) {
        currentData.rainDetected = true;
    } else {
        currentData.rainDetected = false;
    }

    // Closed-loop decision simulation (Simulate ESP32 AUTO mode logic)
    if (currentData.mode === "AUTO") {
        if (!currentData.pumpRunning) {
            // Start pump if dry AND no rain
            if (currentData.soilMoisture < 30 && !currentData.rainDetected) {
                currentData.pumpRunning = true;
                pumpSecs = 0;
                console.log("[Sim Engine] Moisture dry (<30%), starting pump.");
            }
        } else {
            // Stop pump if wet target reached OR rain detected
            if (currentData.soilMoisture >= 70) {
                currentData.pumpRunning = false;
                console.log("[Sim Engine] Target reached (>=70%), stopping pump.");
            } else if (currentData.rainDetected) {
                currentData.pumpRunning = false;
                console.log("[Sim Engine] Rain detected during irrigation, emergency stop.");
            }
        }
    }

    // Apply physics effects to moisture depending on pump state
    if (currentData.pumpRunning) {
        pumpSecs += 2;
        // Increase moisture when pump is running
        currentData.soilMoisture += 4.5;
        currentData.soilMoisture = Math.min(100, currentData.soilMoisture);

        // Simulate Watchdog Lockout if pump runs too long (>= 30 seconds)
        if (pumpSecs >= 30) {
            currentData.pumpRunning = false;
            currentData.pumpTimeout = true;
            console.log("[Sim Engine] Safety timeout triggered. Pump locked out.");
        }
    } else {
        // Dry soil out naturally based on light intensity
        const dryRate = currentData.lightIntensity > 5000 ? 0.4 : 0.15;
        currentData.soilMoisture -= dryRate;
        currentData.soilMoisture = Math.max(10, currentData.soilMoisture);
        
        // Reset timeout slowly if moisture is dry and pump is off (cools down)
        if (currentData.pumpTimeout && currentData.soilMoisture < 20) {
            currentData.pumpTimeout = false;
            console.log("[Sim Engine] Safety timeout reset.");
        }
    }
}
