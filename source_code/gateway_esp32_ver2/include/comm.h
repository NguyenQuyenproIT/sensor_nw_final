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

void wifiConnect();
void mqttConnect();
bool parseJSON(String data, float &temp, float &hum, int &soil);
void sendToThingsBoard(float temp, float hum, int soil, float prediction, String ai_state, int led_state);
void mqttLoop();

#endif