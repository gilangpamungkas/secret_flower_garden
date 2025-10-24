#include "ServoControl.h"
#include "Sensors.h"
#include "NeoPixelControl.h"
#include "MQTTControl.h"

Servo servoMotor;

void initServo() {
    servoMotor.attach(2);
    servoMotor.write(0);
}

void moveServoSmooth(int startAngle, int endAngle, int stepDelay, int &detectedAngle1, long &minDistance1, int &detectedAngle2, long &minDistance2) {
    if(startAngle < endAngle) {
        for(int a=startAngle; a<=endAngle; a++){
            servoMotor.write(a);
            readSensors();
            if(distance1 < minDistance1){ minDistance1 = distance1; detectedAngle1 = a; }
            if(distance2 < minDistance2){ minDistance2 = distance2; detectedAngle2 = a; }
            loopMQTT();
            delay(stepDelay);
        }
    } else {
        for(int a=startAngle; a>=endAngle; a--){
            servoMotor.write(a);
            readSensors();
            if(distance1 < minDistance1){ minDistance1 = distance1; detectedAngle1 = a; }
            if(distance2 < minDistance2){ minDistance2 = distance2; detectedAngle2 = a; }
            loopMQTT();
            delay(stepDelay);
        }
    }
}

void performFullSweep() {
    int detectedAngle1 = 0;
    int detectedAngle2 = 0;
    long minDistance1 = 999;
    long minDistance2 = 999;

    clearAllPixels();
    int stepDelay = 3;

    moveServoSmooth(0, 180, stepDelay, detectedAngle1, minDistance1, detectedAngle2, minDistance2);
    moveServoSmooth(180, 0, stepDelay, detectedAngle1, minDistance1, detectedAngle2, minDistance2);

    int pixel1 = map(detectedAngle1,0,180,0,num_leds-1);
    int pixel2 = map(detectedAngle2,0,180,0,num_leds-1);

    int colorIndex1 = (detectedAngle1/30)*4 + (minDistance1/10); if(colorIndex1>=32) colorIndex1=31;
    int colorIndex2 = (detectedAngle2/30)*4 + (minDistance2/10); if(colorIndex2>=32) colorIndex2=31;

    setRGBPixel(colorMap[colorIndex1].r, colorMap[colorIndex1].g, colorMap[colorIndex1].b, pixel1);
    setRGBPixel(colorMap[colorIndex2].r, colorMap[colorIndex2].g, colorMap[colorIndex2].b, pixel2);

    publishMQTT();
}
