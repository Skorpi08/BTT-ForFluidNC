#include "ssd1963.h"

Adafruit_GFX_16Bit ::Adafruit_GFX_16Bit(void) : Adafruit_GFX(WIDTH, HEIGHT) {}
/*****************************************************************************/
static const uint8_t regValues[] = {
    0xE2, 3, 0x17, 0x04, 0x54,                         // PLL multiplier, set PLL clock to 120M
    0xE0, 1, 0x01,                                     // PLL enable
    0xFF, 10,                                          // delay(10);
    0xE0, 1, 0x03,                                     // PLL enable
    0xFF, 10,                                          // delay(10);
    0x01, 0,                                           // Software reset
    0xFF, 10,                                          // delay(10);
    0xE6, 3, 0x04, 0x93, 0xE0,                         // PLL setting for PCLK, depends on resolution
    0xB0, 7, 0x00, 0x00, 0x03, 0x1F, 0x01, 0xDF, 0x00, // LCD SPECIFICATION
    0xB4, 7, 0x03, 0xA0, 0x00, 0x2E, 0x30, 0x00, 0x00, // HSYNC
    0xB6, 7, 0x02, 0x0D, 0x00, 0x10, 0x10, 0x00, 0x00, // VSYNC
    0xF0, 1, 0x03,                                     // Set pixel data interface format
    0x29, 0,                                           // Set display on
    0x36, 1, 0x00,                                     // rotation
};

void Adafruit_GFX_16Bit::begin()
{
    fsmc_lcd_init();
    for (uint8_t i = 0; i < sizeof(regValues);)
    {
        uint8_t cmd = regValues[i++];
        uint8_t len = regValues[i++];
        if (cmd == 0xFF)
        {
            HAL_Delay(len);
        }
        else
        {
            writeCommand(cmd);
            for (uint8_t d = 0; d < len; d++)
            {
                writeData(regValues[i++]);
            }
        }
    }
}

// /*****************************************************************************/
void Adafruit_GFX_16Bit::writeCommand(uint16_t cmd) { *fsmcCommand = cmd; }
void Adafruit_GFX_16Bit::writeData(uint16_t data) { *fsmcData = data; }
void Adafruit_GFX_16Bit::setAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    writeCommand(0x2A); // Column addr set
    writeAddress(x0, x1);
    writeCommand(0x2B); // Row addr set
    writeAddress(y0, y1);
    writeCommand(0x2C); // Write to RAM
}
void Adafruit_GFX_16Bit::writeAddress(uint16_t start, uint16_t end)
{
    writeData(start >> 8);   // Höhere Byte des Startwerts
    writeData(start & 0xFF); // Niedrigere Byte des Startwerts
    writeData(end >> 8);     // Höhere Byte des Endwerts
    writeData(end & 0xFF);   // Niedrigere Byte des Endwerts
}

uint16_t Adafruit_GFX_16Bit::color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

void Adafruit_GFX_16Bit::setRotation(uint8_t m)
{
    writeCommand(0x36);
    switch (m)
    {
    case 0:
        m = 0;
        break;
    case 1:
        m = 1;
        break;
    case 2:
        m = 2;
        break;
    case 3:
        m = 3;
        break;
    }
    writeData(m);
}

void Adafruit_GFX_16Bit::pushColors(const uint16_t *colors, uint32_t len)
{

    // HAL_DMA_Start_IT(&hdma_fsmc, (uint32_t)colors, (uint32_t)&fsmcData, len);

    while (len--)
    {
        uint16_t color = *colors++;
        writeData(color);
    }
}

void Adafruit_GFX_16Bit::writePixels(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    setAddrWindow(x, y, x + w - 1, y + h - 1); 


    uint32_t numPixels = w * h;
    uint16_t *colorData = new uint16_t[numPixels];
    for (uint32_t i = 0; i < numPixels; i++)
    {
        colorData[i] = color;
    }

    HAL_DMA_Start(&hdma_fsmc, (uint32_t)colorData, (uint32_t)fsmcData, numPixels) ;
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);

    delete[] colorData; // Freigabe des Puffers
}

