#ifndef COMM_H
#define COMM_H

#include <Arduino.h>

enum ControlMode
{
    MODE_AI,
    MODE_MANUAL
};

extern ControlMode controlMode;
extern bool manualLedOn;

extern bool pumpOverrideActive;
extern bool pumpDelayActive;

extern unsigned long pumpOverrideStart;
extern unsigned long pumpOverrideDurationMs;

extern unsigned long pumpDelayStart;
extern unsigned long pumpDelayMs;
extern unsigned long pumpDelayDurationMs;

extern ControlMode previousMode;
extern bool previousManualLedOn;

void wifiConnect();
bool mqttConnect();
bool parseJSON(String data, float &temp, float &hum, int &soil);
void sendToThingsBoard(float temp, float hum, int soil, float prediction, String ai_state, int led_state, int sensor_status);
void mqttLoop();

#endif