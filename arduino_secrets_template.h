#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

// =============================================================
//  Arduino Secrets Template
//  -------------------------------------------------------------
//  This file stores Wi-Fi and MQTT credentials used by your
//  IoT device. Replace placeholder values with your actual
//  credentials. DO NOT share sensitive credentials publicly.
// =============================================================

// ---------------- Wi-Fi Configuration ----------------
// List of Wi-Fi SSIDs and passwords your device can connect to.
// Example: "Home_Network", "MyHotspot", etc.
const char* ssids[] = { "YOUR_WIFI_SSID_1", "YOUR_WIFI_SSID_2" };
const char* passwords[] = { "YOUR_WIFI_PASSWORD_1", "YOUR_WIFI_PASSWORD_2" };
const int numNetworks = 2;

// ---------------- MQTT Configuration ----------------
// MQTT broker settings. Replace with your server credentials.
#define MQTT_SERVER "your.mqtt.server"
#define MQTT_PORT 1883              // Usually 1883 or 8883 (for SSL)
#define MQTT_USER "your_username"
#define MQTT_PASS "your_password"

#endif  // ARDUINO_SECRETS_H
