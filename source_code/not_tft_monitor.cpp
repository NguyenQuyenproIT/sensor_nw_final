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

#define WIFI_SSID      "QUYEN"
#define WIFI_PASSWORD  "66668888"

#define TB_SERVER      "thingsboard.cloud"
#define TB_PORT        1883
#define TB_TOKEN       "dtyj50xqwdq3515biw9l"

#define TELEMETRY_TOPIC "v1/devices/me/telemetry"
#define RPC_REQUEST_TOPIC "v1/devices/me/rpc/request/+"
#define RPC_RESPONSE_TOPIC_PREFIX "v1/devices/me/rpc/response/"

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
// CONTROL MODE
// =====================================================

enum ControlMode
{
    MODE_AI,
    MODE_MANUAL
};

ControlMode controlMode = MODE_AI;
bool manualLedOn = false;

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
// MQTT CALLBACK
// =====================================================

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String topicStr = String(topic);
    String message;

    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    Serial.println("\n====== RPC IN ======");
    Serial.print("Topic : ");
    Serial.println(topicStr);
    Serial.print("Payload : ");
    Serial.println(message);

    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, message);

    if (err)
    {
        Serial.print("JSON ERROR: ");
        Serial.println(err.c_str());
        return;
    }

    String method = doc["method"] | "";
    JsonVariant params = doc["params"];

    Serial.print("RPC Method : ");
    Serial.println(method);

    StaticJsonDocument<128> respDoc;
    String respTopic;

    // -------------------------------------------------
    // setControlMode
    // -------------------------------------------------
    if (method == "setControlMode")
    {
        String mode = "";

        if (params.is<JsonObject>() && params.containsKey("mode"))
        {
            mode = params["mode"].as<String>();
        }

        if (mode == "MANUAL")
        {
            controlMode = MODE_MANUAL;
            Serial.println("CONTROL MODE = MANUAL");
            respDoc["status"] = "OK";
            respDoc["mode"] = "MANUAL";
        }
        else if (mode == "AI")
        {
            controlMode = MODE_AI;
            Serial.println("CONTROL MODE = AI");
            respDoc["status"] = "OK";
            respDoc["mode"] = "AI";
        }
        else
        {
            Serial.println("UNKNOWN CONTROL MODE");
            respDoc["status"] = "UNKNOWN_MODE";
            respDoc["mode"] = mode;
        }
    }
    // -------------------------------------------------
    // setManualState
    // -------------------------------------------------
    else if (method == "setManualState")
    {
        String state = "";

        if (params.is<JsonObject>() && params.containsKey("state"))
        {
            state = params["state"].as<String>();
        }

        if (controlMode == MODE_MANUAL)
        {
            if (state == "ON")
            {
                manualLedOn = true;
                digitalWrite(LED_WARNING, LOW);   // ACTIVE LOW
                Serial.println("MANUAL LED = ON");
                respDoc["status"] = "OK";
                respDoc["state"] = "ON";
            }
            else if (state == "OFF")
            {
                manualLedOn = false;
                digitalWrite(LED_WARNING, HIGH);  // ACTIVE LOW
                Serial.println("MANUAL LED = OFF");
                respDoc["status"] = "OK";
                respDoc["state"] = "OFF";
            }
            else
            {
                Serial.println("UNKNOWN MANUAL STATE");
                respDoc["status"] = "UNKNOWN_STATE";
                respDoc["state"] = state;
            }
        }
        else
        {
            Serial.println("IGNORE MANUAL STATE BECAUSE MODE IS NOT MANUAL");
            respDoc["status"] = "IGNORED_NOT_MANUAL";
            respDoc["state"] = state;
        }
    }
    // -------------------------------------------------
    // getControlMode
    // -------------------------------------------------
    else if (method == "getControlMode")
    {
        respDoc["status"] = "OK";
        respDoc["mode"] = (controlMode == MODE_MANUAL) ? "MANUAL" : "AI";
    }
    // -------------------------------------------------
    // getManualState
    // -------------------------------------------------
    else if (method == "getManualState")
    {
        respDoc["status"] = "OK";
        respDoc["state"] = manualLedOn ? "ON" : "OFF";
    }
    else
    {
        Serial.println("UNKNOWN RPC METHOD");
        respDoc["status"] = "UNKNOWN_METHOD";
        respDoc["method"] = method;
    }

    // publish response
    String respPayload;
    serializeJson(respDoc, respPayload);

    respTopic = String("v1/devices/me/rpc/response/") + topicStr.substring(topicStr.lastIndexOf('/') + 1);

    mqtt.publish(respTopic.c_str(), respPayload.c_str());

    Serial.print("RPC RESP: ");
    Serial.println(respPayload);
}

// =====================================================
// MQTT CONNECT
// =====================================================

void mqttConnect()
{
    mqtt.setServer(TB_SERVER, TB_PORT);
    mqtt.setCallback(mqttCallback);

    while (!mqtt.connected())
    {
        Serial.print("MQTT connecting...");

        if (mqtt.connect("ESP32_AI_Client", TB_TOKEN, NULL))
        {
            Serial.println("OK");
            mqtt.subscribe(RPC_REQUEST_TOPIC);
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

    doc["temperature"] = temp;
    doc["humidity"]    = hum;
    doc["soil"]        = soil;
    doc["prediction"]  = prediction;
    doc["ai_state"]    = ai_state;
    doc["led_state"]   = led_state;

    if (controlMode == MODE_MANUAL)
    {
        doc["control_mode"] = "MANUAL";
        doc["manual_state"] = manualLedOn ? "ON" : "OFF";
    }
    else
    {
        doc["control_mode"] = "AI";
        doc["manual_state"] = "N/A";
    }

    char payload[256];
    serializeJson(doc, payload);

    Serial.println("====== MQTT OUT ======");
    Serial.println(payload);

    bool ok =
        mqtt.publish(TELEMETRY_TOPIC, payload);

    Serial.println(ok ? "PUBLISH OK" : "PUBLISH FAIL");
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

    pinMode(LED_WARNING, OUTPUT);
    digitalWrite(LED_WARNING, HIGH); // default OFF for active-low

    wifiConnect();
    mqttConnect();

    Serial.println("\nESP32 AI READY");
}

// =====================================================
// LOOP
// =====================================================

void loop()
{
    if (!mqtt.connected())
    {
        mqttConnect();
    }

    mqtt.loop();

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

            if (parseJSON(uartBuffer, temp, hum, soil))
            {
                float prediction = 0.0f;
                String ai_state = "N/A";
                int led_state = 0;

                // =============================================
                // CONTROL LOGIC
                // =============================================

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
                    // MANUAL MODE
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

                Serial.println("====== AI RESULT ======");

                Serial.print("Prediction    : ");
                Serial.println(prediction, 6);

                Serial.print("AI State      : ");
                Serial.println(ai_state);

                Serial.print("LED State     : ");
                Serial.println(led_state ? "ON" : "OFF");

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