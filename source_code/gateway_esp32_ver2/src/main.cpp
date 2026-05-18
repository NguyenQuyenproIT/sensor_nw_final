// #include <Arduino.h>
// #include "display.h"
// #include "comm.h"

// extern "C" {
// #include "mlp_model.h"
// }

// #define UART_RX        16
// #define UART_TX        17
// #define UART_BAUD      115200
// #define LED_WARNING    13

// String uartBuffer = "";
    
// void setup()
// {

// // tftInit();
// // drawStaticUI();
// // drawText(20, 170, "BOOTING", rgb565(255,255,255), rgb565(0,0,0), 3);

//     Serial.begin(115200);
//     Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);

//     pinMode(LED_WARNING, OUTPUT);
//     digitalWrite(LED_WARNING, HIGH);

//     tftInit();
//     drawStaticUI();

//     wifiConnect();
//     mqttConnect();

//     drawStaticUI();
//     drawText(20, 170, "ESP32 AI READY", rgb565(0,255,0), rgb565(0,0,0), 2);

//     Serial.println("ESP32 AI READY");
// }

// // void setup()
// // {
// //     Serial.begin(115200);
// //     Serial.println("BOOT 1");

// //     Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);
// //     Serial.println("BOOT 2");

// //     pinMode(LED_WARNING, OUTPUT);
// //     digitalWrite(LED_WARNING, HIGH);
// //     Serial.println("BOOT 3");

// //     tftInit();
// //     Serial.println("BOOT 4 TFT INIT OK");

// //     drawStaticUI();
// //     Serial.println("BOOT 5 UI DRAWN");

// //     wifiConnect();
// //     Serial.println("BOOT 6 WIFI DONE");

// //     mqttConnect();
// //     Serial.println("BOOT 7 MQTT DONE");

// //     drawStaticUI();
// //     drawText(20, 170, "ESP32 AI READY", rgb565(0,255,0), rgb565(0,0,0), 2);

// //     Serial.println("ESP32 AI READY");
// // }

// void loop()
// {
//     mqttLoop();

//     while (Serial2.available())
//     {
//         char c = Serial2.read();

//         if (c == '\n')
//         {
//             uartBuffer.trim();

//             Serial.println("\n==============================");
//             Serial.print("RAW STM32: ");
//             Serial.println(uartBuffer);

//             float temp, hum;
//             int soil;

//             if (parseJSON(uartBuffer, temp, hum, soil))
//             {
//                 Serial.println("JSON PARSE OK");

//                 float prediction = 0.0f;
//                 String ai_state = "N/A";
//                 int led_state = 0;

//                 if (controlMode == MODE_AI)
//                 {
//                     prediction = mlp_predict((float)soil, temp, hum);

//                     if (prediction > 0.0f)
//                     {
//                         ai_state = "WARNING";
//                         digitalWrite(LED_WARNING, LOW);
//                         led_state = 1;
//                     }
//                     else
//                     {
//                         ai_state = "NORMAL";
//                         digitalWrite(LED_WARNING, HIGH);
//                         led_state = 0;
//                     }
//                 }
//                 else
//                 {
//                     ai_state = "MANUAL";
//                     if (manualLedOn)
//                     {
//                         digitalWrite(LED_WARNING, LOW);
//                         led_state = 1;
//                     }
//                     else
//                     {
//                         digitalWrite(LED_WARNING, HIGH);
//                         led_state = 0;
//                     }
//                 }

//                 updateValues(temp, hum, soil, ai_state);
//                 sendToThingsBoard(temp, hum, soil, prediction, ai_state, led_state);
//             }
//             else
//             {
//                 Serial.println("PARSE FAIL");
//             }

//             uartBuffer = "";
//         }
//         else if (c != '\r')
//         {
//             if (uartBuffer.length() < 200) uartBuffer += c;
//             else uartBuffer = "";
//         }
//     }
// }










// #include <Arduino.h>
// #include "display.h"
// #include "comm.h"

// extern "C" {
// #include "mlp_model.h"
// }

// #define UART_RX        16
// #define UART_TX        17
// #define UART_BAUD      115200
// #define LED_WARNING    13
// #define PUMP_OFF       HIGH
// #define PUMP_ON        LOW

