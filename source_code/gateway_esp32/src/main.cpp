// #include <Arduino.h>
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>

// extern "C" {
// #include "mlp_model.h"
// }

// // ================= WIFI / THINGSBOARD =================
// #define WIFI_SSID      "TP-LINK_991C"
// #define WIFI_PASSWORD  "47322547"

// #define TB_SERVER      "thingsboard.cloud"
// #define TB_PORT        1883
// #define TB_TOKEN       "dtyj50xqwdq3515biw9l"

// #define TELEMETRY_TOPIC "v1/devices/me/telemetry"

// // ================= UART =================
// #define UART_RX 16
// #define UART_TX 17
// #define UART_BAUD 115200

// WiFiClient espClient;
// PubSubClient mqtt(espClient);

// String uartBuffer = "";

// // ================= WIFI =================
// void wifiConnect() {
//   WiFi.mode(WIFI_STA);
//   WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

//   Serial.print("WiFi connecting");
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(500);
//     Serial.print(".");
//   }
//   Serial.println("\nWiFi OK");
// }

// // ================= MQTT =================
// void mqttConnect() {
//   mqtt.setServer(TB_SERVER, TB_PORT);

//   while (!mqtt.connected()) {
//     Serial.print("MQTT connecting...");

//     if (mqtt.connect("ESP32_Client", TB_TOKEN, NULL)) {
//       Serial.println("OK");
//     } else {
//       Serial.print("FAIL rc=");
//       Serial.println(mqtt.state());
//       delay(2000);
//     }
//   }
// }

// // ================= PUBLISH =================
// void sendToThingsBoard(float temp, float hum, int soil) {

//   StaticJsonDocument<128> doc;
//   doc["temperature"] = temp;
//   doc["humidity"] = hum;
//   doc["soil"] = soil;

//   char payload[128];
//   size_t len = serializeJson(doc, payload);

//   Serial.print("MQTT OUT: ");
//   Serial.println(payload);

//   bool ok = mqtt.publish(TELEMETRY_TOPIC, payload, len);

//   Serial.println(ok ? "PUBLISH OK" : "PUBLISH FAIL");
// }

// // ================= PARSE =================
// bool parseJSON(String data, float &t, float &h, int &s) {

//   StaticJsonDocument<128> doc;
//   DeserializationError err = deserializeJson(doc, data);

//   if (err) {
//     Serial.print("JSON ERROR: ");
//     Serial.println(err.c_str());
//     return false;
//   }

//   if (!doc.containsKey("temp") ||
//       !doc.containsKey("hum") ||
//       !doc.containsKey("soil")) {
//     Serial.println("Missing field");
//     return false;
//   }

//   t = doc["temp"];
//   h = doc["hum"];
//   s = doc["soil"];

//   return true;
// }

// // ================= SETUP =================
// void setup() {
//   Serial.begin(115200);
//   Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);

//   wifiConnect();
//   mqttConnect();
// }

// // ================= LOOP =================
// void loop() {

//   if (!mqtt.connected()) mqttConnect();
//   mqtt.loop();

//   while (Serial2.available()) {
//     char c = Serial2.read();

//     if (c == '\n') {

//       uartBuffer.trim();

//       Serial.print("RAW STM32: ");
//       Serial.println(uartBuffer);

//       float t, h;
//       int s;

//       if (parseJSON(uartBuffer, t, h, s)) {
//         sendToThingsBoard(t, h, s);
//       } else {
//         Serial.println("PARSE FAIL");
//       }

//       uartBuffer = "";

//     } else if (c != '\r') {

//       if (uartBuffer.length() < 200)
//         uartBuffer += c;
//       else
//         uartBuffer = "";
//     }
//   }
// }









// #include <Arduino.h>

// // COMMENT TẠM CLOUD
// // #include <WiFi.h>
// // #include <PubSubClient.h>
// // #include <ArduinoJson.h>

// #include <ArduinoJson.h>

