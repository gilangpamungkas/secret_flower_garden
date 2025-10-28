# 🌸 Secret Flower Garden  
*A Connected Environment Project for CASA0014 – UCL MSc Connected Environments*

---

## 🧭 Overview
**Secret Flower Garden** is an IoT-based interactive luminaire system that visualizes the proximity and direction of nearby objects using servo-mounted ultrasonic sensors and an RGB LED array.  
The system publishes live color states via **MQTT**, enabling remote visualization and integration with connected environments.

This repository is structured and documented to serve as a **model of clarity, completeness, and reproducibility**, demonstrating best practices for open-source collaboration and research dissemination.

---

## 🧠 Core Concept
The servo performs a 180° sweep while two ultrasonic sensors capture distance data.  
Each detection cycle (10 s) identifies the **closest object**, computes its **angle** and **distance bin**, and maps these to one of 32 pre-defined RGB colors.  
The color is displayed locally via LEDs and simultaneously published to an MQTT broker.

---

## 🧩 System Architecture

| Component | Function |
|------------|-----------|
| **Arduino Nano 33 IoT** | Main controller with built-in Wi-Fi module |
| **HCSR04 Ultrasonic Sensors (x2)** | Measure distances from two directions |
| **Servo Motor** | Rotates sensor pair for a 180° sweep |
| **RGB LED Strip (72 LEDs)** | Displays color representing closest object |
| **MQTT Broker (e.g. mqtt.cetools.org)** | Receives published LED data for remote visualization |

---

## 📁 Repository Structure
