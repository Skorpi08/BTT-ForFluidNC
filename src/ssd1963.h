#ifndef _ssd1963_H_
#define _ssd1963_H_
#include "sram.h"
#include "Adafruit_GFX.h"

#define WIDTH TFT_WIDTH
#define HEIGHT TFT_HEIGHT

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

extern DMA_HandleTypeDef hdma_fsmc;

#define lcdCommand (*fsmcCommand)
#define lcdData (*fsmcData)

/*****************************************************************************/

class Adafruit_GFX_16Bit : public Adafruit_GFX
{
public:
  Adafruit_GFX_16Bit(void);

  uint16_t color565(uint8_t r, uint8_t g, uint8_t b);
  void begin();
  void drawPixel(int16_t x, int16_t y, uint16_t color);
  void setRotation(uint8_t x);
  void setAddrWindow(int16_t x1, int16_t y1, int16_t x2, int16_t y2);
  void writeCommand(uint16_t c);
  void writeData(uint16_t d);
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h);

  void pushColors(uint16_t *colors, uint32_t len, int16_t x, int16_t y);
  void pushColors(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void pushColors(const uint16_t *data, uint32_t len);

  void drawFastHLine(int16_t x0, int16_t y0, int16_t w, uint16_t color);
  void drawFastVLine(int16_t x0, int16_t y0, int16_t h, uint16_t color);
  void fillScreen(uint16_t color);
  void writePixels(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
  void writeAddress(uint16_t start, uint16_t end);

private:
};

#endif
