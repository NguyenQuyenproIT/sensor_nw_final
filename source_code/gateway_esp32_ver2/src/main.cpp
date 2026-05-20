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
#define PUMP_OFF       HIGH
#define PUMP_ON        LOW

#define SAME_FRAME_LIMIT 100
#define STM_TIMEOUT_MS   5000UL

String uartBuffer = "";
String lastValidFrame = "";
int sameFrameCount = 0;

unsigned long lastFrameTime = 0;
bool sensorErrorActive = false;

static void enterSensorError()
{
    if (!sensorErrorActive)
    {
        showSensorError();
        sensorErrorActive = true;
        sendToThingsBoard(0.0f, 0.0f, 0, 0.0f, "SENSOR ERROR", 0, 0);
    }
}

static void leaveSensorError()
{
    if (sensorErrorActive)
    {
        restoreNormalUI();
        sensorErrorActive = false;
    }
}

void setup()
{
    Serial.begin(115200);
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);

    pinMode(LED_WARNING, OUTPUT);
    digitalWrite(LED_WARNING, PUMP_OFF);

    tftInit();
    drawStaticUI();
    drawText(20, 170, "BOOTING", rgb565(255,255,255), rgb565(0,0,0), 3);

    wifiConnect();
    mqttConnect();

    drawStaticUI();
    drawText(20, 170, "ESP32 AI READY", rgb565(0,255,0), rgb565(0,0,0), 2);

    lastFrameTime = millis();
}

void loop()
{
    mqttLoop();

    if (pumpOverrideActive && (millis() - pumpOverrideStart >= pumpOverrideDurationMs))
    {
        pumpOverrideActive = false;
        controlMode = previousMode;
        manualLedOn = previousManualLedOn;

        if (controlMode == MODE_MANUAL)
            digitalWrite(LED_WARNING, manualLedOn ? PUMP_ON : PUMP_OFF);
        else
            digitalWrite(LED_WARNING, PUMP_OFF);
    }

    if (pumpDelayActive)
    {
        unsigned long elapsed = millis() - pumpDelayStart;

        if (elapsed >= pumpDelayMs && elapsed < (pumpDelayMs + pumpDelayDurationMs))
        {
            digitalWrite(LED_WARNING, PUMP_ON);
        }
        else if (elapsed >= (pumpDelayMs + pumpDelayDurationMs))
        {
            pumpDelayActive = false;
            controlMode = previousMode;
            manualLedOn = previousManualLedOn;

            if (controlMode == MODE_MANUAL)
                digitalWrite(LED_WARNING, manualLedOn ? PUMP_ON : PUMP_OFF);
            else
                digitalWrite(LED_WARNING, PUMP_OFF);
        }
        else
        {
            digitalWrite(LED_WARNING, PUMP_OFF);
        }
    }

    while (Serial2.available())
    {
        char c = Serial2.read();

        if (c == '\n')
        {
            uartBuffer.trim();

            float temp, hum;
            int soil;

            if (parseJSON(uartBuffer, temp, hum, soil))
            {
                lastFrameTime = millis();

                if (sensorErrorActive)
                    leaveSensorError();

                if (uartBuffer == lastValidFrame)
                    sameFrameCount++;
                else
                {
                    sameFrameCount = 1;
                    lastValidFrame = uartBuffer;
                }

                if (sameFrameCount >= SAME_FRAME_LIMIT)
                {
                    enterSensorError();
                    uartBuffer = "";
                    continue;
                }

                float prediction = 0.0f;
                String ai_state = "N/A";
                int led_state = 0;
                int sensor_status = 1;

                if (pumpOverrideActive)
                {
                    ai_state = "AUTO PUMP";
                    digitalWrite(LED_WARNING, PUMP_ON);
                    led_state = 1;
                }
                else if (pumpDelayActive)
                {
                    unsigned long elapsed = millis() - pumpDelayStart;
                    if (elapsed >= pumpDelayMs && elapsed < (pumpDelayMs + pumpDelayDurationMs))
                    {
                        ai_state = "AUTO PUMP";
                        digitalWrite(LED_WARNING, PUMP_ON);
                        led_state = 1;
                    }
                    else
                    {
                        ai_state = "TIMER";
                        digitalWrite(LED_WARNING, PUMP_OFF);
                        led_state = 0;
                    }
                }
                else if (controlMode == MODE_AI)
                {
                    prediction = mlp_predict((float)soil, temp, hum);

                    if (prediction > 0.0f)
                    {
                        ai_state = "WARNING";
                        digitalWrite(LED_WARNING, PUMP_ON);
                        led_state = 1;
                    }
                    else
                    {
                        ai_state = "NORMAL";
                        digitalWrite(LED_WARNING, PUMP_OFF);
                        led_state = 0;
                    }
                }
                else
                {
                    ai_state = "MANUAL";
                    if (manualLedOn)
                    {
                        digitalWrite(LED_WARNING, PUMP_ON);
                        led_state = 1;
                    }
                    else
                    {
                        digitalWrite(LED_WARNING, PUMP_OFF);
                        led_state = 0;
                    }
                }

                updateValues(temp, hum, soil, ai_state);
                sendToThingsBoard(temp, hum, soil, prediction, ai_state, led_state, sensor_status);
            }

            uartBuffer = "";
        }
        else if (c != '\r')
        {
            if (uartBuffer.length() < 200) uartBuffer += c;
            else uartBuffer = "";
        }
    }

    if (!sensorErrorActive && (millis() - lastFrameTime >= STM_TIMEOUT_MS))
    {
        enterSensorError();
    }
}