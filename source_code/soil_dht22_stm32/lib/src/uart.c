#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART

#include "uart.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

char uart2_buffer[120];

// Retarget printf to UART1
struct __FILE {
    int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
    return ch;
}

void UART2_Config(void)
{
    GPIO_InitTypeDef gpio_pin;
    USART_InitTypeDef usart_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    gpio_pin.GPIO_Pin = GPIO_Pin_2;
    gpio_pin.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_pin);

    gpio_pin.GPIO_Pin = GPIO_Pin_3;
    gpio_pin.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio_pin);

    usart_init.USART_BaudRate = 115200;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART2, &usart_init);
    USART_Cmd(USART2, ENABLE);
}

void UART1_Config(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef uart;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    // TX PA9
    gpio.GPIO_Pin = GPIO_Pin_9;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // RX PA10
    gpio.GPIO_Pin = GPIO_Pin_10;
    gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &gpio);

    uart.USART_BaudRate = 115200;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
}

void UART2_SendString(char *str)
{
    while (*str)
    {
        USART_SendData(USART2, *str++);
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

void UART2_SendJSON(float temp, float hum, int soil)
{
    sprintf(uart2_buffer,
            "{\"temp\":%.1f,\"hum\":%.1f,\"soil\":%d}\r\n",
            temp, hum, soil);

    UART2_SendString(uart2_buffer);
}


