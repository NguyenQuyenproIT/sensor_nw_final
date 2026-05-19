// #include "display.h"
// #include <SPI.h>

// // =====================================================
// // TFT SPI PINS
// // =====================================================

// // GND  -> GND
// // VCC  -> 3V3
// // SCK  -> GPIO 18
// // SDA  -> GPIO 23
// // RES  -> GPIO 4
// // DC   -> GPIO 2
// // BLK  -> GPIO 15

// #define TFT_DC   2
// #define TFT_RST  4
// #define TFT_BL   15
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS   -1

// // =====================================================
// // TFT SIZE
// // =====================================================
// #define TFT_WIDTH   240
// #define TFT_HEIGHT  240

// // =====================================================
// // ST7789 COMMANDS
// // =====================================================
// #define ST7789_SWRESET 0x01
// #define ST7789_SLPOUT  0x11
// #define ST7789_NORON   0x13
// #define ST7789_INVON   0x21
// #define ST7789_INVOFF  0x20
// #define ST7789_DISPON  0x29
// #define ST7789_CASET   0x2A
// #define ST7789_RASET   0x2B
// #define ST7789_RAMWR   0x2C
// #define ST7789_COLMOD  0x3A
// #define ST7789_MADCTL  0x36

// SPIClass tftSPI(VSPI);

// // =====================================================
// // PREVIOUS VALUES FOR TFT
// // =====================================================
// static float prevTemp = -1000.0f;
// static float prevHum  = -1000.0f;
// static int   prevSoil = -1;
// static String prevStatus = "";

// // =====================================================
// // COLOR HELPER
// // =====================================================
// uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
// {
//     return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
// }

// // =====================================================
// // TFT LOW LEVEL
// // =====================================================
// static void tftWriteCommand(uint8_t cmd)
// {
//     digitalWrite(TFT_DC, LOW);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer(cmd);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// static void tftWriteData(uint8_t data)
// {
//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer(data);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// static void tftSetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
// {
//     tftWriteCommand(ST7789_CASET);
//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer16(x0);
//     tftSPI.transfer16(x1);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);

//     tftWriteCommand(ST7789_RASET);
//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer16(y0);
//     tftSPI.transfer16(y1);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);

//     tftWriteCommand(ST7789_RAMWR);
// }

// static void tftReset()
// {
//     digitalWrite(TFT_RST, HIGH);
//     delay(50);
//     digitalWrite(TFT_RST, LOW);
//     delay(150);
//     digitalWrite(TFT_RST, HIGH);
//     delay(150);
// }

// static void tftFillScreen(uint16_t color)
// {
//     tftSetAddrWindow(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

//     for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++)
//     {
//         tftSPI.transfer16(color);
//     }

//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// static void tftDrawPixel(int x, int y, uint16_t color)
// {
//     if (x < 0 || x >= TFT_WIDTH || y < 0 || y >= TFT_HEIGHT) return;

//     tftSetAddrWindow(x, y, x, y);

//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer16(color);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// // =====================================================
// // SIMPLE FONT 5x7
// // =====================================================
// static const uint8_t simpleFont[96][5] PROGMEM = {
//     {0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x5F,0x00,0x00}, {0x00,0x07,0x00,0x07,0x00},
//     {0x14,0x7F,0x14,0x7F,0x14}, {0x24,0x2A,0x7F,0x2A,0x12}, {0x23,0x13,0x08,0x64,0x62},
//     {0x36,0x49,0x55,0x22,0x50}, {0x00,0x05,0x03,0x00,0x00}, {0x00,0x1C,0x22,0x41,0x00},
//     {0x00,0x41,0x22,0x1C,0x00}, {0x14,0x08,0x3E,0x08,0x14}, {0x08,0x08,0x3E,0x08,0x08},
//     {0x00,0x50,0x30,0x00,0x00}, {0x08,0x08,0x08,0x08,0x08}, {0x00,0x60,0x60,0x00,0x00},
//     {0x20,0x10,0x08,0x04,0x02}, {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
//     {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10},
//     {0x27,0x45,0x45,0x45,0x39}, {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
//     {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}, {0x00,0x36,0x36,0x00,0x00},
//     {0x00,0x56,0x36,0x00,0x00}, {0x08,0x14,0x22,0x41,0x00}, {0x14,0x14,0x14,0x14,0x14},
//     {0x00,0x41,0x22,0x14,0x08}, {0x02,0x01,0x51,0x09,0x06}, {0x32,0x49,0x79,0x41,0x3E},
//     {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36}, {0x3E,0x41,0x41,0x41,0x22},
//     {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
//     {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F}, {0x00,0x41,0x7F,0x41,0x00},
//     {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
//     {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F}, {0x3E,0x41,0x41,0x41,0x3E},
//     {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
//     {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01}, {0x3F,0x40,0x40,0x40,0x3F},
//     {0x1F,0x20,0x40,0x20,0x1F}, {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
//     {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
// };

