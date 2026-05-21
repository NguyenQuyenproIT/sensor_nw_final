// #ifndef DISPLAY_H
// #define DISPLAY_H

// #include <Arduino.h>

// void tftInit();
// void drawStaticUI();
// void updateValues(float temp, float hum, int soil, const String &status);
// void showSensorError();
// void restoreNormalUI();

// // thêm 2 dòng này
// uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
// void drawText(int x, int y, const String& text, uint16_t color, uint16_t bg, uint8_t scale = 2);

// #endif

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>

void tftInit();
void drawStaticUI();
void updateValues(float temp, float hum, int soil, const String &status);
void showSensorError();
void restoreNormalUI();
void updateTimeDisplay();

// thêm 2 dòng này
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b);
void drawText(int x, int y, const String& text, uint16_t color, uint16_t bg, uint8_t scale = 2);

#endif