// extern "C" {
// #include "mlp_model.h"
// }

// // ================= UART =================
// #define UART_RX 16
// #define UART_TX 17
// #define UART_BAUD 115200

// String uartBuffer = "";

// // ================= PARSE JSON =================
// bool parseJSON(String data, float &t, float &h, int &s)
// {
//     StaticJsonDocument<128> doc;

//     DeserializationError err = deserializeJson(doc, data);

//     if (err)
//     {
//         Serial.print("JSON ERROR: ");
//         Serial.println(err.c_str());
//         return false;
//     }

//     if (!doc.containsKey("temp") ||
//         !doc.containsKey("hum") ||
//         !doc.containsKey("soil"))
//     {
//         Serial.println("Missing field");
//         return false;
//     }

//     t = doc["temp"];
//     h = doc["hum"];
//     s = doc["soil"];

//     return true;
// }

// // ================= SETUP =================
// void setup()
// {
//     Serial.begin(115200);

//     Serial2.begin(UART_BAUD, SERIAL_8N1,
//                   UART_RX, UART_TX);

//     Serial.println("\nESP32 AI READY");
// }

// // ================= LOOP =================
// void loop()
// {
//     while (Serial2.available())
//     {
//         char c = Serial2.read();

//         if (c == '\n')
//         {
//             uartBuffer.trim();

//             Serial.print("\nRAW STM32: ");
//             Serial.println(uartBuffer);

//             float t, h;
//             int s;

//             if (parseJSON(uartBuffer, t, h, s))
//             {
//                 // ================= AI =================

//                 float result;

//                 result = mlp_predict(
//                             (float)s,
//                             t,
//                             h
//                          );

//                 // ================= PRINT =================

//                 Serial.println("====== AI INPUT ======");

//                 Serial.print("Soil: ");
//                 Serial.println(s);

//                 Serial.print("Temp: ");
//                 Serial.println(t);

//                 Serial.print("Humidity: ");
//                 Serial.println(h);

//                 Serial.println("====== AI OUTPUT ======");

//                 Serial.print("Prediction: ");
//                 Serial.println(result, 6);

//                 // ================= DECISION =================

//                 if(result > 0.5f)
//                 {
//                     Serial.println("AI RESULT: WARNING");
//                 }
//                 else
//                 {
//                     Serial.println("AI RESULT: NORMAL");
//                 }
//             }
//             else
//             {
//                 Serial.println("PARSE FAIL");
//             }

//             uartBuffer = "";
//         }

//         else if (c != '\r')
//         {
//             if (uartBuffer.length() < 200)
//                 uartBuffer += c;
//             else
//                 uartBuffer = "";
//         }
//     }
// }






// #include <Arduino.h>
// #include <ArduinoJson.h>

// extern "C" {
// #include "mlp_model.h"
// }

// // ================= UART =================
// #define UART_RX     16
// #define UART_TX     17
// #define UART_BAUD   115200

// // ================= LED =================
// #define LED_WARNING 13

// String uartBuffer = "";

// // ================= PARSE JSON =================
// bool parseJSON(String data, float &t, float &h, int &s)
// {
//     StaticJsonDocument<128> doc;

//     DeserializationError err = deserializeJson(doc, data);

//     if (err)
//     {
//         Serial.print("JSON ERROR: ");
//         Serial.println(err.c_str());
//         return false;
//     }

//     if (!doc.containsKey("temp") ||
//         !doc.containsKey("hum") ||
//         !doc.containsKey("soil"))
//     {
//         Serial.println("Missing field");
//         return false;
//     }

//     t = doc["temp"];
//     h = doc["hum"];
//     s = doc["soil"];

//     return true;
// }

// // ================= SETUP =================
// void setup()
// {
//     Serial.begin(115200);

//     Serial2.begin(
//         UART_BAUD,
//         SERIAL_8N1,
//         UART_RX,
//         UART_TX
//     );

