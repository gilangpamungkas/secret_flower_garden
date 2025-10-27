#include <SPI.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <HCSR04.h>
#include "arduino_secrets.h"  // Contains SSIDs, passwords, and MQTT credentials

// *******************************************************************
// 1. PIN DEFINITIONS AND HARDWARE SETUP
// *******************************************************************
const int SERVO_PIN = 2; // Digital pin for the Servo Motor control.
const int TRIG_PIN_1 = 6; // Trigger pin for the first distance sensor.
const int ECHO_PIN_1 = 7; // Echo pin for the first distance sensor.
const int TRIG_PIN_2 = 5; // Trigger pin for the second distance sensor.
const int ECHO_PIN_2 = 4; // Echo pin for the second distance sensor.

Servo servoMotor;
// Initialize two UltraSonicDistanceSensor objects for a wider field of view.
UltraSonicDistanceSensor distanceSensor1(TRIG_PIN_1, ECHO_PIN_1);
UltraSonicDistanceSensor distanceSensor2(TRIG_PIN_2, ECHO_PIN_2);
long distance1 = 0, distance2 = 0; // Global variables to store the last measured distances (cm).

// *******************************************************************
// 2. MQTT & LED CONSTANTS
// *******************************************************************
WiFiClient wifiClient;
PubSubClient client(wifiClient);

const int NUM_LEDS = 72;
const int PIXEL_COLOR_CHANNELS = 3; // RGB channels (Red, Green, Blue)
const int PAYLOAD_SIZE = NUM_LEDS * PIXEL_COLOR_CHANNELS;
byte RGBpayload[PAYLOAD_SIZE];

// The unique identifier for this specific luminaire device.
String lightId = "21"; 
String mqtt_topic; // The full MQTT topic string for publishing.

struct Color { byte r, g, b; };
// A pre-defined map of 32 colors, primarily organized by hue.
// Used to visually map the combination of object angle and distance.
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

// *******************************************************************
// 3. COLOR MAPPING CONSTANTS (For Logic Clarity)
// *******************************************************************
// These constants define the bins used to map the distance and angle to one of the 32 colors.
const int SWEEP_PERIOD_MS = 10000; // The duration of one full detection cycle (10 seconds).
const int SERVO_STEP_DELAY_MS = 3; // Delay between each servo step to allow sensor readings.
const int MAX_DISTANCE_INIT_CM = 999; // Initial value for minimum distance tracking.

// Mapping parameters for `colorMap` index calculation:
const int ANGLE_BIN_SIZE = 30; // Divides the 180-degree sweep into 6 angle bins (180/30 = 6).
const int DIST_BIN_SIZE = 10;  // Creates distance bins (0-10cm, 10-20cm, etc.).
const int DIST_BINS_PER_ANGLE = 4; // This is the multiplier from your original code (4).
const int MAX_COLOR_INDEX = (sizeof(colorMap)/sizeof(Color)) - 1; // Maximum index is 31.


// *******************************************************************
// FUNCTION: startWifi()
// DESCRIPTION: Iterates through the stored SSIDs and passwords in 'arduino_secrets.h'
// and attempts to connect to the first available network. Halts if no WiFi module is found.
// PARAMETERS: None
// RETURNS: None. Exits if WiFi module is missing; returns on successful connection.
// *******************************************************************
void startWifi() {
  if (WiFi.status() == WL_NO_MODULE) { 
    Serial.println("Error: WiFi module not found! Check hardware connection."); 
    while(true); 
  }

  for(int i=0; i<numNetworks; i++){
    Serial.print("Trying SSID: "); Serial.println(ssids[i]);
    int status = WiFi.begin(ssids[i], passwords[i]);
    unsigned long startAttempt = millis();
    
    // Wait for connection for up to 10 seconds per network attempt.
    while(status != WL_CONNECTED && millis() - startAttempt < 10000){
      Serial.print(".");
      delay(500);
      status = WiFi.status();
    }
    
    if(status == WL_CONNECTED){
      Serial.println("\nWiFi connected successfully!");
      Serial.print("IP: "); Serial.println(WiFi.localIP());
      return;
    } else {
      Serial.println("\nFailed to connect, trying next network...");
    }
  }
  Serial.println("Could not connect to any specified WiFi network.");
}

