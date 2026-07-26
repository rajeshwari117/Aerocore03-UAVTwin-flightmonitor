# AeroCore 03 — UAV Twin

A real-time UAV digital twin and flight monitoring system built around an **ESP32 + MPU6050**, combining embedded sensor fusion, live HTTP telemetry, and a browser-based 3D UAV visualization.

The goal of AeroCore 03 is to create a software representation of a physical UAV attitude system that responds to real sensor data rather than relying only on simulated motion.

## Project Overview

AeroCore 03 continuously reads orientation data from an MPU6050 connected to an ESP32, estimates **roll and pitch using sensor fusion**, and exposes the processed data through a lightweight HTTP server.

A browser-based 3D interface consumes the telemetry and visualizes the UAV's orientation in real time.

```text
          MPU6050
             │
             │ I²C
             ▼
           ESP32
             │
      Sensor Fusion
             │
      Roll / Pitch Data
             │
        HTTP /data
             │
             ▼
     AeroCore 03 Web UI
             │
             ▼
       3D UAV Digital Twin
```

## Current Features

- **Real-time MPU6050 sensor acquisition**
- **I²C communication between ESP32 and MPU6050**
- **Complementary-filter-based roll and pitch estimation**
- **Steady sensor update loop at approximately 66 Hz**
- **ESP32 Wi-Fi connectivity**
- **Lightweight HTTP server running on port 80**
- **Live JSON telemetry through `/data`**
- **Browser-based UAV digital twin visualization**
- **Real sensor data driving the digital twin instead of purely simulated values**

##  Sensor Fusion

The ESP32 reads:

- Accelerometer X, Y, Z
- Gyroscope X, Y, Z

Accelerometer angles provide a gravity-based reference while gyroscope measurements provide responsive angular-rate information.

A complementary filter combines both:

```text
Gyroscope ───────┐
                 ├──► Complementary Filter ──► Roll / Pitch
Accelerometer ───┘
```

Current filter weighting:

```text
98% Gyroscope
 2% Accelerometer
```

This provides a responsive and relatively stable orientation estimate for the digital twin.

## 📡 Telemetry

The ESP32 hosts a simple HTTP server.

### Root endpoint

```text
GET /
```

Returns:

```text
AeroTwin ESP32 is running. Visit /data for JSON.
```

### Live data endpoint

```text
GET /data
```

Example response:

```json
{
  "roll": 12.45,
  "pitch": -3.21,
  "t": 12345
}
```

The `t` value represents the ESP32 millisecond timestamp.

## 🖥️ Digital Twin

The web interface acts as the visual layer of the system.

The digital twin receives live telemetry from the ESP32 and uses the orientation values to represent the UAV's current attitude.

```text
Physical Sensor State
        ↓
     ESP32
        ↓
   HTTP Telemetry
        ↓
    Web Interface
        ↓
   3D UAV Twin
```

This creates a direct relationship between the physical sensing layer and the digital representation.

## 🛠️ Hardware

### Required

- ESP32 development board
- MPU6050 IMU
- Breadboard
- Jumper wires
- USB cable

### MPU6050 → ESP32

| MPU6050 | ESP32 |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

## 💻 Software & Technologies

- **C++ / Arduino**
- **ESP32**
- **MPU6050**
- **I²C**
- **Wi-Fi**
- **HTTP / REST-style JSON endpoint**
- **HTML**
- **CSS**
- **JavaScript**
- **3D Web Visualization**


## 🎯 Project Objective

AeroCore 03 is designed to demonstrate the integration of:

**Embedded Systems + Sensor Fusion + Wireless Telemetry + Web Technologies + 3D Visualization + UAV Flight Monitoring**

The long-term objective is to develop a small-scale UAV digital-twin platform where physical sensor behaviour is reflected in a live digital model and flight data can be monitored, recorded, analysed, and replayed.

## 👩‍💻 Author

**Rajeshwari**

Computer Engineering

---

### Project Series

**AeroCore 01** — UAV Flight Controller  
**AeroCore 02** — UAV Payload Stabilization  
**AeroCore 03** — UAV Twin Flight Monitoring