//     // ================= LED =================
//     pinMode(LED_WARNING, OUTPUT);

//     digitalWrite(LED_WARNING, LOW);

//     Serial.println("\nESP32 AI READY");
// }

// // ================= LOOP =================
// void loop()
// {
//     while (Serial2.available())
//     {
//         char c = Serial2.read();

//         if (c == '\n')
//         {
//             uartBuffer.trim();

//             Serial.print("\nRAW STM32: ");
//             Serial.println(uartBuffer);

//             float t, h;
//             int s;

//             if (parseJSON(uartBuffer, t, h, s))
//             {
//                 // ================= AI PREDICT =================

//                 float result = mlp_predict(
//                                     (float)s,
//                                     t,
//                                     h
//                                );

//                 // ================= PRINT INPUT =================

//                 Serial.println("====== AI INPUT ======");

//                 Serial.print("Soil: ");
//                 Serial.println(s);

//                 Serial.print("Temp: ");
//                 Serial.println(t);

//                 Serial.print("Humidity: ");
//                 Serial.println(h);

//                 // ================= PRINT OUTPUT =================

//                 Serial.println("====== AI OUTPUT ======");

//                 Serial.print("Prediction: ");
//                 Serial.println(result, 6);

//                 // ================= AI DECISION =================

//                 if(result > 0.0f)
//                 {
//                     Serial.println("AI RESULT: WARNING");

//                     // LED ON
//                     digitalWrite(LED_WARNING, LOW);
//                 }
//                 else
//                 {
//                     Serial.println("AI RESULT: NORMAL");

//                     // LED OFF
//                     digitalWrite(LED_WARNING, HIGH);
//                 }
//             }
//             else
//             {
//                 Serial.println("PARSE FAIL");
//             }

//             uartBuffer = "";
//         }

//         else if (c != '\r')
//         {
//             if (uartBuffer.length() < 200)
//                 uartBuffer += c;
//             else
//                 uartBuffer = "";
//         }
//     }
// }










#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

extern "C" {
#include "mlp_model.h"
}

// =====================================================
// WIFI / THINGSBOARD
// =====================================================

#define WIFI_SSID      "TP-LINK_991C"
#define WIFI_PASSWORD  "47322547"

#define TB_SERVER      "thingsboard.cloud"
#define TB_PORT        1883
#define TB_TOKEN       "dtyj50xqwdq3515biw9l"

#define TELEMETRY_TOPIC "v1/devices/me/telemetry"

// =====================================================
// UART STM32
// =====================================================

#define UART_RX        16
#define UART_TX        17
#define UART_BAUD      115200

// =====================================================
// LED WARNING
// =====================================================

#define LED_WARNING    13

// =====================================================
// GLOBAL
// =====================================================

WiFiClient espClient;
PubSubClient mqtt(espClient);

String uartBuffer = "";

// =====================================================
// WIFI CONNECT
// =====================================================

void wifiConnect()
{
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("WiFi connecting");

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi OK");
}

// =====================================================
// MQTT CONNECT
// =====================================================

void mqttConnect()
{
    mqtt.setServer(TB_SERVER, TB_PORT);

    while (!mqtt.connected())
    {
        Serial.print("MQTT connecting...");

        if (mqtt.connect("ESP32_AI_Client",
                         TB_TOKEN,
                         NULL))
        {
            Serial.println("OK");
        }
        else
        {
            Serial.print("FAIL rc=");
            Serial.println(mqtt.state());

            delay(2000);
        }
    }
}

// =====================================================
// PARSE JSON FROM STM32
// =====================================================

bool parseJSON(String data,
               float &temp,
               float &hum,
               int &soil)
{
    StaticJsonDocument<128> doc;

    DeserializationError err =
        deserializeJson(doc, data);

    if (err)
    {
        Serial.print("JSON ERROR: ");
        Serial.println(err.c_str());

        return false;
    }

    if (!doc.containsKey("temp") ||
        !doc.containsKey("hum")  ||
        !doc.containsKey("soil"))
    {
        Serial.println("Missing field");

        return false;
    }

    temp = doc["temp"];
    hum  = doc["hum"];
    soil = doc["soil"];

    return true;
}

