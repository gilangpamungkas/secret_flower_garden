#pragma once
#include <HCSR04.h>

// Extern variables to share across modules
extern UltraSonicDistanceSensor distanceSensor;
extern UltraSonicDistanceSensor distanceSensor2;
extern long distance1;
extern long distance2;

void readSensors();