// static void drawChar5x7(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t scale = 2)
// {
//     c = toupper((unsigned char)c);
//     if (c < 32 || c > 90) c = '?';
//     const uint8_t* glyph = simpleFont[c - 32];

//     for (int col = 0; col < 5; col++)
//     {
//         uint8_t line = pgm_read_byte(&glyph[col]);
//         for (int row = 0; row < 8; row++)
//         {
//             uint16_t drawColor = (line & (1 << row)) ? color : bg;
//             for (int sx = 0; sx < scale; sx++)
//             {
//                 for (int sy = 0; sy < scale; sy++)
//                 {
//                     tftDrawPixel(x + col * scale + sx, y + row * scale + sy, drawColor);
//                 }
//             }
//         }
//     }
// }

// void drawText(int x, int y, const String& text, uint16_t color, uint16_t bg, uint8_t scale)
// {
//     int cursorX = x;
//     for (size_t i = 0; i < text.length(); i++)
//     {
//         drawChar5x7(cursorX, y, text[i], color, bg, scale);
//         cursorX += 6 * scale;
//     }
// }

// static void clearField(int x, int y, int w, int h, uint16_t bg)
// {
//     tftSetAddrWindow(x, y, x + w - 1, y + h - 1);

//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

//     for (int i = 0; i < w * h; i++)
//     {
//         tftSPI.transfer16(bg);
//     }

//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// // =====================================================
// // PUBLIC API
// // =====================================================
// void tftInit()
// {
//     pinMode(TFT_DC, OUTPUT);
//     pinMode(TFT_RST, OUTPUT);
//     pinMode(TFT_BL, OUTPUT);
//     if (TFT_CS >= 0) pinMode(TFT_CS, OUTPUT);

//     digitalWrite(TFT_BL, HIGH);

//     tftSPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
//     tftSPI.beginTransaction(SPISettings(27000000, MSBFIRST, SPI_MODE3));

//     tftReset();

//     tftWriteCommand(ST7789_SWRESET);
//     delay(150);

//     tftWriteCommand(ST7789_SLPOUT);
//     delay(150);

//     tftWriteCommand(ST7789_COLMOD);
//     tftWriteData(0x55);

//     tftWriteCommand(ST7789_MADCTL);
//     tftWriteData(0x00);

//     tftWriteCommand(ST7789_INVON);
//     delay(10);

//     tftWriteCommand(ST7789_NORON);
//     delay(10);

//     tftWriteCommand(ST7789_DISPON);
//     delay(120);
// }

// void drawStaticUI()
// {
//     uint16_t bg = rgb565(0, 0, 0);
//     tftFillScreen(bg);

//     drawText(20, 12, "Sensor Networks", rgb565(255,255,0), bg, 2);
//     drawText(20, 55, "HUMI :", rgb565(0,255,255), bg, 2);
//     drawText(20, 90, "TEMP :", rgb565(255,0,0), bg, 2);
//     drawText(20, 125, "SOIL :", rgb565(0,255,0), bg, 2);
//     drawText(20, 170, "BOOTING", rgb565(255,255,255), bg, 3);
//     prevTemp = -1000.0f;
//     prevHum = -1000.0f;
//     prevSoil = -1;
//     prevStatus = "";
// }

// void updateValues(float temp, float hum, int soil, const String &status)
// {
//     uint16_t bg = rgb565(0, 0, 0);

//     if (hum != prevHum)
//     {
//         clearField(110, 52, 120, 28, bg);
//         drawText(110, 55, String(hum, 1) + " %", rgb565(0,255,255), bg, 2);
//         prevHum = hum;
//     }

//     if (temp != prevTemp)
//     {
//         clearField(110, 87, 120, 28, bg);
//         drawText(110, 90, String(temp, 1) + " C", rgb565(255,0,0), bg, 2);
//         prevTemp = temp;
//     }