// *******************************************************************
// FUNCTION: reconnectMQTT()
// DESCRIPTION: Attempts to establish a connection to the MQTT broker using the
// credentials defined in 'arduino_secrets.h'. Retries every 5 seconds until connected.
// PARAMETERS: None
// RETURNS: None
// *******************************************************************
void reconnectMQTT() {
  while(!client.connected()){
    Serial.print("Connecting to MQTT...");
    // Use a unique Client ID based on the luminaire ID.
    String clientId = "ArduinoClient-" + lightId; 
    
    if(client.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");
    } else { 
      Serial.print("failed, rc="); 
      Serial.print(client.state()); 
      Serial.println(" retrying in 5 seconds..."); 
      delay(5000);
    }
  }
}

// ---------------- Servo & Sensor Helper Functions ----------------

// *******************************************************************
// FUNCTION: initServo()
// DESCRIPTION: Initializes the servo motor by attaching it to the specified pin 
// and setting its initial position to 0 degrees (start of sweep).
// PARAMETERS: None
// RETURNS: None
// *******************************************************************
void initServo() { 
  servoMotor.attach(SERVO_PIN); 
  servoMotor.write(0); 
  delay(1000); 
}

// *******************************************************************
// FUNCTION: readSensors()
// DESCRIPTION: Measures the distance (in cm) from both ultrasonic sensors 
// and updates the global state variables 'distance1' and 'distance2'.
// PARAMETERS: None
// RETURNS: None (updates global state)
// *******************************************************************
void readSensors() { 
  distance1 = distanceSensor1.measureDistanceCm(); 
  distance2 = distanceSensor2.measureDistanceCm(); 
}

// *******************************************************************
// FUNCTION: set_RGB_pixel()
// DESCRIPTION: Sets the Red, Green, and Blue values for a specific pixel 
// in the global RGB payload buffer.
// PARAMETERS: r, g, b (int): 0-255 color values. pixel (int): The index of the LED.
// RETURNS: None (updates global state: RGBpayload)
// *******************************************************************
void set_RGB_pixel(int r,int g,int b,int pixel){
  RGBpayload[pixel*PIXEL_COLOR_CHANNELS] = r; 
  RGBpayload[pixel*PIXEL_COLOR_CHANNELS+1] = g; 
  RGBpayload[pixel*PIXEL_COLOR_CHANNELS+2] = b;
}

// *******************************************************************
// FUNCTION: clearAllPixels()
// DESCRIPTION: Sets all LEDs in the buffer to black (off).
// PARAMETERS: None
// RETURNS: None
// *******************************************************************
void clearAllPixels(){
  for(int i=0;i<NUM_LEDS;i++) set_RGB_pixel(0,0,0,i);
}

// *******************************************************************
// FUNCTION: fillAllPixels()
// DESCRIPTION: Sets all LEDs in the buffer to a single specified color.
// PARAMETERS: c (Color struct): The Color to fill all pixels with.
// RETURNS: None
// *******************************************************************
void fillAllPixels(Color c){
  for(int i=0;i<NUM_LEDS;i++) set_RGB_pixel(c.r,c.g,c.b,i);
}