void Adafruit_GFX_16Bit::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    setAddrWindow(x, y, x, y);

    // DMA-Transfer für das Schreiben eines einzelnen Pixels
    HAL_DMA_Start(&hdma_fsmc, (uint32_t)&color, (uint32_t)fsmcData, 1);
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
}

void Adafruit_GFX_16Bit::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    setAddrWindow(x, y, x + w - 1, y + h - 1); 

    uint32_t numPixels = w * h;
    uint16_t *colorData = new uint16_t[numPixels];
    for (uint32_t i = 0; i < numPixels; i++)
    {
        colorData[i] = color;
    }

    HAL_DMA_Start(&hdma_fsmc, (uint32_t)colorData, (uint32_t)fsmcData, numPixels);
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);

    delete[] colorData; // Freigabe des Puffers
}

void Adafruit_GFX_16Bit::fillScreen(uint16_t color)
{
    setAddrWindow(0, 0, WIDTH - 1, HEIGHT - 1);

    uint32_t pixels = WIDTH * HEIGHT;
    uint32_t blockSize = 1000; // 64000
    uint32_t blocks = pixels / blockSize;
    uint32_t remainingPixels = pixels % blockSize;

    // Erstelle den Farbpuffer einmal mit der festen Größe `blockSize`
    uint16_t *colorData = new uint16_t[blockSize];
    for (uint32_t i = 0; i < blockSize; i++)
    {
        colorData[i] = color;
    }

    // Übertragung der vollen Blöcke
    for (uint32_t i = 0; i < blocks; i++)
    {
        HAL_DMA_Start(&hdma_fsmc, (uint32_t)colorData, (uint32_t)fsmcData, blockSize);
        HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
    }

    // Übertragung der verbleibenden Pixel, wenn notwendig
    if (remainingPixels > 0)
    {
        HAL_DMA_Start(&hdma_fsmc, (uint32_t)colorData, (uint32_t)fsmcData, remainingPixels);
        HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
    }

    // Freigabe des dynamisch allokierten Puffers
    delete[] colorData;
}

void Adafruit_GFX_16Bit::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{ //////////+++++++++++++
    int16_t x2;

    // Überprüfen, ob die Linie außerhalb des Bildschirms liegt
    if ((x >= WIDTH) || (y >= HEIGHT) || (x + w - 1 < 0) || (y < 0))
        return;

    // Korrigiere die Breite, falls sie das Ende des Bildschirms überschreitet
    if (x < 0)
    {
        w += x;
        x = 0;
    }
    if ((x + w) > WIDTH)
    {
        w = WIDTH - x;
    }

    // Setze das Adressfenster für die Linie
    setAddrWindow(x, y, x + w - 1, y);

    // Erstelle einen Puffer mit der Länge der Linie
    uint16_t *buffer = new uint16_t[w];
    for (int16_t i = 0; i < w; i++)
    {
        buffer[i] = color;
    }

    // DMA-Transfer für das Übertragen der Linie
    HAL_DMA_Start(&hdma_fsmc, (uint32_t)buffer, (uint32_t)fsmcData, w);
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);

    // Speicher freigeben
    delete[] buffer;
}

void Adafruit_GFX_16Bit::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{ //////////+++++++++++++
    // Überprüfen, ob die Linie außerhalb des Bildschirms liegt
    if ((x >= WIDTH) || (y >= HEIGHT) || (y + h - 1 < 0) || (x < 0))
        return;

    // Korrigiere die Höhe, falls sie das Ende des Bildschirms überschreitet
    if (y < 0)
    {
        h += y;
        y = 0;
    }
    if ((y + h) > HEIGHT)
    {
        h = HEIGHT - y;
    }

    // Setze das Adressfenster für die Linie
    setAddrWindow(x, y, x, y + h - 1);

    // Erstelle einen Puffer mit der Länge der Linie
    uint16_t *buffer = new uint16_t[h];
    for (int16_t i = 0; i < h; i++)
    {
        buffer[i] = color;
    }

    // DMA-Transfer für das Übertragen der Linie
    HAL_DMA_Start(&hdma_fsmc, (uint32_t)buffer, (uint32_t)fsmcData, h);
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);

    // Speicher freigeben
    delete[] buffer;
}

