#include "Secrets.h"
#include "Sensors.h"
#include "NeoPixelControl.h"
#include "ServoControl.h"
#include "MQTTControl.h"

void setup() {
    Serial.begin(9600);

    startWiFi();

    mqtt_topic = "student/CASA0014/luminaire/21";

    client.setServer(mqtt_server, mqtt_port);
    reconnectMQTT();

    initServo();

    // Initialize all LEDs to blue
    for(int i=0; i<num_leds; i++) setRGBPixel(0,0,255,i);
    publishMQTT();
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) startWiFi();
    if (!client.connected()) reconnectMQTT();
    loopMQTT();

    performFullSweep();
}