//     if (soil != prevSoil)
//     {
//         clearField(110, 122, 120, 28, bg);
//         drawText(110, 125, String(soil) + " %", rgb565(0,255,0), bg, 2);
//         prevSoil = soil;
//     }

//     if (status != prevStatus)
//     {
//         clearField(20, 166, 200, 40, bg);

//         if (status == "WARNING")
//             drawText(20, 170, "WARNING", rgb565(255,0,0), bg, 3);
//         else if (status == "NORMAL")
//             drawText(20, 170, "NORMAL", rgb565(0,255,0), bg, 3);
//         else if (status == "MANUAL")
//             drawText(20, 170, "MANUAL", rgb565(255,165,0), bg, 3);
//         else
//             drawText(20, 170, status, rgb565(255,255,255), bg, 3);

//         prevStatus = status;
//     }
// }















// #include "display.h"
// #include <SPI.h>

// // =====================================================
// // TFT SPI PINS
// // =====================================================

// // GND  -> GND
// // VCC  -> 3V3
// // SCK  -> GPIO 18
// // SDA  -> GPIO 23
// // RES  -> GPIO 4
// // DC   -> GPIO 2
// // BLK  -> GPIO 15

// #define TFT_DC   2
// #define TFT_RST  4
// #define TFT_BL   15
// #define TFT_MOSI 23
// #define TFT_SCLK 18
// #define TFT_CS   -1

// // =====================================================
// // TFT SIZE
// // =====================================================
// #define TFT_WIDTH   240
// #define TFT_HEIGHT  240

// // =====================================================
// // ST7789 COMMANDS
// // =====================================================
// #define ST7789_SWRESET 0x01
// #define ST7789_SLPOUT  0x11
// #define ST7789_NORON   0x13
// #define ST7789_INVON   0x21
// #define ST7789_INVOFF  0x20
// #define ST7789_DISPON  0x29
// #define ST7789_CASET   0x2A
// #define ST7789_RASET   0x2B
// #define ST7789_RAMWR   0x2C
// #define ST7789_COLMOD  0x3A
// #define ST7789_MADCTL  0x36

// SPIClass tftSPI(VSPI);

// // =====================================================
// // PREVIOUS VALUES FOR TFT
// // =====================================================
// static float prevTemp = -1000.0f;
// static float prevHum  = -1000.0f;
// static int   prevSoil = -1;
// static String prevStatus = "";
// static bool errorScreenActive = false;

// // =====================================================
// // COLOR HELPER
// // =====================================================
// uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
// {
//     return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
// }

// // =====================================================
// // TFT LOW LEVEL
// // =====================================================
// static void tftWriteCommand(uint8_t cmd)
// {
//     digitalWrite(TFT_DC, LOW);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer(cmd);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// static void tftWriteData(uint8_t data)
// {
//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer(data);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// static void tftSetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
// {
//     tftWriteCommand(ST7789_CASET);
//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer16(x0);
//     tftSPI.transfer16(x1);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);

//     tftWriteCommand(ST7789_RASET);
//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer16(y0);
//     tftSPI.transfer16(y1);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);

//     tftWriteCommand(ST7789_RAMWR);
// }

// static void tftReset()
// {
//     digitalWrite(TFT_RST, HIGH);
//     delay(50);
//     digitalWrite(TFT_RST, LOW);
//     delay(150);
//     digitalWrite(TFT_RST, HIGH);
//     delay(150);
// }

// static void tftFillScreen(uint16_t color)
// {
//     tftSetAddrWindow(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

//     for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++)
//     {
//         tftSPI.transfer16(color);
//     }

//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// static void tftDrawPixel(int x, int y, uint16_t color)
// {
//     if (x < 0 || x >= TFT_WIDTH || y < 0 || y >= TFT_HEIGHT) return;

//     tftSetAddrWindow(x, y, x, y);

