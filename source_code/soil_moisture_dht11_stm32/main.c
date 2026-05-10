#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"

#include <stdio.h>


// ================= UART1 =================

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


// ================= DHT11 =================

void DHT11_GPIO_Config(){

	GPIO_InitTypeDef gpio;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	gpio.GPIO_Pin = GPIO_Pin_12;
	gpio.GPIO_Mode = GPIO_Mode_IPU;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(GPIOB, &gpio);
}

void DHT11_Start(){

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

uint8_t DHT11_Read_Byte(){

	uint8_t i, byte = 0;

	for(i = 0; i < 8; i++){

		while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0);

		TIM_SetCounter(TIM2, 0);

		while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1);

		if(TIM_GetCounter(TIM2) > 40)
			byte = (byte << 1) | 1;

		else
			byte = (byte << 1);
	}

	return byte;
}

uint8_t DHT11_Read_Data(uint8_t *hum_int,
												uint8_t *hum_dec,
												uint8_t *temp_int,
												uint8_t *temp_dec){

	uint8_t data[5];
	uint8_t i;

	uint32_t timeout = 0;

	while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1){
		delay_us(1);
		if(++timeout > 100) return 0;
	}

	timeout = 0;

	while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 0){
		delay_us(1);
		if(++timeout > 100) return 0;
	}

	timeout = 0;

	while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) == 1){
		delay_us(1);
		if(++timeout > 100) return 0;
	}

	for(i = 0; i < 5; i++){

		data[i] = DHT11_Read_Byte();
	}

	*hum_int = data[0];
	*hum_dec = data[1];

	*temp_int = data[2];
	*temp_dec = data[3];

	return data[4];
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


// ================= MAIN =================

int main(){

	uint8_t hum_int, hum_dec;
	uint8_t temp_int, temp_dec;
	uint8_t checksum;

	uint16_t adc_value;

	int moisture;

	UART1_Config();

	TIM2_Config();

	DHT11_GPIO_Config();

	ADC_Config();

	delay_ms(1000);

	while(1){

		// ===== Soil Moisture =====

		adc_value = Read_ADC_Average();

		moisture = (4095 - adc_value) * 100 / 4095;


		// ===== DHT11 =====

		DHT11_Start();

		checksum = DHT11_Read_Data(&hum_int,
															 &hum_dec,
															 &temp_int,
															 &temp_dec);


		// ===== Print =====

		if((hum_int + hum_dec + temp_int + temp_dec) == checksum){

			printf("Temp: %d.%d C | ",
							temp_int,
							temp_dec);

			printf("Humidity: %d.%d %% | ",
							hum_int,
							hum_dec);

			printf("Soil: %d %%\r\n",
							moisture);
		}
		else{

			printf("DHT11 checksum error!\r\n");
		}

		delay_ms(2000);
	}
}