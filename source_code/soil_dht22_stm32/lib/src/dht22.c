#include "dht22.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "timer_delay.h"

static uint16_t temp_raw;
static uint16_t hum_raw;

static uint8_t DHT22_Read_Byte(void)
{
    uint8_t i, byte = 0;

    for (i = 0; i < 8; i++)
    {
        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0);

        TIM_SetCounter(TIM2, 0);

        while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1);

        if (TIM_GetCounter(TIM2) > 40)
            byte = (byte << 1) | 1;
        else
            byte = (byte << 1);
    }

    return byte;
}

void DHT22_GPIO_Config(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &gpio);
}

void DHT22_Start(void)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Pin = GPIO_Pin_12;
    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &gpio);

    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    delay_ms(20);

    GPIO_SetBits(GPIOB, GPIO_Pin_12);
    delay_us(30);

    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &gpio);
}

uint8_t DHT22_Read_Data(float *humidity, float *temperature)
{
    uint8_t byte[5];
    uint8_t i;
    uint32_t timeout = 0;

    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1)
    {
        delay_us(1);
        if (++timeout > 100) return 0;
    }

    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0)
    {
        delay_us(1);
        if (++timeout > 100) return 0;
    }

    timeout = 0;
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1)
    {
        delay_us(1);
        if (++timeout > 100) return 0;
    }

    for (i = 0; i < 5; i++)
        byte[i] = DHT22_Read_Byte();

    if ((uint8_t)(byte[0] + byte[1] + byte[2] + byte[3]) != byte[4])
        return 0;

    hum_raw  = (byte[0] << 8) | byte[1];
    temp_raw = (byte[2] << 8) | byte[3];

    *humidity = hum_raw / 10.0f;

    if (temp_raw & 0x8000)
    {
        temp_raw &= 0x7FFF;
        *temperature = -(temp_raw / 10.0f);
    }
    else
    {
        *temperature = temp_raw / 10.0f;
    }

    return 1;
}


