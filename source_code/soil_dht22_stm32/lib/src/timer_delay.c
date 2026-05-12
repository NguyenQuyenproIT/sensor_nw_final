#include "timer_delay.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void TIM2_Config(void)
{
    TIM_TimeBaseInitTypeDef tim;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    tim.TIM_Prescaler = 72 - 1;   // 1 MHz if system clock = 72 MHz
    tim.TIM_Period = 0xFFFF;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(TIM2, &tim);
    TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t us)
{
    TIM_SetCounter(TIM2, 0);
    while (TIM_GetCounter(TIM2) < us);
}

void delay_ms(uint16_t ms)
{
    while (ms--)
        delay_us(1000);
}


