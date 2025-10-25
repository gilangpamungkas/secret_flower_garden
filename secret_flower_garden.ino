#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <HCSR04.h>
#include "arduino_secrets.h"  // contains ssids, passwords, MQTT credentials

// ---------------- Servo & Sensors ----------------
Servo servoMotor;
UltraSonicDistanceSensor distanceSensor(6, 7);
UltraSonicDistanceSensor distanceSensor2(5, 4);
long distance1 = 0, distance2 = 0;

// ---------------- LEDs & MQTT ----------------
WiFiClient wifiClient;
PubSubClient client(wifiClient);

const int num_leds = 72;
const int payload_size = num_leds * 3;
byte RGBpayload[payload_size];
String lightId = "21";
String mqtt_topic;

struct Color { byte r, g, b; };
Color colorMap[32] = {
  {255,0,0},{255,64,0},{255,128,0},{255,192,0},
  {255,255,0},{192,255,0},{128,255,0},{64,255,0},
  {0,255,0},{0,255,64},{0,255,128},{0,255,192},
  {0,255,255},{0,192,255},{0,128,255},{0,64,255},
  {0,0,255},{64,0,255},{128,0,255},{192,0,255},
  {255,0,255},{255,0,192},{255,0,128},{255,0,64},
  {128,128,128},{64,64,64},{192,192,192},{255,255,255},
  {128,0,0},{0,128,0},{0,0,128},{128,128,0}
};

// ---------------- WiFi ----------------
void startWifi() {
  if (WiFi.status() == WL_NO_MODULE) { Serial.println("WiFi module not found!"); while(true); }

  for(int i=0; i<numNetworks; i++){
    Serial.print("Trying SSID: "); Serial.println(ssids[i]);
    int status = WiFi.begin(ssids[i], passwords[i]);
    unsigned long startAttempt = millis();
    while(status != WL_CONNECTED && millis() - startAttempt < 10000){
      Serial.print(".");
      delay(500);
      status = WiFi.status();
    }
    if(status == WL_CONNECTED){
      Serial.println("\nWiFi connected!");
      Serial.print("IP: "); Serial.println(WiFi.localIP());
      return;
    } else {
      Serial.println("\nFailed, trying next network...");
    }
  }
  Serial.println("Could not connect to any WiFi network.");
}

// ---------------- MQTT ----------------
void reconnectMQTT() {
  while(!client.connected()){
    Serial.print("Connecting to MQTT...");
    String clientId = "ArduinoClient-" + lightId;
    if(client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) Serial.println("connected");
    else { Serial.print("failed, rc="); Serial.print(client.state()); Serial.println(" try again"); delay(5000);}
  }
}

// ---------------- Servo & Sensor Helpers ----------------
void initServo() { servoMotor.attach(2); servoMotor.write(0); delay(1000); }
void readSensors() { distance1 = distanceSensor.measureDistanceCm(); distance2 = distanceSensor2.measureDistanceCm(); }
void set_RGB_pixel(int r,int g,int b,int pixel){RGBpayload[pixel*3]=r; RGBpayload[pixel*3+1]=g; RGBpayload[pixel*3+2]=b;}
void clearAllPixels(){for(int i=0;i<num_leds;i++) set_RGB_pixel(0,0,0,i);}
void fillAllPixels(Color c){for(int i=0;i<num_leds;i++) set_RGB_pixel(c.r,c.g,c.b,i);}

// ---------------- Fast Servo Sweep with Shortest Distance Logging ----------------
void moveServoFullSweep() {
  static unsigned long periodStart = millis();
  static long shortestDistance1 = 999, shortestDistance2 = 999;
  static int shortestAngle1 = 0, shortestAngle2 = 0;

  // Fast sweep 0 -> 180
  for(int angle=0; angle<=180; angle++){
    servoMotor.write(angle);
    readSensors();

    if(distance1 < shortestDistance1){ shortestDistance1 = distance1; shortestAngle1 = angle; }
    if(distance2 < shortestDistance2){ shortestDistance2 = distance2; shortestAngle2 = angle; }

    client.loop();
    delay(3);

    if(millis() - periodStart >= 10000) break;
  }

  // Fast sweep 180 -> 0
  for(int angle=180; angle>=0; angle--){
    servoMotor.write(angle);
    readSensors();

    if(distance1 < shortestDistance1){ shortestDistance1 = distance1; shortestAngle1 = angle; }
    if(distance2 < shortestDistance2){ shortestDistance2 = distance2; shortestAngle2 = angle; }

    client.loop();
    delay(3);

    if(millis() - periodStart >= 10000) break;
  }

  // After 10s, choose closest object (sensor1 or sensor2)
  long minDist = (shortestDistance1 < shortestDistance2) ? shortestDistance1 : shortestDistance2;
  int angleAtMin = (shortestDistance1 < shortestDistance2) ? shortestAngle1 : shortestAngle2;
  int colorIndex = (angleAtMin/30)*4 + (minDist/10); if(colorIndex>=32) colorIndex=31;

  // Fill all 72 LEDs with that color
  fillAllPixels(colorMap[colorIndex]);

  // Publish to MQTT
  if(client.connected()) client.publish(mqtt_topic.c_str(), RGBpayload, payload_size);

// Serial output (for Processing visualization)
Serial.print(angleAtMin);
Serial.print(",");
Serial.print(minDist);
Serial.print(",");
Serial.println(colorIndex);


  // Reset for next period
  shortestDistance1 = 999; shortestDistance2 = 999;
  shortestAngle1 = 0; shortestAngle2 = 0;
  periodStart = millis();
}

// ---------------- Arduino Core ----------------
void setup(){
  Serial.begin(9600);
  startWifi();
  initServo();
  mqtt_topic = "student/CASA0014/luminaire/" + lightId;
  client.setServer(MQTT_SERVER, MQTT_PORT);
  reconnectMQTT();

  // Initialize LEDs to blue
  fillAllPixels({0,0,255});
  client.publish(mqtt_topic.c_str(),RGBpayload,payload_size);
}

void loop(){
  if(WiFi.status()!=WL_CONNECTED) startWifi();
  if(!client.connected()) reconnectMQTT();
  client.loop();
  moveServoFullSweep();
}
