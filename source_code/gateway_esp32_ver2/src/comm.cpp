// #include "comm.h"
// #include <WiFi.h>
// #include <PubSubClient.h>
// #include <ArduinoJson.h>
// #include "display.h"

// extern "C" {
// #include "mlp_model.h"
// }

// // =====================================================
// // WIFI / THINGSBOARD
// // =====================================================
// #define WIFI_SSID      "QUYEN"
// #define WIFI_PASSWORD  "66668888"

// #define TB_SERVER      "thingsboard.cloud"
// #define TB_PORT        1883
// #define TB_TOKEN       "dtyj50xqwdq3515biw9l"

// #define TELEMETRY_TOPIC "v1/devices/me/telemetry"
// #define RPC_REQUEST_TOPIC "v1/devices/me/rpc/request/+"

// // =====================================================
// // RELAY / PUMP TEST
// // =====================================================
// #define LED_WARNING    13
// #define PUMP_ON        LOW
// #define PUMP_OFF       HIGH

// WiFiClient espClient;
// PubSubClient mqtt(espClient);

// ControlMode controlMode = MODE_AI;
// bool manualLedOn = false;

// // Pump override state
// bool pumpOverrideActive = false;
// unsigned long pumpOverrideStart = 0;
// unsigned long pumpOverrideDurationMs = 0;

// // Save/restore state around override
//  ControlMode previousMode = MODE_AI;
//  bool previousManualLedOn = false;

// void mqttCallback(char *topic, byte *payload, unsigned int length)
// {
//     String topicStr = String(topic);
//     String message;
//     for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

//     Serial.println("\n====== RPC IN ======");
//     Serial.print("Topic : ");
//     Serial.println(topicStr);
//     Serial.print("Payload : ");
//     Serial.println(message);

//     StaticJsonDocument<256> doc;
//     if (deserializeJson(doc, message))
//     {
//         Serial.println("RPC JSON ERROR");
//         return;
//     }

//     String method = doc["method"] | "";
//     JsonVariant params = doc["params"];

//     Serial.print("RPC Method : ");
//     Serial.println(method);

//     StaticJsonDocument<128> respDoc;

//     if (method == "setControlMode")
//     {
//         String mode = "";
//         if (params.is<JsonObject>() && params.containsKey("mode"))
//             mode = params["mode"].as<String>();

//         if (mode == "MANUAL")
//         {
//             controlMode = MODE_MANUAL;
//             Serial.println("CONTROL MODE = MANUAL");
//             respDoc["status"] = "OK";
//             respDoc["mode"] = "MANUAL";
//         }
//         else if (mode == "AI")
//         {
//             controlMode = MODE_AI;
//             Serial.println("CONTROL MODE = AI");
//             respDoc["status"] = "OK";
//             respDoc["mode"] = "AI";
//         }
//         else
//         {
//             Serial.println("UNKNOWN CONTROL MODE");
//             respDoc["status"] = "UNKNOWN_MODE";
//             respDoc["mode"] = mode;
//         }
//     }
//     else if (method == "setManualState")
//     {
//         String state = "";
//         if (params.is<JsonObject>() && params.containsKey("state"))
//             state = params["state"].as<String>();

//         if (controlMode == MODE_MANUAL)
//         {
//             if (state == "ON")
//             {
//                 manualLedOn = true;
//                 digitalWrite(LED_WARNING, PUMP_ON);
//                 Serial.println("MANUAL LED = ON");
//                 respDoc["status"] = "OK";
//                 respDoc["state"] = "ON";
//             }
//             else if (state == "OFF")
//             {
//                 manualLedOn = false;
//                 digitalWrite(LED_WARNING, PUMP_OFF);
//                 Serial.println("MANUAL LED = OFF");
//                 respDoc["status"] = "OK";
//                 respDoc["state"] = "OFF";
//             }
//             else
//             {
//                 Serial.println("UNKNOWN MANUAL STATE");
//                 respDoc["status"] = "UNKNOWN_STATE";
//                 respDoc["state"] = state;
//             }
//         }
//         else
//         {
//             Serial.println("IGNORE MANUAL STATE BECAUSE MODE IS NOT MANUAL");
//             respDoc["status"] = "IGNORED_NOT_MANUAL";
//             respDoc["state"] = state;
//         }
//     }
//     else if (method == "pumpOverride")
//     {
//         int duration = 0;
//         if (params.is<JsonObject>() && params.containsKey("duration"))
//             duration = params["duration"].as<int>();

