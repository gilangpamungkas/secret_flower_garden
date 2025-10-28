# 🌙 Vespera: Connected Luminaire with Dual Ultrasonic Sensing and MQTT Control

**Author:** Gilang Pamungkas  
**Course:** CASA0014 – Prototyping the Internet of Things (UCL)  
**Year:** 2025  

---

## ✨ Concept

**Vespera** — derived from the Latin word for “evening” — represents the *moment when a single point of light emerges in twilight*, symbolizing the passage of time and transition from day to night.  

This project reimagines Vespera as an **interactive luminaire** that detects proximity and motion using **dual ultrasonic sensors** and translates these spatial readings into **color gradients** across 72 NeoPixels. The data are transmitted via **MQTT** to a broker, allowing remote visualization through web interfaces or connection to the original Vespera installation.

---

## 🧩 System Overview

This repository builds on the CASA0014 workshop ecosystem, extending its functionality with a servo-based scanning mechanism.

| Component | Description |
|------------|-------------|
| **Vespera Luminaire (Arduino MKR1010)** | Central light installation receiving RGB color data via MQTT and displaying them on 72 LEDs. |
| **Dual Ultrasonic Scanner (This Project)** | A WiFi-enabled Arduino controller that measures distance from two ultrasonic sensors during a servo sweep, maps readings to colors, and publishes to MQTT topic `student/CASA0014/luminaire/[id]`. |
| **MQTT Broker** | `mqtt.cetools.org` — central communication hub between controllers, visualisers, and the Vespera installation. |
| **Web Visualiser** | Browser-based interface that subscribes to the same MQTT topics and mirrors the LED colors in real time. |
| **Python & Dial Controllers** | Alternative interfaces for publishing color data to the same MQTT topic. |

---

## 🛠️ Features

- **WiFi-enabled MQTT publishing**
- **Dual ultrasonic ranging (HCSR04)**
- **Servo-based 180° scanning**
- **Dynamic color mapping** based on distance and angle
- **Full payload replication** for compatibility with CASA0014 Vespera
- **Reproducible, open-source hardware and code**

---

## 🗂️ Repository Contents

---
Vespera/
|-- src/
|   |-- secret_flower_garden.ino       # Main Arduino sketch
|   |-- arduino_secrets_template.h     # Safe template for credentials
|   |-- images/
|       |-- prototype.jpg              # Project photo
|       |-- wiring.png                  # Circuit wiring
|       |-- color_mapping.png           # Color logic diagram
|       |-- demo.gif                    # Demo animation
|-- .gitignore
|-- LICENSE
|-- README.md

---

## ⚙️ Hardware Setup

| Component | Model | Role |
|------------|--------|------|
| Microcontroller | Arduino MKR WiFi 1010 | WiFi + MQTT client |
| Servo Motor | SG90 | Sweeps dual sensors 0°–180° |
| Ultrasonic Sensors | HC-SR04 (x2) | Measure distance |
| LEDs | WS2812 (72 NeoPixels) | Color output |
| Power | 5V regulated | Supply for all modules |

**Pin configuration** (defined in code):  
- Servo: D2  
- Ultrasonic #1: Trig D6 / Echo D7  
- Ultrasonic #2: Trig D5 / Echo D4  

---

