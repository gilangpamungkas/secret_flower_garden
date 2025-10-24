#include "Sensors.h"

// Define two HC-SR04 sensors
UltraSonicDistanceSensor distanceSensor(6, 7);
UltraSonicDistanceSensor distanceSensor2(5, 4);

long distance1 = 0;
long distance2 = 0;

void readSensors() {
    distance1 = distanceSensor.measureDistanceCm();
    distance2 = distanceSensor2.measureDistanceCm();
}