//         if (duration > 0)
//         {
//             // Save current state so we can restore it exactly after override.
//             previousMode = controlMode;
//             previousManualLedOn = manualLedOn;

//             pumpOverrideActive = true;
//             pumpOverrideStart = millis();
//             pumpOverrideDurationMs = (unsigned long)duration * 1000UL;

//             // Start override immediately.
//             digitalWrite(LED_WARNING, PUMP_ON);

//             // Do NOT change controlMode/manualLedOn here.
//             // We only override the physical relay temporarily.
//             Serial.println("PUMP OVERRIDE START");
//             respDoc["status"] = "OK";
//             respDoc["duration"] = duration;
//         }
//         else
//         {
//             Serial.println("INVALID DURATION");
//             respDoc["status"] = "INVALID_DURATION";
//             respDoc["duration"] = duration;
//         }
//     }
//     else if (method == "getControlMode")
//     {
//         respDoc["status"] = "OK";
//         respDoc["mode"] = (controlMode == MODE_MANUAL) ? "MANUAL" : "AI";
//     }
//     else if (method == "getManualState")
//     {
//         respDoc["status"] = "OK";
//         respDoc["state"] = manualLedOn ? "ON" : "OFF";
//     }
//     else
//     {
//         Serial.println("UNKNOWN RPC METHOD");
//         respDoc["status"] = "UNKNOWN_METHOD";
//         respDoc["method"] = method;
//     }

//     String respPayload;
//     serializeJson(respDoc, respPayload);
//     String respTopic = String("v1/devices/me/rpc/response/") +
//                        topicStr.substring(topicStr.lastIndexOf('/') + 1);
//     mqtt.publish(respTopic.c_str(), respPayload.c_str());

//     Serial.print("RPC RESP: ");
//     Serial.println(respPayload);
// }

// void wifiConnect()
// {
//     WiFi.mode(WIFI_STA);
//     WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

//     Serial.print("WiFi connecting");
//     while (WiFi.status() != WL_CONNECTED)
//     {
//         delay(500);
//         Serial.print(".");
//     }
//     Serial.println("\nWiFi OK");
// }

// bool mqttConnect()
// {
//     mqtt.setServer(TB_SERVER, TB_PORT);
//     mqtt.setCallback(mqttCallback);

//     if (mqtt.connected())
//         return true;

//     Serial.print("MQTT connecting...");

//     if (mqtt.connect("ESP32_AI_Client", TB_TOKEN, NULL))
//     {
//         Serial.println("OK");
//         mqtt.subscribe(RPC_REQUEST_TOPIC);
//         return true;
//     }
//     else
//     {
//         Serial.print("FAIL rc=");
//         Serial.println(mqtt.state());
//         return false;
//     }
// }

// void mqttLoop()
// {
//     if (!mqtt.connected())
//         mqttConnect();

//     mqtt.loop();
// }

// bool parseJSON(String data, float &temp, float &hum, int &soil)
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
//         !doc.containsKey("hum")  ||
//         !doc.containsKey("soil"))
//     {
//         Serial.println("Missing field");
//         return false;
//     }

//     temp = doc["temp"];
//     hum  = doc["hum"];
//     soil = doc["soil"];
//     return true;
// }

// void sendToThingsBoard(float temp, float hum, int soil, float prediction, String ai_state, int led_state)
// {
//     StaticJsonDocument<256> doc;

//     doc["temperature"] = temp;
//     doc["humidity"]    = hum;
//     doc["soil"]        = soil;
//     doc["prediction"]  = prediction;
//     doc["ai_state"]    = ai_state;
//     doc["led_state"]   = led_state;

