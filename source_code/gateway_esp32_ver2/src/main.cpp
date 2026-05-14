#include <Arduino.h>
#include "display.h"
#include "comm.h"

extern "C" {
#include "mlp_model.h"
}

#define UART_RX        16
#define UART_TX        17
#define UART_BAUD      115200
#define LED_WARNING    13

String uartBuffer = "";

void setup()
{
    Serial.begin(115200);
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);

    pinMode(LED_WARNING, OUTPUT);
    digitalWrite(LED_WARNING, HIGH);

    tftInit();
    drawStaticUI();

    wifiConnect();
    mqttConnect();

    drawStaticUI();
    drawText(20, 170, "ESP32 AI READY", rgb565(0,255,0), rgb565(0,0,0), 2);

    Serial.println("ESP32 AI READY");
}

void loop()
{
    mqttLoop();

    while (Serial2.available())
    {
        char c = Serial2.read();

        if (c == '\n')
        {
            uartBuffer.trim();

            Serial.println("\n==============================");
            Serial.print("RAW STM32: ");
            Serial.println(uartBuffer);

            float temp, hum;
            int soil;

            if (parseJSON(uartBuffer, temp, hum, soil))
            {
                Serial.println("JSON PARSE OK");

                float prediction = 0.0f;
                String ai_state = "N/A";
                int led_state = 0;

                if (controlMode == MODE_AI)
                {
                    prediction = mlp_predict((float)soil, temp, hum);

                    if (prediction > 0.0f)
                    {
                        ai_state = "WARNING";
                        digitalWrite(LED_WARNING, LOW);
                        led_state = 1;
                    }
                    else
                    {
                        ai_state = "NORMAL";
                        digitalWrite(LED_WARNING, HIGH);
                        led_state = 0;
                    }
                }
                else
                {
                    ai_state = "MANUAL";
                    if (manualLedOn)
                    {
                        digitalWrite(LED_WARNING, LOW);
                        led_state = 1;
                    }
                    else
                    {
                        digitalWrite(LED_WARNING, HIGH);
                        led_state = 0;
                    }
                }

                updateValues(temp, hum, soil, ai_state);
                sendToThingsBoard(temp, hum, soil, prediction, ai_state, led_state);
            }
            else
            {
                Serial.println("PARSE FAIL");
            }

            uartBuffer = "";
        }
        else if (c != '\r')
        {
            if (uartBuffer.length() < 200) uartBuffer += c;
            else uartBuffer = "";
        }
    }
}