#pragma once
#include <Arduino.h>

extern const int num_leds;
extern const int payload_size;
extern byte RGBpayload[];

struct Color { byte r, g, b; };
extern Color colorMap[32];

void setRGBPixel(int r, int g, int b, int pixel);
void clearAllPixels();
