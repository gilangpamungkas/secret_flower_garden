#include "MQTTControl.h"
#include "Secrets.h"
#include "NeoPixelControl.h"

WiFiClient wifiClient;
PubSubClient client(wifiClient);
String mqtt_topic;

void startWiFi() {
    int status = WL_IDLE_STATUS;
    if (WiFi.status() == WL_NO_MODULE) { Serial.println("WiFi module not found!"); while(true); }
    while (status != WL_CONNECTED) {
        Serial.print("Connecting to "); Serial.println(SECRET_SSID);
        status = WiFi.begin(SECRET_SSID, SECRET_PASS);
        delay(5000);
    }
    Serial.println("WiFi connected! IP: "); Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
    while (!client.connected()) {
        Serial.print("Connecting to MQTT...");
        String clientId = "ArduinoClient-" + mqtt_topic;
        if (client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) Serial.println("connected");
        else { Serial.print("failed, rc="); Serial.print(client.state()); Serial.println(" try again"); delay(5000); }
    }
}

void publishMQTT() {
    if(client.connected()) client.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
}

void loopMQTT() {
    client.loop();
}
