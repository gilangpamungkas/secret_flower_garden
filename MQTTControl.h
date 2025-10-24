#pragma once
#include <WiFiNINA.h>
#include <PubSubClient.h>

extern WiFiClient wifiClient;
extern PubSubClient client;
extern String mqtt_topic;

void startWiFi();
void reconnectMQTT();
void publishMQTT();
void loopMQTT();
