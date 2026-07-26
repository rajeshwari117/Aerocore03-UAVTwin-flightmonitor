# AeroTwin — Real-Time Drone Digital Twin

Final project in the AeroCore series. An ESP32 and MPU6050 track real-time
orientation and stream it to a browser, where a 3D UAV model, an aviation-style
attitude indicator, and a simulated failsafe monitoring system all mirror the
physical sensor live — with flight recording, replay, and auto-generated
flight reports.

Series context: **AeroCore01** (PID flight controller) → **AeroCore02**
(payload stabilization) → **AeroTwin** (real-time digital twin + flight
monitoring), closing the loop from "fly a drone" to "model and monitor one
digitally in real time."

---

## What it does

- Reads live orientation (roll, pitch) from an MPU6050 using a complementary
  filter for sensor fusion
- Streams that data over WiFi via HTTP polling to a browser dashboard
- Renders a 3D UAV model (X-configuration quadcopter, spinning propellers,
  camera gimbal) that tilts in real time to match the physical sensor
- Displays a cockpit-style **artificial horizon** instrument
- Runs a **Flight Intelligence / Failsafe state machine**: classifies the
  system as `NORMAL`, `WARNING`, or `FAILSAFE ACTIVE` based on rolling
  stability analysis, absolute tilt limits, frozen-sensor detection, and
  telemetry timeout
- Supports **Record → Stop → Replay** of a flight, with a scrubber and an
  auto-generated **Flight Report** (duration, max roll/pitch, warning count,
  failsafe events, telemetry loss %, stability score)
- Offers **Chase Cam** and **FPV Cam** views, plus a draggable camera-angle
  gizmo to orbit the 3D view freely

---

## Hardware

All reused from AeroCore01/02 — nothing new to buy.

| Component | Purpose |
|---|---|
| ESP32 dev board | Main controller |
| MPU6050 | Accelerometer + gyroscope (I2C address `0x68`) |
| SSD1306 OLED *(optional)* | Local status display, not required for the dashboard to work |

### Wiring

| MPU6050 pin | ESP32 pin |
|---|---|
| VCC | 3.3V |
| GND | GND |
| SCL | GPIO 22 |
| SDA | GPIO 21 |
| AD0 | GND (sets I2C address to `0x68`) |

---

## Software architecture

```
┌─────────────────────────────────────────────┐
│  ESP32 + MPU6050                             │
│  1. Read raw accel + gyro (I2C)              │
│  2. Complementary filter -> roll, pitch      │
│  3. Serve /data as JSON over HTTP            │
│     (fixed ~66Hz fusion loop, decoupled      │
│      from request timing)                    │
└───────────────────┬───────────────────────────┘
                     │  HTTP polling, ~100ms interval
                     ▼
┌─────────────────────────────────────────────┐
│  Browser Dashboard (HTML + Three.js)         │
│  - Polls /data, updates 3D model live        │
│  - Artificial horizon + gauge dials          │
│  - Flight Intelligence state machine         │
│  - Record / Replay / Flight Report           │
│  - Chase / FPV camera + orbit gizmo          │
└─────────────────────────────────────────────┘
```

**Firmware:** `firmware/AeroTwin_Full_HTTP.ino` — Arduino framework, uses
`Wire.h`, `WiFi.h`, `WebServer.h`. No external libraries beyond the ESP32
board package.

**Dashboard:** `aerotwin_3d_v6.html` — plain HTML/CSS/JS, Three.js (r128) via
CDN, no build step, no other dependencies.


## Sensor fusion

Raw accelerometer angle is noisy; raw gyro angle drifts over time. A
complementary filter blends both:

```
angle = 0.98 * (angle + gyro_rate * dt) + 0.02 * (accel_angle)
```

Sensor fusion runs on a **fixed ~66Hz timer inside the ESP32's main loop**,
independent of when the browser happens to request `/data`. This was a
deliberate fix — an earlier version computed orientation only on-demand
inside the HTTP handler, which tied the filter's timing to network jitter
and caused visibly laggy, slow-to-settle orientation on an unstable network.

**Known limitation:** yaw is not implemented with absolute reference (no
magnetometer), so only roll and pitch are tracked. A magnetometer (e.g.
QMC5883L) or a 9-DOF IMU (e.g. BNO055) would be the natural next step for
full heading tracking.

---

## Flight Intelligence / Failsafe system

A rolling window (last ~30 samples, ~3 seconds) of roll/pitch readings is
used to compute standard deviation — a measure of how erratic the motion is,
not just its instantaneous value. This is combined with:

- **Frozen sensor detection** — same exact reading repeated 5+ times in a
  row is treated as a likely stuck I2C read
- **Absolute attitude limit** — tilt beyond ±45°
- **Telemetry timeout** — a failed/unanswered poll request

into a three-state machine: `NORMAL → WARNING → FAILSAFE`, each with a
specific reason displayed in the UI.


## Record, Replay, and Flight Report

- **Record** captures every polled sample (timestamp, roll, pitch, state,
  reason) into memory in the browser
- **Replay** plays the captured sequence back on the 3D model at its
  original ~100ms sample rate, with a scrubber for seeking to any point
- Stopping a recording automatically generates a **Flight Report**:
  duration, max roll, max pitch, warning count, failsafe event count,
  telemetry loss percentage, and an overall stability score


