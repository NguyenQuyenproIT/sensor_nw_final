#ifndef __UART_H
#define __UART_H

#include "stm32f10x.h"
#include <stdio.h>
#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART

void UART1_Config(void);
void UART2_Config(void);
void UART2_SendString(char *str);
void UART2_SendJSON(float temp, float hum, int soil);

#endif