// String uartBuffer = "";

// void setup()
// {
//     Serial.begin(115200);
//     Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);

//     pinMode(LED_WARNING, OUTPUT);
//     digitalWrite(LED_WARNING, PUMP_OFF);

//     tftInit();
//     drawStaticUI();
//     drawText(20, 170, "BOOTING", rgb565(255,255,255), rgb565(0,0,0), 3);

//     wifiConnect();
//     mqttConnect();

//     drawStaticUI();
//     drawText(20, 170, "ESP32 AI READY", rgb565(0,255,0), rgb565(0,0,0), 2);

//     Serial.println("ESP32 AI READY");
// }

// void loop()
// {
//     mqttLoop();

//     // End timed pump override and re-enable AI
//     if (pumpOverrideActive && (millis() - pumpOverrideStart >= pumpOverrideDurationMs))
//     {
//         pumpOverrideActive = false;
//         digitalWrite(LED_WARNING, PUMP_OFF);
//         manualLedOn = false;
//         controlMode = MODE_AI;

//         Serial.println("PUMP OVERRIDE END -> AI ON");
//     }

//     while (Serial2.available())
//     {
//         char c = Serial2.read();

//         if (c == '\n')
//         {
//             uartBuffer.trim();

//             Serial.println("\n==============================");
//             Serial.print("RAW STM32: ");
//             Serial.println(uartBuffer);

//             float temp, hum;
//             int soil;

//             if (parseJSON(uartBuffer, temp, hum, soil))
//             {
//                 Serial.println("JSON PARSE OK");

//                 float prediction = 0.0f;
//                 String ai_state = "N/A";
//                 int led_state = 0;

//                 if (controlMode == MODE_AI && !pumpOverrideActive)
//                 {
//                     prediction = mlp_predict((float)soil, temp, hum);

//                     if (prediction > 0.0f)
//                     {
//                         ai_state = "WARNING";
//                         digitalWrite(LED_WARNING, PUMP_ON);
//                         led_state = 1;
//                     }
//                     else
//                     {
//                         ai_state = "NORMAL";
//                         digitalWrite(LED_WARNING, PUMP_OFF);
//                         led_state = 0;
//                     }
//                 }
//                 else if (pumpOverrideActive)
//                 {
//                     ai_state = "OVERRIDE";
//                     digitalWrite(LED_WARNING, PUMP_ON);
//                     led_state = 1;
//                 }
//                 else
//                 {
//                     ai_state = "MANUAL";
//                     if (manualLedOn)
//                     {
//                         digitalWrite(LED_WARNING, PUMP_ON);
//                         led_state = 1;
//                     }
//                     else
//                     {
//                         digitalWrite(LED_WARNING, PUMP_OFF);
//                         led_state = 0;
//                     }
//                 }

//                 updateValues(temp, hum, soil, ai_state);
//                 sendToThingsBoard(temp, hum, soil, prediction, ai_state, led_state);
//             }
//             else
//             {
//                 Serial.println("PARSE FAIL");
//             }

//             uartBuffer = "";
//         }
//         else if (c != '\r')
//         {
//             if (uartBuffer.length() < 200) uartBuffer += c;
//             else uartBuffer = "";
//         }
//     }
// }








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

String uartBuffer = "";

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

    Serial.println("ESP32 AI READY");
}

void loop()
{
    mqttLoop();

    // End timed pump override and restore previous state exactly.
    if (pumpOverrideActive && (millis() - pumpOverrideStart >= pumpOverrideDurationMs))
    {
        pumpOverrideActive = false;
        controlMode = previousMode;
        manualLedOn = previousManualLedOn;

        if (controlMode == MODE_MANUAL)
        {
            digitalWrite(LED_WARNING, manualLedOn ? PUMP_ON : PUMP_OFF);
        }
        else
        {
            // Back to AI mode. Next sensor frame will let AI decide the relay state.
            digitalWrite(LED_WARNING, PUMP_OFF);
        }

        Serial.println("PUMP OVERRIDE END -> RESTORE PREVIOUS STATE");
    }

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

                if (pumpOverrideActive)
                {
                    ai_state = "AUTO PUMP";
                    digitalWrite(LED_WARNING, PUMP_ON);
                    led_state = 1;
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