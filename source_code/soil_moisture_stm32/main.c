#include "stm32f10x.h"                  // Device header
#include "stm32f10x_adc.h"              // Keil::Device:StdPeriph Drivers:ADC
#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART
#include "stm32f10x_tim.h"              // Keil::Device:StdPeriph Drivers:TIM
#include <stdio.h>

void config_uart1(){
		
	GPIO_InitTypeDef gpio_pin;
	USART_InitTypeDef usart_init;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// TX
	gpio_pin.GPIO_Pin = GPIO_Pin_9;
	gpio_pin.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_pin);

	// RX
	gpio_pin.GPIO_Pin = GPIO_Pin_10;
	gpio_pin.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio_pin.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio_pin);

	usart_init.USART_BaudRate = 115200;
	usart_init.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usart_init.USART_WordLength = USART_WordLength_8b;
	usart_init.USART_StopBits = USART_StopBits_1;
	usart_init.USART_Parity = USART_Parity_No;
	usart_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;

	USART_Init(USART1, &usart_init);
	USART_Cmd(USART1, ENABLE);
}

struct __FILE{
	int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f){
	
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
	
	return ch;
}

void timer2_init(void){

	TIM_TimeBaseInitTypeDef timerInit;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	timerInit.TIM_CounterMode = TIM_CounterMode_Up;
	timerInit.TIM_Period = 0xFFFF;
	timerInit.TIM_Prescaler = 72 - 1;

	TIM_TimeBaseInit(TIM2, &timerInit);
	TIM_Cmd(TIM2, ENABLE);
}

void delay_us(uint16_t us){
	
	TIM_SetCounter(TIM2,0);
	while(TIM_GetCounter(TIM2) < us);
}

void delay_ms(uint16_t ms){
	
	while(ms--)
		delay_us(1000);
}



void ADC_Config(){  
	GPIO_InitTypeDef GPIO_InitStructure; // cau hinh chan GPIO
	ADC_InitTypeDef ADC_InitStructure;	 // cau hinh ADC
	
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); // bat xung clock cho ADC1
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // bat xung clock cho portA
	
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // (PA0-ADC-channel-0)
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // mode Analog INput(doc analog signal (voltage)
			GPIO_Init(GPIOA, &GPIO_InitStructure);  // goi ham khoi tao pin, mode
		
		ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // hoat dong rieng biet, khong dong bo voi ADC khac
		ADC_InitStructure.ADC_ScanConvMode = DISABLE; // khong quet nhieu kenh, chi doc 1 kenh(channel 0)
	  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; // Che do chuyen doi lien tuc: ADC tu chuyen doi lien tuc sau khi enable
		ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // khong dung ngat ngoai, kich hoat chuyen doi, dung software
		ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // dich bit return value, gia tri 12 bit nam o 12 bit thap cua register 16 bit
		ADC_InitStructure.ADC_NbrOfChannel = 1; // so luong kenh can chuyen doi la 1
		ADC_Init(ADC1, &ADC_InitStructure); // Goi ham khoi tao ADC1 da thiet lap

		ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_55Cycles5); // similar PA0, do uu tien so 1)
//																																thoi gian lay mau 55.5 chu ky ADC(cham vua phai, cho tin hieu on dinh)
		ADC_Cmd(ADC1, ENABLE); // turn on ADC1
		// reset va hieu chinh ADC, giup nang cao do chinh xác, bat buoc sau khi bat ADC lan dau
		ADC_ResetCalibration(ADC1); 
		while(ADC_GetResetCalibrationStatus(ADC1)); 
		ADC_StartCalibration(ADC1);
		while(ADC_GetCalibrationStatus(ADC1));
		
		ADC_SoftwareStartConvCmd(ADC1, ENABLE); // bat dau chuyen doi equal software, neu mode lien tuc thi no se tu chuyen doi
}
		
uint16_t Read_ADC() {
			ADC_SoftwareStartConvCmd(ADC1, ENABLE); // goi lenh khoi tao ADC
			while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)); // EOC duoc set, qua trinh chuyen doi hoan thanh -> DR co gia tri sa~n
			return ADC_GetConversionValue(ADC1); // doc va tra ve gia tri 12 bit tu thanh ghi ADC
	}	// (ADC1 -> DR) data register: luu ket qua 12 bit cua qua trinh ADC
		// copy vào variable in RAM
		
//float Convert_ADC_To_Voltage(uint16_t adc_value) { // chuyen doi gia tri ADC tu(0-4095) become (0-VREF)
//				return ((float)adc_value / ADC_MAX) * VREF;
//		}
	
int i;	
	
uint16_t Read_ADC_Average(){

	uint32_t sum = 0;

	for( i=0;i<10;i++){
		sum += Read_ADC();
		delay_ms(5);
	}

	return sum / 10;
}	
	
	uint16_t adc_value;
	int moisture;

float voltage;



int main(){

config_uart1();
timer2_init();
ADC_Config();

while(1){

		adc_value = Read_ADC_Average();

		// convert %
		moisture = (4095 - adc_value) * 100 / 4095;
//		voltage = adc_value * 3.3 / 4095;
//	
//		printf("Voltage = %.2fV\r\n", voltage);
		printf("ADC = %d | Moisture = %d%%\r\n",
						adc_value, moisture);

		delay_ms(1000);
	}
}