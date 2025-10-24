#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <HCSR04.h>
#include "arduino_secrets.h"

// ======================================================
//                      GLOBAL VARIABLES
// ======================================================

// --- Servo Configuration ---
Servo servoMotor;   // Controls angular sweep for sensor scanning

// Ultrasonic sensors
UltraSonicDistanceSensor distanceSensor(6, 7);
UltraSonicDistanceSensor distanceSensor2(5, 4);
long distance1 = 0;
long distance2 = 0;

// WiFi
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

// MQTT & LEDs
const char* mqtt_server = "mqtt.cetools.org";
const int mqtt_port = 1884;
const char* mqtt_user = "student";       
const char* mqtt_pass = "ce2021-mqtt-forget-whale"; 

WiFiClient wifiClient;
PubSubClient client(wifiClient);

const int num_leds = 72;
const int payload_size = num_leds * 3;
byte RGBpayload[payload_size];

String lightId = "21";
String mqtt_topic;

// Color mapping (32 colors)
struct Color { byte r, g, b; };
Color colorMap[32] = {
  {255,0,0}, {255,64,0}, {255,128,0}, {255,192,0},
  {255,255,0}, {192,255,0}, {128,255,0}, {64,255,0},
  {0,255,0}, {0,255,64}, {0,255,128}, {0,255,192},
  {0,255,255}, {0,192,255}, {0,128,255}, {0,64,255},
  {0,0,255}, {64,0,255}, {128,0,255}, {192,0,255},
  {255,0,255}, {255,0,192}, {255,0,128}, {255,0,64},
  {128,128,128}, {64,64,64}, {192,192,192}, {255,255,255},
  {128,0,0}, {0,128,0}, {0,0,128}, {128,128,0}
};

// -------------------- Functions --------------------
void startWifi() {
  if (WiFi.status() == WL_NO_MODULE) { Serial.println("WiFi module not found!"); while(true); }
  while (status != WL_CONNECTED) {
    Serial.print("Connecting to "); Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(5000);
  }
  Serial.println("WiFi connected!"); Serial.print("IP: "); Serial.println(WiFi.localIP());
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ArduinoClient-" + lightId;
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) Serial.println("connected");
    else { Serial.print("failed, rc="); Serial.print(client.state()); Serial.println(" try again"); delay(5000); }
  }
}

void initServo() { servoMotor.attach(2); servoMotor.write(0); delay(1000); }

void readSensors() {
  distance1 = distanceSensor.measureDistanceCm();
  distance2 = distanceSensor2.measureDistanceCm();
}

void set_RGB_pixel(int r, int g, int b, int pixel) {
  RGBpayload[pixel*3 + 0] = (byte)r;
  RGBpayload[pixel*3 + 1] = (byte)g;
  RGBpayload[pixel*3 + 2] = (byte)b;
}

void clearAllPixels() { for(int i=0;i<num_leds;i++) set_RGB_pixel(0,0,0,i); }

// -------------------- Smooth servo movement --------------------
void moveServoSmooth(int startAngle, int endAngle, int stepDelay, int &detectedAngle1, long &minDistance1, int &detectedAngle2, long &minDistance2) {
  if(startAngle < endAngle) {
    for(int a=startAngle; a<=endAngle; a++) {
      servoMotor.write(a);
      readSensors();
      if(distance1 < minDistance1) { minDistance1 = distance1; detectedAngle1 = a; }
      if(distance2 < minDistance2) { minDistance2 = distance2; detectedAngle2 = a; }
      client.loop();
      delay(stepDelay);
    }
  } else {
    for(int a=startAngle; a>=endAngle; a--) {
      servoMotor.write(a);
      readSensors();
      if(distance1 < minDistance1) { minDistance1 = distance1; detectedAngle1 = a; }
      if(distance2 < minDistance2) { minDistance2 = distance2; detectedAngle2 = a; }
      client.loop();
      delay(stepDelay);
    }
  }
}

// -------------------- Full 0-180 sweep --------------------
void moveServoFullSweep() {
  int detectedAngle1 = 0;
  int detectedAngle2 = 0;
  long minDistance1 = 999;
  long minDistance2 = 999;

  clearAllPixels();
  int stepDelay = 3; // adjust speed

  // Forward sweep 0 -> 180
  moveServoSmooth(0, 180, stepDelay, detectedAngle1, minDistance1, detectedAngle2, minDistance2);

  // Backward sweep 180 -> 0
  moveServoSmooth(180, 0, stepDelay, detectedAngle1, minDistance1, detectedAngle2, minDistance2);

  // Map angles to LED pixels
  int pixel1 = map(detectedAngle1,0,180,0,num_leds-1);
  int pixel2 = map(detectedAngle2,0,180,0,num_leds-1);

  int colorIndex1 = (detectedAngle1/30)*4 + (minDistance1/10); if(colorIndex1>=32) colorIndex1=31;
  int colorIndex2 = (detectedAngle2/30)*4 + (minDistance2/10); if(colorIndex2>=32) colorIndex2=31;

  set_RGB_pixel(colorMap[colorIndex1].r, colorMap[colorIndex1].g, colorMap[colorIndex1].b, pixel1);
  set_RGB_pixel(colorMap[colorIndex2].r, colorMap[colorIndex2].g, colorMap[colorIndex2].b, pixel2);

  // Publish MQTT once per sweep
  if(client.connected()) {
    client.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
    Serial.print("Sensor1: angle="); Serial.print(detectedAngle1); Serial.print("°, distance="); Serial.print(minDistance1); Serial.println(" cm");
    Serial.print("Sensor2: angle="); Serial.print(detectedAngle2); Serial.print("°, distance="); Serial.print(minDistance2); Serial.println(" cm");
  }
}

// -------------------- Arduino Core --------------------
void setup() {
  Serial.begin(9600);
  startWifi();
  initServo();
  mqtt_topic = "student/CASA0014/luminaire/" + lightId;
  client.setServer(mqtt_server, mqtt_port);
  reconnectMQTT();

  // Initialize LEDs to blue
  for(int i=0;i<num_leds;i++) set_RGB_pixel(0,0,255,i);
  client.publish(mqtt_topic.c_str(), RGBpayload, payload_size);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) startWifi();
  if (!client.connected()) reconnectMQTT();
  client.loop();
  moveServoFullSweep();
}