//     if (controlMode == MODE_MANUAL)
//     {
//         doc["control_mode"] = "MANUAL";
//         doc["manual_state"] = manualLedOn ? "ON" : "OFF";
//     }
//     else
//     {
//         doc["control_mode"] = "AI";
//         doc["manual_state"] = "N/A";
//     }

//     char payload[256];
//     serializeJson(doc, payload);

//     Serial.println("====== MQTT OUT ======");
//     Serial.println(payload);

//     bool ok = mqtt.publish(TELEMETRY_TOPIC, payload);
//     Serial.println(ok ? "PUBLISH OK" : "PUBLISH FAIL");
// }

#include "comm.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "display.h"

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

#define TELEMETRY_TOPIC  "v1/devices/me/telemetry"
#define RPC_REQUEST_TOPIC "v1/devices/me/rpc/request/+"

// =====================================================
// RELAY / PUMP TEST
// =====================================================
#define LED_WARNING    13
#define PUMP_ON        LOW
#define PUMP_OFF       HIGH

WiFiClient espClient;
PubSubClient mqtt(espClient);

ControlMode controlMode = MODE_AI;
bool manualLedOn = false;

// Override: pump immediately for a duration, then restore previous state
bool pumpOverrideActive = false;
unsigned long pumpOverrideStart = 0;
unsigned long pumpOverrideDurationMs = 0;

// Delay: wait, then pump for duration, then restore previous state
bool pumpDelayActive = false;
unsigned long pumpDelayStart = 0;
unsigned long pumpDelayMs = 0;
unsigned long pumpDelayDurationMs = 0;

// Save/restore state around timed actions
ControlMode previousMode = MODE_AI;
bool previousManualLedOn = false;