// *******************************************************************
// FUNCTION: moveServoFullSweep()
// DESCRIPTION: Performs a 10-second continuous servo sweep (0 to 180 and back).
// During the sweep, it tracks the shortest distance measured and the angle at which it occurred.
// After the 10s period, it maps this shortest distance/angle to a color and publishes it via MQTT.
// PARAMETERS: None
// RETURNS: None
// *******************************************************************
void moveServoFullSweep() {
  static unsigned long periodStart = millis();
  // Initialize tracking variables to worst-case values at the start of the 10s period.
  static long shortestDistance1 = MAX_DISTANCE_INIT_CM, shortestDistance2 = MAX_DISTANCE_INIT_CM;
  static int shortestAngle1 = 0, shortestAngle2 = 0;

  // Sweep 1: 0 degrees -> 180 degrees
  for(int angle=0; angle<=180; angle++){
    servoMotor.write(angle);
    readSensors();

    // Logic to continuously track the closest object seen by each sensor
    if(distance1 < shortestDistance1){ shortestDistance1 = distance1; shortestAngle1 = angle; }
    if(distance2 < shortestDistance2){ shortestDistance2 = distance2; shortestAngle2 = angle; }

    client.loop(); // Keep MQTT connection alive during the sweep
    delay(SERVO_STEP_DELAY_MS);

    if(millis() - periodStart >= SWEEP_PERIOD_MS) break; // Exit if 10s is up
  }

  // Sweep 2: 180 degrees -> 0 degrees
  for(int angle=180; angle>=0; angle--){
    servoMotor.write(angle);
    readSensors();

    // Continue tracking the shortest distance
    if(distance1 < shortestDistance1){ shortestDistance1 = distance1; shortestAngle1 = angle; }
    if(distance2 < shortestDistance2){ shortestDistance2 = distance2; shortestAngle2 = angle; }

    client.loop(); // Keep MQTT connection alive
    delay(SERVO_STEP_DELAY_MS);

    if(millis() - periodStart >= SWEEP_PERIOD_MS) break; // Exit if 10s is up
  }

  // --- Post-Sweep Analysis & Action ---
  
  // 1. Determine the overall closest object and its angle
  long minDist = (shortestDistance1 < shortestDistance2) ? shortestDistance1 : shortestDistance2;
  int angleAtMin = (shortestDistance1 < shortestDistance2) ? shortestAngle1 : shortestAngle2;

  // 2. Map distance and angle to a color index (CRITICAL LOGIC)
  // Index = (Angle Bin) * (Distance Bins per Angle) + (Distance Bin)
  // Angle Bin: 0-5 (0-30, 30-60, etc.)
  // Distance Bin: 0-3 (0-10, 10-20, 20-30, >30)
  int colorIndex = (angleAtMin / ANGLE_BIN_SIZE) * DIST_BINS_PER_ANGLE + (minDist / DIST_BIN_SIZE); 
  
  // Clamp the index to the valid range (0-31)
  if(colorIndex > MAX_COLOR_INDEX) colorIndex = MAX_COLOR_INDEX; 

  // 3. Fill the LEDs with the determined color
  fillAllPixels(colorMap[colorIndex]);

  // 4. Publish LED state to MQTT
  if(client.connected()) client.publish(mqtt_topic.c_str(), RGBpayload, PAYLOAD_SIZE);

  // 5. Serial output for debugging and external visualization (e.g., Processing)
  Serial.print(angleAtMin);
  Serial.print(",");
  Serial.print(minDist);
  Serial.print(",");
  Serial.println(colorIndex);

  // 6. Reset for the next 10-second period
  shortestDistance1 = MAX_DISTANCE_INIT_CM; 
  shortestDistance2 = MAX_DISTANCE_INIT_CM;
  shortestAngle1 = 0; 
  shortestAngle2 = 0;
  periodStart = millis();
}

// *******************************************************************
// Arduino Core: setup()
// *******************************************************************
void setup(){
  Serial.begin(9600);
  startWifi();
  initServo();
  
  // Build the full, unique MQTT topic string.
  mqtt_topic = "student/CASA0014/luminaire/" + lightId; 
  client.setServer(MQTT_SERVER, MQTT_PORT);
  reconnectMQTT();

  // Initial LED state: Solid Blue (0,0,255) to indicate initialization complete.
  fillAllPixels({0,0,255});
  client.publish(mqtt_topic.c_str(), RGBpayload, PAYLOAD_SIZE);
}

// *******************************************************************
// Arduino Core: loop()
// *******************************************************************
void loop(){
  // Maintain connectivity
  if(WiFi.status()!=WL_CONNECTED) startWifi();
  if(!client.connected()) reconnectMQTT();
  
  client.loop(); // Process incoming MQTT messages and keep connection alive
  moveServoFullSweep(); // Execute the main logic cycle
}