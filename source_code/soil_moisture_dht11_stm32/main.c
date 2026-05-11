#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"

#include <stdio.h>


// ================= UART1 =================

void UART2_Config(){
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
    usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    usart_init.USART_WordLength = USART_WordLength_8b;
    usart_init.USART_StopBits = USART_StopBits_1;
    usart_init.USART_Parity = USART_Parity_No;
    usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART2, &usart_init);
    USART_Cmd(USART2, ENABLE);
}

void UART1_Config(){

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

struct __FILE{
	int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f){

	USART_SendData(USART1, ch);

	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);

	return ch;
}


// ================= TIMER2 =================

void TIM2_Config(){

	TIM_TimeBaseInitTypeDef tim;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	tim.TIM_Prescaler = 72 - 1;
	tim.TIM_Period = 0xFFFF;
	tim.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &tim);

	TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t us){

	TIM_SetCounter(TIM2, 0);

	while(TIM_GetCounter(TIM2) < us);
}

void delay_ms(uint16_t ms){

	while(ms--)
		delay_us(1000);
}


// ================= DHT22 =================

void DHT22_GPIO_Config(){

	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &gpio);
}

void DHT22_Start(){

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

uint8_t DHT22_Read_Byte(void){

	uint8_t i,byte=0;

	for(i=0;i<8;i++)
	{
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0);

		TIM_SetCounter(TIM2,0);

		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1);

		if(TIM_GetCounter(TIM2) > 40)
			byte = (byte<<1) | 1;
		else
			byte = (byte<<1);
	}

	return byte;
}

uint16_t temp_raw;
uint16_t hum_raw;

uint8_t DHT22_Read_Data(float *humidity, float *temperature){

	uint8_t byte[5];
	uint8_t i;
	uint32_t timeout=0;

	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1){
		delay_us(1);
		if(++timeout>100) return 0;
	}

	timeout=0;
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==0){
		delay_us(1);
		if(++timeout>100) return 0;
	}

	timeout=0;
	while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)==1){
		delay_us(1);
		if(++timeout>100) return 0;
	}

	for(i=0;i<5;i++)
		byte[i]=DHT22_Read_Byte();

	// checksum
	if((uint8_t)(byte[0]+byte[1]+byte[2]+byte[3]) != byte[4])
		return 0;

	// ===== DHT22 FORMAT =====
	 hum_raw  = (byte[0] << 8) | byte[1];
	 temp_raw = (byte[2] << 8) | byte[3];

	*humidity = hum_raw / 10.0f;

	// sign bit (bit 15)
	if(temp_raw & 0x8000){
		temp_raw &= 0x7FFF;
		*temperature = -(temp_raw / 10.0f);
	}else{
		*temperature = temp_raw / 10.0f;
	}

	return 1;
}

// ================= ADC SOIL =================

void ADC_Config(){

	GPIO_InitTypeDef gpio;
	ADC_InitTypeDef adc;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// PA0
	gpio.GPIO_Pin = GPIO_Pin_0;
	gpio.GPIO_Mode = GPIO_Mode_AIN;

	GPIO_Init(GPIOA, &gpio);

	adc.ADC_Mode = ADC_Mode_Independent;
	adc.ADC_ScanConvMode = DISABLE;
	adc.ADC_ContinuousConvMode = ENABLE;
	adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	adc.ADC_DataAlign = ADC_DataAlign_Right;
	adc.ADC_NbrOfChannel = 1;

	ADC_Init(ADC1, &adc);

	ADC_RegularChannelConfig(ADC1,
													 ADC_Channel_0,
													 1,
													 ADC_SampleTime_55Cycles5);

	ADC_Cmd(ADC1, ENABLE);

	ADC_ResetCalibration(ADC1);

	while(ADC_GetResetCalibrationStatus(ADC1));

	ADC_StartCalibration(ADC1);

	while(ADC_GetCalibrationStatus(ADC1));

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

uint16_t Read_ADC(){

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);

	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);

	return ADC_GetConversionValue(ADC1);
}

uint16_t Read_ADC_Average(){

	uint32_t sum = 0;

	int i;

	for(i = 0; i < 10; i++){

		sum += Read_ADC();

		delay_ms(5);
	}

	return sum / 10;
}

void UART2_SendString(char *str)
{
    while(*str)
    {
        USART_SendData(USART2, *str++);
        while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
    }
}

char buffer[120];

void UART2_SendJSON(float temp, float hum, int soil)
{
    sprintf(buffer,
            "{\"temp\":%.1f,\"hum\":%.1f,\"soil\":%d}\r\n",
            temp, hum, soil);

    UART2_SendString(buffer);
}

// ================= MAIN =================

float humidity, temperature;
uint8_t checksum;
uint16_t adc_value;
int moisture;

int main(){
	
	UART2_Config();
	UART1_Config();

	TIM2_Config();

	DHT22_GPIO_Config();

	ADC_Config();

	delay_ms(1000);

	while(1){

		// ===== Soil Moisture =====
		adc_value = Read_ADC_Average();
		moisture = (4095 - adc_value) * 100 / 4095;
		// ===== DHT22 =====
		DHT22_Start();
		checksum = DHT22_Read_Data(&humidity, &temperature);
		// ===== Print =====
	if(checksum)
{
		// fix bug - UART1
    printf("Humi: %.1f %% | Temp: %.1f °C | Soil: %d %%\r\n",
       humidity, temperature, moisture);

    UART2_SendJSON(temperature, humidity, moisture);
}
else
{
    printf("{\"error\":\"dht\"}\r\n");
}

		delay_ms(2000);
	}
}