static void applyCurrentManualState()
{
    if (controlMode == MODE_MANUAL)
    {
        digitalWrite(LED_WARNING, manualLedOn ? PUMP_ON : PUMP_OFF);
    }
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String topicStr = String(topic);
    String message;
    for (unsigned int i = 0; i < length; i++) message += (char)payload[i];

    Serial.println("\n====== RPC IN ======");
    Serial.print("Topic : ");
    Serial.println(topicStr);
    Serial.print("Payload : ");
    Serial.println(message);

    StaticJsonDocument<256> doc;
    if (deserializeJson(doc, message))
    {
        Serial.println("RPC JSON ERROR");
        return;
    }

    String method = doc["method"] | "";
    JsonVariant params = doc["params"];

    Serial.print("RPC Method : ");
    Serial.println(method);

    StaticJsonDocument<192> respDoc;

    if (method == "setControlMode")
    {
        String mode = "";
        if (params.is<JsonObject>() && params.containsKey("mode"))
            mode = params["mode"].as<String>();

        if (mode == "MANUAL")
        {
            controlMode = MODE_MANUAL;
            Serial.println("CONTROL MODE = MANUAL");
            applyCurrentManualState();
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
    else if (method == "setManualState")
    {
        String state = "";
        if (params.is<JsonObject>() && params.containsKey("state"))
            state = params["state"].as<String>();

        if (controlMode == MODE_MANUAL)
        {
            if (state == "ON")
            {
                manualLedOn = true;
                digitalWrite(LED_WARNING, PUMP_ON);
                Serial.println("MANUAL RELAY = ON");
                respDoc["status"] = "OK";
                respDoc["state"] = "ON";
            }
            else if (state == "OFF")
            {
                manualLedOn = false;
                digitalWrite(LED_WARNING, PUMP_OFF);
                Serial.println("MANUAL RELAY = OFF");
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
    else if (method == "pumpOverride")
    {
        int duration = 0;
        if (params.is<JsonObject>() && params.containsKey("duration"))
            duration = params["duration"].as<int>();

        if (duration > 0)
        {
            previousMode = controlMode;
            previousManualLedOn = manualLedOn;

            pumpOverrideActive = true;
            pumpOverrideStart = millis();
            pumpOverrideDurationMs = (unsigned long)duration * 1000UL;

            digitalWrite(LED_WARNING, PUMP_ON);

            Serial.println("PUMP OVERRIDE START");
            respDoc["status"] = "OK";
            respDoc["duration"] = duration;
            respDoc["saved_mode"] = (previousMode == MODE_MANUAL) ? "MANUAL" : "AI";
            respDoc["saved_manual_state"] = previousManualLedOn ? "ON" : "OFF";
        }
        else
        {
            Serial.println("INVALID DURATION");
            respDoc["status"] = "INVALID_DURATION";
            respDoc["duration"] = duration;
        }
    }
    else if (method == "pumpDelay")
    {
        int delaySec = 0;
        int duration = 0;

        if (params.is<JsonObject>())
        {
            if (params.containsKey("delay"))
                delaySec = params["delay"].as<int>();

            if (params.containsKey("duration"))
                duration = params["duration"].as<int>();
        }

        if (delaySec >= 0 && duration > 0)
        {
            previousMode = controlMode;
            previousManualLedOn = manualLedOn;

            pumpDelayActive = true;
            pumpDelayStart = millis();
            pumpDelayMs = (unsigned long)delaySec * 1000UL;
            pumpDelayDurationMs = (unsigned long)duration * 1000UL;

            // Keep relay OFF during waiting phase.
            digitalWrite(LED_WARNING, PUMP_OFF);

            Serial.println("PUMP DELAY START");
            respDoc["status"] = "OK";
            respDoc["delay"] = delaySec;
            respDoc["duration"] = duration;
            respDoc["saved_mode"] = (previousMode == MODE_MANUAL) ? "MANUAL" : "AI";
            respDoc["saved_manual_state"] = previousManualLedOn ? "ON" : "OFF";
        }
        else
        {
            Serial.println("INVALID DELAY/DURATION");
            respDoc["status"] = "INVALID_PARAMS";
            respDoc["delay"] = delaySec;
            respDoc["duration"] = duration;
        }
    }
    else if (method == "getControlMode")
    {
        respDoc["status"] = "OK";
        respDoc["mode"] = (controlMode == MODE_MANUAL) ? "MANUAL" : "AI";
    }
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

    String respPayload;
    serializeJson(respDoc, respPayload);

    String respTopic = String("v1/devices/me/rpc/response/") +
                       topicStr.substring(topicStr.lastIndexOf('/') + 1);
    mqtt.publish(respTopic.c_str(), respPayload.c_str());

    Serial.print("RPC RESP: ");
    Serial.println(respPayload);
}

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

bool mqttConnect()
{
    mqtt.setServer(TB_SERVER, TB_PORT);
    mqtt.setCallback(mqttCallback);

    if (mqtt.connected())
        return true;

    Serial.print("MQTT connecting...");

    if (mqtt.connect("ESP32_AI_Client", TB_TOKEN, NULL))
    {
        Serial.println("OK");
        mqtt.subscribe(RPC_REQUEST_TOPIC);
        return true;
    }
    else
    {
        Serial.print("FAIL rc=");
        Serial.println(mqtt.state());
        return false;
    }
}

void mqttLoop()
{
    if (!mqtt.connected())
        mqttConnect();

    mqtt.loop();
}

bool parseJSON(String data, float &temp, float &hum, int &soil)
{
    StaticJsonDocument<128> doc;

    DeserializationError err = deserializeJson(doc, data);
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

void sendToThingsBoard(float temp, float hum, int soil, float prediction, String ai_state, int led_state)
{
    StaticJsonDocument<256> doc;

    doc["temperature"] = temp;
    doc["humidity"]    = hum;
    doc["soil"]        = soil;
    doc["prediction"]  = prediction;
    doc["ai_state"]    = ai_state;
    doc["led_state"]   = led_state;

    if (pumpOverrideActive)
    {
        doc["control_mode"] = "OVERRIDE";
        doc["manual_state"] = "N/A";
    }
    else if (pumpDelayActive)
    {
        doc["control_mode"] = "DELAY";
        doc["manual_state"] = "N/A";
    }
    else if (controlMode == MODE_MANUAL)
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

    bool ok = mqtt.publish(TELEMETRY_TOPIC, payload);
    Serial.println(ok ? "PUBLISH OK" : "PUBLISH FAIL");
}