// =====================================================
// SEND AI DATA TO THINGSBOARD
// =====================================================

void sendToThingsBoard(float temp,
                       float hum,
                       int soil,
                       float prediction,
                       String ai_state,
                       int led_state)
{
    StaticJsonDocument<256> doc;

    // ================= SENSOR =================

    doc["temperature"] = temp;
    doc["humidity"]    = hum;
    doc["soil"]        = soil;

    // ================= AI =================

    doc["prediction"]  = prediction;

    doc["ai_state"]    = ai_state;

    // ================= LED =================

    doc["led_state"]   = led_state;

    // =================================================

    char payload[256];

    serializeJson(doc, payload);

    Serial.println("====== MQTT OUT ======");
    Serial.println(payload);

    bool ok =
        mqtt.publish(TELEMETRY_TOPIC,
                     payload);

    Serial.println(ok ?
                   "PUBLISH OK"
                   :
                   "PUBLISH FAIL");
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
    Serial.begin(115200);

    Serial2.begin(
        UART_BAUD,
        SERIAL_8N1,
        UART_RX,
        UART_TX
    );

    // ================= LED =================

    pinMode(LED_WARNING, OUTPUT);

    digitalWrite(LED_WARNING, HIGH);

    // ================= WIFI =================

    wifiConnect();

    mqttConnect();

    Serial.println("\nESP32 AI READY");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    // ================= MQTT =================

    if (!mqtt.connected())
    {
        mqttConnect();
    }

    mqtt.loop();

    // =================================================

    while (Serial2.available())
    {
        char c = Serial2.read();

        if (c == '\n')
        {
            uartBuffer.trim();

            Serial.println("\n==============================");
            Serial.print("RAW STM32: ");
            Serial.println(uartBuffer);

            float temp;
            float hum;
            int soil;

            // =================================================

            if (parseJSON(uartBuffer,
                          temp,
                          hum,
                          soil))
            {
                // =============================================
                // AI PREDICT
                // =============================================

                float prediction =
                    mlp_predict(
                        (float)soil,
                        temp,
                        hum
                    );

                // =============================================
                // AI DECISION
                // =============================================

                String ai_state;

                int led_state;

                if (prediction > 0.0f)
                {
                    ai_state = "WARNING";

                    // ACTIVE LOW
                    digitalWrite(LED_WARNING, LOW);

                    led_state = 1;
                }
                else
                {
                    ai_state = "NORMAL";

                    digitalWrite(LED_WARNING, HIGH);

                    led_state = 0;
                }

                // =============================================
                // SERIAL MONITOR
                // =============================================

                Serial.println("====== SENSOR DATA ======");

                Serial.print("Soil Moisture : ");
                Serial.println(soil);

                Serial.print("Temperature   : ");
                Serial.println(temp);

                Serial.print("Humidity      : ");
                Serial.println(hum);

                // =============================================

                Serial.println("====== AI RESULT ======");

                Serial.print("Prediction    : ");
                Serial.println(prediction, 6);

                Serial.print("AI State      : ");
                Serial.println(ai_state);

                Serial.print("LED State     : ");

                if (led_state)
                    Serial.println("ON");
                else
                    Serial.println("OFF");

                // =============================================
                // SEND TO THINGSBOARD
                // =============================================

                sendToThingsBoard(
                    temp,
                    hum,
                    soil,
                    prediction,
                    ai_state,
                    led_state
                );
            }
            else
            {
                Serial.println("PARSE FAIL");
            }

            uartBuffer = "";
        }

        else if (c != '\r')
        {
            if (uartBuffer.length() < 200)
            {
                uartBuffer += c;
            }
            else
            {
                uartBuffer = "";
            }
        }
    }
}