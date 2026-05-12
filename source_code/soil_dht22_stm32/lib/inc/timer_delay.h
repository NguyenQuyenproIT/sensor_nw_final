#ifndef __TIMER_DELAY_H
#define __TIMER_DELAY_H

#include "stm32f10x.h"

void TIM2_Config(void);
void delay_us(uint16_t us);
void delay_ms(uint16_t ms);

#endif
