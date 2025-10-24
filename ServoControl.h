#pragma once
#include <Servo.h>

extern Servo servoMotor;

void initServo();
void moveServoSmooth(int startAngle, int endAngle, int stepDelay, int &detectedAngle1, long &minDistance1, int &detectedAngle2, long &minDistance2);
void performFullSweep();