//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
//     tftSPI.transfer16(color);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// // =====================================================
// // SIMPLE FONT 5x7
// // =====================================================
// static const uint8_t simpleFont[96][5] PROGMEM = {
//     {0x00,0x00,0x00,0x00,0x00}, {0x00,0x00,0x5F,0x00,0x00}, {0x00,0x07,0x00,0x07,0x00},
//     {0x14,0x7F,0x14,0x7F,0x14}, {0x24,0x2A,0x7F,0x2A,0x12}, {0x23,0x13,0x08,0x64,0x62},
//     {0x36,0x49,0x55,0x22,0x50}, {0x00,0x05,0x03,0x00,0x00}, {0x00,0x1C,0x22,0x41,0x00},
//     {0x00,0x41,0x22,0x1C,0x00}, {0x14,0x08,0x3E,0x08,0x14}, {0x08,0x08,0x3E,0x08,0x08},
//     {0x00,0x50,0x30,0x00,0x00}, {0x08,0x08,0x08,0x08,0x08}, {0x00,0x60,0x60,0x00,0x00},
//     {0x20,0x10,0x08,0x04,0x02}, {0x3E,0x51,0x49,0x45,0x3E}, {0x00,0x42,0x7F,0x40,0x00},
//     {0x42,0x61,0x51,0x49,0x46}, {0x21,0x41,0x45,0x4B,0x31}, {0x18,0x14,0x12,0x7F,0x10},
//     {0x27,0x45,0x45,0x45,0x39}, {0x3C,0x4A,0x49,0x49,0x30}, {0x01,0x71,0x09,0x05,0x03},
//     {0x36,0x49,0x49,0x49,0x36}, {0x06,0x49,0x49,0x29,0x1E}, {0x00,0x36,0x36,0x00,0x00},
//     {0x00,0x56,0x36,0x00,0x00}, {0x08,0x14,0x22,0x41,0x00}, {0x14,0x14,0x14,0x14,0x14},
//     {0x00,0x41,0x22,0x14,0x08}, {0x02,0x01,0x51,0x09,0x06}, {0x32,0x49,0x79,0x41,0x3E},
//     {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36}, {0x3E,0x41,0x41,0x41,0x22},
//     {0x7F,0x41,0x41,0x22,0x1C}, {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
//     {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F}, {0x00,0x41,0x7F,0x41,0x00},
//     {0x20,0x40,0x41,0x3F,0x01}, {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
//     {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F}, {0x3E,0x41,0x41,0x41,0x3E},
//     {0x7F,0x09,0x09,0x09,0x06}, {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
//     {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01}, {0x3F,0x40,0x40,0x40,0x3F},
//     {0x1F,0x20,0x40,0x20,0x1F}, {0x3F,0x40,0x38,0x40,0x3F}, {0x63,0x14,0x08,0x14,0x63},
//     {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
// };

// static void drawChar5x7(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t scale = 2)
// {
//     c = toupper((unsigned char)c);
//     if (c < 32 || c > 90) c = '?';
//     const uint8_t* glyph = simpleFont[c - 32];

//     for (int col = 0; col < 5; col++)
//     {
//         uint8_t line = pgm_read_byte(&glyph[col]);
//         for (int row = 0; row < 8; row++)
//         {
//             uint16_t drawColor = (line & (1 << row)) ? color : bg;
//             for (int sx = 0; sx < scale; sx++)
//             {
//                 for (int sy = 0; sy < scale; sy++)
//                 {
//                     tftDrawPixel(x + col * scale + sx, y + row * scale + sy, drawColor);
//                 }
//             }
//         }
//     }
// }

// void drawText(int x, int y, const String& text, uint16_t color, uint16_t bg, uint8_t scale)
// {
//     int cursorX = x;
//     for (size_t i = 0; i < text.length(); i++)
//     {
//         drawChar5x7(cursorX, y, text[i], color, bg, scale);
//         cursorX += 6 * scale;
//     }
// }

// static void clearField(int x, int y, int w, int h, uint16_t bg)
// {
//     tftSetAddrWindow(x, y, x + w - 1, y + h - 1);

//     digitalWrite(TFT_DC, HIGH);
//     if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

//     for (int i = 0; i < w * h; i++)
//     {
//         tftSPI.transfer16(bg);
//     }

//     if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
// }

// // =====================================================
// // PUBLIC API
// // =====================================================
// void tftInit()
// {
//     pinMode(TFT_DC, OUTPUT);
//     pinMode(TFT_RST, OUTPUT);
//     pinMode(TFT_BL, OUTPUT);
//     if (TFT_CS >= 0) pinMode(TFT_CS, OUTPUT);

//     digitalWrite(TFT_BL, HIGH);

//     tftSPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
//     tftSPI.beginTransaction(SPISettings(27000000, MSBFIRST, SPI_MODE3));

//     tftReset();

//     tftWriteCommand(ST7789_SWRESET);
//     delay(150);

//     tftWriteCommand(ST7789_SLPOUT);
//     delay(150);

