#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

void timeInit();
bool getCurrentTimeString(String &timeStr, String &dateStr);
bool isTimeReady();

#endif