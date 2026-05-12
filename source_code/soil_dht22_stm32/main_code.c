#include "stm32f10x.h"
#include "uart.h"
#include "timer_delay.h"
#include "dht22.h"
#include "adc_soil.h"
#include "stm32f10x_usart.h"            // Keil::Device:StdPeriph Drivers:USART


float humidity, temperature;
uint8_t checksum;
int moisture;

int main(void)
{
    UART2_Config();
    UART1_Config();
    TIM2_Config();
    DHT22_GPIO_Config();
    ADC_Config();

    delay_ms(1000);

    while (1)
    {
        moisture = Soil_Moisture_Percent();

        DHT22_Start();
        checksum = DHT22_Read_Data(&humidity, &temperature);

        if (checksum)
        {
            printf("Humi: %.1f %% | Temp: %.1f C | Soil: %d %%\r\n",
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
