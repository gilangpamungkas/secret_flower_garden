#include "NeoPixelControl.h"

const int num_leds = 72;
const int payload_size = num_leds * 3;
byte RGBpayload[payload_size];

// Color mapping for 32 levels
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

void setRGBPixel(int r, int g, int b, int pixel) {
    if(pixel < 0 || pixel >= num_leds) return;
    RGBpayload[pixel*3 + 0] = r;
    RGBpayload[pixel*3 + 1] = g;
    RGBpayload[pixel*3 + 2] = b;
}

void clearAllPixels() {
    for(int i=0; i<num_leds; i++)
        setRGBPixel(0,0,0,i);
}
