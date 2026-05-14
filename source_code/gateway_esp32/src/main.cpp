#include <Arduino.h>
#include <SPI.h>
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
// TFT SPI PINS
// =====================================================
#define TFT_DC   2
#define TFT_RST  4
#define TFT_BL   15
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   -1

// =====================================================
// TFT SIZE
// =====================================================
#define TFT_WIDTH   240
#define TFT_HEIGHT  240

// =====================================================
// ST7789 COMMANDS
// =====================================================
#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT  0x11
#define ST7789_NORON   0x13
#define ST7789_INVON   0x21
#define ST7789_INVOFF  0x20
#define ST7789_DISPON  0x29
#define ST7789_CASET   0x2A
#define ST7789_RASET   0x2B
#define ST7789_RAMWR   0x2C
#define ST7789_COLMOD  0x3A
#define ST7789_MADCTL  0x36

SPIClass tftSPI(VSPI);

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
// LAST SENSOR VALUES
// =====================================================
float lastTemp = -1000.0f;
float lastHum  = -1000.0f;
int   lastSoil = -1;
String lastStatus = "";

// =====================================================
// PREVIOUS VALUES FOR TFT
// =====================================================
float prevTemp = -1000.0f;
float prevHum  = -1000.0f;
int   prevSoil = -1;
String prevStatus = "";

// =====================================================
// COLOR HELPER
// =====================================================
uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// =====================================================
// TFT LOW LEVEL
// =====================================================
void tftWriteCommand(uint8_t cmd)
{
    digitalWrite(TFT_DC, LOW);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer(cmd);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

void tftWriteData(uint8_t data)
{
    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer(data);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

void tftSetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    tftWriteCommand(ST7789_CASET);
    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer16(x0);
    tftSPI.transfer16(x1);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);

    tftWriteCommand(ST7789_RASET);
    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer16(y0);
    tftSPI.transfer16(y1);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);

    tftWriteCommand(ST7789_RAMWR);
}

void tftReset()
{
    digitalWrite(TFT_RST, HIGH);
    delay(50);
    digitalWrite(TFT_RST, LOW);
    delay(150);
    digitalWrite(TFT_RST, HIGH);
    delay(150);
}

void tftInit()
{
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_BL, OUTPUT);
    if (TFT_CS >= 0) pinMode(TFT_CS, OUTPUT);

    digitalWrite(TFT_BL, HIGH);   // nếu không sáng, thử LOW

    tftSPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    tftSPI.beginTransaction(SPISettings(27000000, MSBFIRST, SPI_MODE3));

    tftReset();

    tftWriteCommand(ST7789_SWRESET);
    delay(150);

    tftWriteCommand(ST7789_SLPOUT);
    delay(150);

    tftWriteCommand(ST7789_COLMOD);
    tftWriteData(0x55);

    tftWriteCommand(ST7789_MADCTL);
    tftWriteData(0x00);

    tftWriteCommand(ST7789_INVON);
    delay(10);

    tftWriteCommand(ST7789_NORON);
    delay(10);

    tftWriteCommand(ST7789_DISPON);
    delay(120);
}

void tftFillScreen(uint16_t color)
{
    tftSetAddrWindow(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

    for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++)
    {
        tftSPI.transfer16(color);
    }

    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

void tftDrawPixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= TFT_WIDTH || y < 0 || y >= TFT_HEIGHT) return;

    tftSetAddrWindow(x, y, x, y);

    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer16(color);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