//     tftWriteCommand(ST7789_COLMOD);
//     tftWriteData(0x55);

//     tftWriteCommand(ST7789_MADCTL);
//     tftWriteData(0x00);

//     tftWriteCommand(ST7789_INVON);
//     delay(10);

//     tftWriteCommand(ST7789_NORON);
//     delay(10);

//     tftWriteCommand(ST7789_DISPON);
//     delay(120);
// }

// void drawStaticUI()
// {
//     uint16_t bg = rgb565(0, 0, 0);
//     tftFillScreen(bg);

//     drawText(20, 12, "Sensor Networks", rgb565(255,255,0), bg, 2);
//     drawText(20, 55, "HUMI :", rgb565(0,255,255), bg, 2);
//     drawText(20, 90, "TEMP :", rgb565(255,0,0), bg, 2);
//     drawText(20, 125, "SOIL :", rgb565(0,255,0), bg, 2);
//     drawText(20, 170, "BOOTING", rgb565(255,255,255), bg, 3);

//     prevTemp = -1000.0f;
//     prevHum = -1000.0f;
//     prevSoil = -1;
//     prevStatus = "";
//     errorScreenActive = false;
// }

// void updateValues(float temp, float hum, int soil, const String &status)
// {
//     uint16_t bg = rgb565(0, 0, 0);

//     if (errorScreenActive)
//     {
//         drawStaticUI();
//         errorScreenActive = false;
//     }

//     if (hum != prevHum)
//     {
//         clearField(110, 52, 120, 28, bg);
//         drawText(110, 55, String(hum, 1) + " %", rgb565(0,255,255), bg, 2);
//         prevHum = hum;
//     }

//     if (temp != prevTemp)
//     {
//         clearField(110, 87, 120, 28, bg);
//         drawText(110, 90, String(temp, 1) + " C", rgb565(255,0,0), bg, 2);
//         prevTemp = temp;
//     }

//     if (soil != prevSoil)
//     {
//         clearField(110, 122, 120, 28, bg);
//         drawText(110, 125, String(soil) + " %", rgb565(0,255,0), bg, 2);
//         prevSoil = soil;
//     }

//     if (status != prevStatus)
//     {
//         clearField(20, 166, 200, 40, bg);

//         if (status == "WARNING")
//             drawText(20, 170, "WARNING", rgb565(255,0,0), bg, 3);
//         else if (status == "NORMAL")
//             drawText(20, 170, "NORMAL", rgb565(0,255,0), bg, 3);
//         else if (status == "MANUAL")
//             drawText(20, 170, "MANUAL", rgb565(255,165,0), bg, 3);
//         else if (status == "AUTO PUMP")
//             drawText(20, 170, "AUTO PUMP", rgb565(255,165,0), bg, 3);
//         else if (status == "TIMER")
//             drawText(20, 170, "TIMER", rgb565(0,191,255), bg, 3);
//         else
//             drawText(20, 170, status, rgb565(255,255,255), bg, 3);

//         prevStatus = status;
//     }
// }

// void showSensorError()
// {
//     uint16_t bg = rgb565(0, 0, 0);
//     if (!errorScreenActive)
//     {
//         tftFillScreen(bg);
//         drawText(20, 100, "SENSOR ERROR", rgb565(255, 0, 0), bg, 3);
//         errorScreenActive = true;
//     }

//     prevTemp = -1000.0f;
//     prevHum = -1000.0f;
//     prevSoil = -1;
//     prevStatus = "SENSOR ERROR";
// }

// void restoreNormalUI()
// {
//     drawStaticUI();
// }









#include "display.h"
#include <SPI.h>

#define TFT_DC   2
#define TFT_RST  4
#define TFT_BL   15
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS   -1

#define TFT_WIDTH   240
#define TFT_HEIGHT  240

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

static float prevTemp = -1000.0f;
static float prevHum  = -1000.0f;
static int   prevSoil = -1;
static String prevStatus = "";
static bool errorScreenActive = false;

