# 🌱 IoT-Based Automatic Plant Water System and Sensor

An **ESP32-based smart irrigation system** that utilizes a **capacitive soil moisture sensor** to control a water pump based on soil moisture levels. It also features an **ultrasonic sensor** that monitors the water tank level and notifies when water is low. The system has a **16x2 LCD** for local feedback and **Blynk IoT** integration for real-time remote monitoring, data logging, and manual pump control over WiFi.

---

## ✨ Features
- Automatic watering using a **capacitive soil moisture sensor**
- **Water tank level detection** via ultrasonic sensor (prevents pump burnout)
- **ESP32 WiFi connectivity** with **Blynk IoT dashboard** for real-time moisture levels, water tank status, and manual pump override
- **16x2 LCD live display** (moisture %, pump state, warnings)
- **Smart watering logic** with absorption delays to prevent overwatering
- **Push notifications** for low water and pump activation alerts via Blynk

---

## 🛠️ Hardware Requirements
- ESP32 Development Board
- Capacitive Soil Moisture Sensor (v2.0 recommended)
- HC-SR04 Ultrasonic Sensor(or similar)
- 5V Relay Module (1-channel)
- Submersible Water Pump + Vinyl Tubing
- 16x2 LCD Display (Standard 16-pin)
- Power Supply Module 
- Resistors: 10kΩ and 5kΩ (for voltage divider)
- Jumper Wires & Breadboard

---

## ⚡ Wiring Notes
- **Soil moisture sensor** (AOUT) → ESP32 GPIO 34
- **Relay IN** → ESP32 GPIO 15 (active LOW)
- **Ultrasonic sensor**
   - **TRIG** → GPIO 33
   - **ECHO** → GPIO 32 (Two resistors (~10kΩ & ~5kΩ) used in series to drop 5V → ~3.3V)
- **LCD pins** → `RS=13, E=12, D4=14, D5=27, D6=26, D7=25`
- **Pump Grounding** → The Pump ground wire is connected directly to the Power Module GND (not the breadboard rail) to reduce interference.
- **External Power Module** → Connected to a 9V power supply.

---

## 📐 Circuit Schematic
<img width="2771" height="2902" alt="Image" src="https://github.com/user-attachments/assets/8df847bf-026d-46e3-be50-4ae8acd559f2" />
---

## 🎥 Demo
- [Watch Full Demo on YouTube](https://youtube.com/shorts/LfqWjdnBEMg?feature=share)
- [Watch Ultrasonic and Low Water Warning Demo on YouTube](https://youtube.com/shorts/u12fPIr-B-0?feature=share)
- [Watch Blynk IoT Dashboard and Notification Demo on YouTube](https://youtube.com/shorts/bibf3A-_fsU?feature=share) 

---

## 💻 Software Setup
### PlatformIO
Add the following to your `platformio.ini`:
```ini
lib_deps =
  fmalpartida/LiquidCrystal@^1.5.0
  blynkkk/Blynk@^1.3.2
  marcoschwartz/LiquidCrystal_I2C@^1.1.4
```

### WiFi & Blynk Configuration
Create a `secrets.h` file(or a file name of your choice) using this template:
```
// secrets.h
#ifndef SECRETS_H
#define SECRETS_H

#define WIFI_SSID "your_wifi_name"
#define WIFI_PASSWORD "your_wifi_password"

#define BLYNK_TEMPLATE_ID "your_template_id"
#define BLYNK_TEMPLATE_NAME "Plant Watering"
#define BLYNK_AUTH_TOKEN "your_auth_token"

#endif
```

### Blynk Dashboard Setup
**Configure the following virtual pins in your Blynk app:**
- **Virtual pin**: V0, V1, V2, V3, V4
  - **V0** → Gauge(0-100%): Soil moisture percentage
  - **V1** → Value Display: Water level (cm)
  - **V2** → LED: Pump status indicator
  - **V3** → Value Display: Raw soil sensor value
  - **V4** → Button(Switch): Manual pump control
 
    
**Events (for notifications):**

    Note: Blynk notification event code must match the 1st parameter of logEvent(p1,p2)
  - **Manual Pump Activation Alert:**
     - Event Code (in Blynk): `pump_manual`
     - Code: `Blynk.logEvent("pump_manual", "Manual pump activated");`
  - **Low Water Level Alert:**
     - Event Code (in Blynk): `low_water`
     - Code: `Blynk.logEvent("low_water", "Water tank is empty!");`

### Calibration
1. Place the soil sensor in dry soil and note the raw value (Serial Monitor)
2. Place the sensor in wet soil(water the soil to your liking) and note the raw value after 10 minutes of soaking.
3. Update these constants in the `main.cpp`:
```
const int SOIL_DRY_THRESHOLD = 2100; // Increase if pump turns on too late
const int SOIL_WET_THRESHOLD = 1550; // Decrease if pump stays on too long
```