// =====================================================
// SIMPLE FONT 5x7
// =====================================================
const uint8_t simpleFont[96][5] PROGMEM = {
    {0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x5F,0x00,0x00}, {0x00,0x07,0x00,0x07,0x00},
    {0x14,0x7F,0x14,0x7F,0x14}, {0x24,0x2A,0x7F,0x2A,0x12}, {0x23,0x13,0x08,0x64,0x62},
    {0x36,0x49,0x55,0x22,0x50}, {0x00,0x05,0x03,0x00,0x00}, {0x00,0x1C,0x22,0x41,0x00},
    {0x00,0x41,0x22,0x1C,0x00}, {0x14,0x08,0x3E,0x08,0x14}, {0x08,0x08,0x3E,0x08,0x08},
    {0x00,0x50,0x30,0x00,0x00}, {0x08,0x08,0x08,0x08,0x08}, {0x00,0x60,0x60,0x00,0x00},
    {0x20,0x10,0x08,0x04,0x02}, {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
    {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10},
    {0x27,0x45,0x45,0x45,0x39}, {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
    {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}, {0x00,0x36,0x36,0x00,0x00},
    {0x00,0x56,0x36,0x00,0x00}, {0x08,0x14,0x22,0x41,0x00}, {0x14,0x14,0x14,0x14,0x14},
    {0x00,0x41,0x22,0x14,0x08}, {0x02,0x01,0x51,0x09,0x06}, {0x32,0x49,0x79,0x41,0x3E},
    {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36}, {0x3E,0x41,0x41,0x41,0x22},
    {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
    {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F}, {0x00,0x41,0x7F,0x41,0x00},
    {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
    {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F}, {0x3E,0x41,0x41,0x41,0x3E},
    {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
    {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01}, {0x3F,0x40,0x40,0x40,0x3F},
    {0x1F,0x20,0x40,0x20,0x1F}, {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
    {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
};

void drawChar5x7(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t scale = 2)
{
    c = toupper((unsigned char)c);
    if (c < 32 || c > 90) c = '?';
    const uint8_t* glyph = simpleFont[c - 32];

    for (int col = 0; col < 5; col++)
    {
        uint8_t line = pgm_read_byte(&glyph[col]);
        for (int row = 0; row < 8; row++)
        {
            uint16_t drawColor = (line & (1 << row)) ? color : bg;
            for (int sx = 0; sx < scale; sx++)
            {
                for (int sy = 0; sy < scale; sy++)
                {
                    tftDrawPixel(x + col * scale + sx, y + row * scale + sy, drawColor);
                }
            }
        }
    }
}

void drawText(int x, int y, const String& text, uint16_t color, uint16_t bg, uint8_t scale = 2)
{
    int cursorX = x;
    for (size_t i = 0; i < text.length(); i++)
    {
        drawChar5x7(cursorX, y, text[i], color, bg, scale);
        cursorX += 6 * scale;
    }
}

void clearField(int x, int y, int w, int h, uint16_t bg)
{
    tftSetAddrWindow(x, y, x + w - 1, y + h - 1);

    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

    for (int i = 0; i < w * h; i++)
    {
        tftSPI.transfer16(bg);
    }

    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

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
// STATIC UI
// =====================================================
void drawStaticUI()
{
    uint16_t bg = rgb565(0, 0, 0);
    tftFillScreen(bg);

    drawText(20, 12, "Sensor Networks", rgb565(255,255,0), bg, 2);

    drawText(20, 55, "HUMI :", rgb565(0,255,255), bg, 2);
    drawText(20, 90, "TEMP :", rgb565(255,0,0), bg, 2);
    drawText(20, 125, "SOIL :", rgb565(0,255,0), bg, 2);

    drawText(20, 170, "BOOTING", rgb565(255,255,255), bg, 3);
}

// =====================================================
// UPDATE ONLY VALUES
// =====================================================
void updateValues(float temp, float hum, int soil, const String &status)
{
    uint16_t bg = rgb565(0, 0, 0);

    if (hum != prevHum)
    {
        clearField(110, 52, 120, 28, bg);
        drawText(110, 55, String(hum, 1) + " %", rgb565(0,255,255), bg, 2);
        prevHum = hum;
    }

    if (temp != prevTemp)
    {
        clearField(110, 87, 120, 28, bg);
        drawText(110, 90, String(temp, 1) + " C", rgb565(255,0,0), bg, 2);
        prevTemp = temp;
    }

    if (soil != prevSoil)
    {
        clearField(110, 122, 120, 28, bg);
        drawText(110, 125, String(soil) + " %", rgb565(0,255,0), bg, 2);
        prevSoil = soil;
    }

    if (status != prevStatus)
    {
        clearField(20, 166, 200, 40, bg);

        if (status == "WARNING")
            drawText(20, 170, "WARNING", rgb565(255,0,0), bg, 3);
        else if (status == "NORMAL")
            drawText(20, 170, "NORMAL", rgb565(0,255,0), bg, 3);
        else if (status == "MANUAL")
            drawText(20, 170, "MANUAL", rgb565(255,165,0), bg, 3);
        else
            drawText(20, 170, status, rgb565(255,255,255), bg, 3);

        prevStatus = status;
    }
}

// =====================================================
// MQTT CALLBACK
// =====================================================
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

    StaticJsonDocument<128> respDoc;

    if (method == "setControlMode")
    {
        String mode = "";
        if (params.is<JsonObject>() && params.containsKey("mode"))
            mode = params["mode"].as<String>();

        if (mode == "MANUAL")
        {
            controlMode = MODE_MANUAL;
            Serial.println("CONTROL MODE = MANUAL");
            respDoc["status"] = "OK";
            respDoc["mode"] = "MANUAL";
            lastStatus = "MANUAL";
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
                digitalWrite(LED_WARNING, LOW);
                Serial.println("MANUAL LED = ON");
                respDoc["status"] = "OK";
                respDoc["state"] = "ON";
                lastStatus = "MANUAL";
            }
            else if (state == "OFF")
            {
                manualLedOn = false;
                digitalWrite(LED_WARNING, HIGH);
                Serial.println("MANUAL LED = OFF");
                respDoc["status"] = "OK";
                respDoc["state"] = "OFF";
                lastStatus = "MANUAL";
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

// =====================================================
// SEND AI DATA TO THINGSBOARD
// =====================================================
void sendToThingsBoard(float temp, float hum, int soil, float prediction, String ai_state, int led_state)
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

    bool ok = mqtt.publish(TELEMETRY_TOPIC, payload);
    Serial.println(ok ? "PUBLISH OK" : "PUBLISH FAIL");
}

// =====================================================
// SETUP
// =====================================================
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

// =====================================================
// LOOP
// =====================================================
void loop()
{
    if (!mqtt.connected()) mqttConnect();
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

                lastTemp = temp;
                lastHum = hum;
                lastSoil = soil;
                lastStatus = ai_state;

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