uint16_t rgb565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void tftWriteCommand(uint8_t cmd)
{
    digitalWrite(TFT_DC, LOW);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer(cmd);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

static void tftWriteData(uint8_t data)
{
    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer(data);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

static void tftSetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
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

static void tftReset()
{
    digitalWrite(TFT_RST, HIGH);
    delay(50);
    digitalWrite(TFT_RST, LOW);
    delay(150);
    digitalWrite(TFT_RST, HIGH);
    delay(150);
}

static void tftFillScreen(uint16_t color)
{
    tftSetAddrWindow(0, 0, TFT_WIDTH - 1, TFT_HEIGHT - 1);

    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

    for (uint32_t i = 0; i < (uint32_t)TFT_WIDTH * TFT_HEIGHT; i++)
        tftSPI.transfer16(color);

    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

static void tftDrawPixel(int x, int y, uint16_t color)
{
    if (x < 0 || x >= TFT_WIDTH || y < 0 || y >= TFT_HEIGHT) return;

    tftSetAddrWindow(x, y, x, y);

    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);
    tftSPI.transfer16(color);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

static const uint8_t simpleFont[96][5] PROGMEM = {
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

static void drawChar5x7(int x, int y, char c, uint16_t color, uint16_t bg, uint8_t scale = 2)
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
                for (int sy = 0; sy < scale; sy++)
                    tftDrawPixel(x + col * scale + sx, y + row * scale + sy, drawColor);
        }
    }
}

void drawText(int x, int y, const String& text, uint16_t color, uint16_t bg, uint8_t scale)
{
    int cursorX = x;
    for (size_t i = 0; i < text.length(); i++)
    {
        drawChar5x7(cursorX, y, text[i], color, bg, scale);
        cursorX += 6 * scale;
    }
}

static void clearField(int x, int y, int w, int h, uint16_t bg)
{
    tftSetAddrWindow(x, y, x + w - 1, y + h - 1);

    digitalWrite(TFT_DC, HIGH);
    if (TFT_CS >= 0) digitalWrite(TFT_CS, LOW);

    for (int i = 0; i < w * h; i++)
        tftSPI.transfer16(bg);

    if (TFT_CS >= 0) digitalWrite(TFT_CS, HIGH);
}

void tftInit()
{
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_BL, OUTPUT);
    if (TFT_CS >= 0) pinMode(TFT_CS, OUTPUT);

    digitalWrite(TFT_BL, HIGH);

    tftSPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    tftSPI.beginTransaction(SPISettings(27000000, MSBFIRST, SPI_MODE3));

    tftReset();
    tftWriteCommand(ST7789_SWRESET); delay(150);
    tftWriteCommand(ST7789_SLPOUT);  delay(150);
    tftWriteCommand(ST7789_COLMOD);  tftWriteData(0x55);
    tftWriteCommand(ST7789_MADCTL);  tftWriteData(0x00);
    tftWriteCommand(ST7789_INVON);    delay(10);
    tftWriteCommand(ST7789_NORON);    delay(10);
    tftWriteCommand(ST7789_DISPON);   delay(120);
}

void drawStaticUI()
{
    uint16_t bg = rgb565(0, 0, 0);
    tftFillScreen(bg);

    drawText(20, 12, "Sensor Networks", rgb565(255,255,0), bg, 2);
    drawText(20, 55, "HUMI :", rgb565(0,255,255), bg, 2);
    drawText(20, 90, "TEMP :", rgb565(255,0,0), bg, 2);
    drawText(20, 125, "SOIL :", rgb565(0,255,0), bg, 2);
    drawText(20, 170, "BOOTING", rgb565(255,255,255), bg, 3);

    prevTemp = -1000.0f;
    prevHum = -1000.0f;
    prevSoil = -1;
    prevStatus = "";
    errorScreenActive = false;
}

void updateValues(float temp, float hum, int soil, const String &status)
{
    uint16_t bg = rgb565(0, 0, 0);

    if (errorScreenActive)
    {
        drawStaticUI();
        errorScreenActive = false;
    }

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
        else if (status == "AUTO PUMP")
            drawText(20, 170, "AUTO PUMP", rgb565(255,165,0), bg, 3);
        else if (status == "TIMER")
            drawText(20, 170, "TIMER", rgb565(0,191,255), bg, 3);
        else
            drawText(20, 170, status, rgb565(255,255,255), bg, 3);

        prevStatus = status;
    }
}

void showSensorError()
{
    uint16_t bg = rgb565(0, 0, 0);
    tftFillScreen(bg);
    drawText(20, 100, "SENSOR ERROR", rgb565(255, 0, 0), bg, 3);

    prevTemp = -1000.0f;
    prevHum = -1000.0f;
    prevSoil = -1;
    prevStatus = "SENSOR ERROR";
    errorScreenActive = true;
}

void restoreNormalUI()
{
    drawStaticUI();
}