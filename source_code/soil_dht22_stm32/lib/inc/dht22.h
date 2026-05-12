#ifndef __DHT22_H
#define __DHT22_H

#include "stm32f10x.h"

void DHT22_GPIO_Config(void);
void DHT22_Start(void);
uint8_t DHT22_Read_Data(float *humidity, float *temperature);

#endif

