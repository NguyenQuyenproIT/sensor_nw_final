#ifndef __ADC_SOIL_H
#define __ADC_SOIL_H

#include "stm32f10x.h"

void ADC_Config(void);
uint16_t Read_ADC(void);
uint16_t Read_ADC_Average(void);
int Soil_Moisture_Percent(void);

#endif