void Adafruit_GFX_16Bit::pushColors(uint16_t *colors, uint32_t len, int16_t x, int16_t y)
{
    // Überprüfen, ob len > 0 ist
    if (len == 0)
        return;

    // Setze das Adressfenster für den Bereich
    setAddrWindow(x, y, x + len - 1, y);

    // DMA-Transfer für das Übertragen der Farben
    HAL_DMA_Start(&hdma_fsmc, (uint32_t)colors, (uint32_t)fsmcData, len);
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
}

void Adafruit_GFX_16Bit::pushColors(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    setAddrWindow(x0, y0, x1, y1); // Setze das Adressfenster auf den Bereich
    // writeCommand(0x2C); // Memory Write

    uint32_t numPixels = (x1 - x0 + 1) * (y1 - y0 + 1);
    uint16_t *colorData = new uint16_t[numPixels];
    for (uint32_t i = 0; i < numPixels; i++)
    {
        colorData[i] = color;
    }

    // Start des DMA-Transfers
   HAL_DMA_Start(&hdma_fsmc, (uint32_t)colorData, (uint32_t)fsmcData, numPixels);
    HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);

    delete[] colorData; // Freigabe des Puffers
}

void Adafruit_GFX_16Bit::drawRGBBitmap(int16_t x, int16_t y, const uint16_t *bitmap, int16_t w, int16_t h)
{
    if (x >= 0 && (x + w) < _width && y >= 0 && (y + h) < _height)
    {
        // Wenn das gesamte Bild innerhalb des sichtbaren Bereichs liegt
        setAddrWindow(x, y, x + w - 1, y + h - 1);
        pushColors((uint16_t *)bitmap, w * h);
    }
    else
    {
        // Wenn Teile des Bildes außerhalb des sichtbaren Bereichs liegen, zeichnen wir pixelweise
        for (int16_t j = 0; j < h; j++)
        {
            for (int16_t i = 0; i < w; i++)
            {
                drawPixel(x + i, y + j, bitmap[j * w + i]);
            }
        }
    }
}

#include <cmath>   // Für abs() Funktion
#include <cstdlib> // Für abs() Funktion
#include <vector>

void Adafruit_GFX_16Bit::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    // Bresenham-Algorithmus zum Zeichnen einer Linie
    bool steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep)
    {
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    if (x0 > x1)
    {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = abs(y1 - y0);
    int error = dx / 2;
    int y = y0;
    int yStep = (y0 < y1) ? 1 : -1;

    uint32_t numPixels = dx + 1; // Anzahl der Pixel in der Linie
    uint16_t *lineData = new uint16_t[numPixels];
    for (uint32_t i = 0; i < numPixels; i++)
    {
        lineData[i] = color;
    }

    // Zeichnen der Linie
    if (steep)
    {
        for (int x = x0; x <= x1; x++)
        {
            drawPixel(y, x, color);
            error -= dy;
            if (error < 0)
            {
                y += yStep;
                error += dx;
            }
        }
    }
    else
    {
        // Adressfenster setzen
        setAddrWindow(x0, y, x1, y);
        writeCommand(0x2C); // Memory Write

        // DMA-Transfer starten
        HAL_DMA_Start(&hdma_fsmc, (uint32_t)lineData, (uint32_t)fsmcData, numPixels) ;
        HAL_DMA_PollForTransfer(&hdma_fsmc, HAL_DMA_FULL_TRANSFER, HAL_MAX_DELAY);
    }

    delete[] lineData; // Freigabe des Puffers
}
