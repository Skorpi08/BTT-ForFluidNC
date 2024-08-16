
#include <ssd1963.h>
#include "GrblParserC.h"
#include "pin_config.h"
#include "fnc.h"

Adafruit_GFX_16Bit tft;
HardwareSerial Serial2(PIN_SERIAL_RX2, PIN_SERIAL_TX2); //  UART2

#define FNCSerial Serial2
#define DebugSerial Serial

// local copies so we can do one update function
String myState = "Err";
float myAxes[MAX_N_AXIS] = {0};
int my_n_axis = 6;
bool myLimits[MAX_N_AXIS] = {false};
bool myProbe = false;
bool use_mm = true;

void updateDisplay();
void drawUI(float axisValue, bool limit, int index);

extern "C" int fnc_getchar()
{
    if (FNCSerial.available())
    {
        return FNCSerial.read();
    }
    return -1;
}
extern "C" void fnc_putchar(uint8_t c)
{
    FNCSerial.write(c);
}
extern "C" void debug_putchar(char c)
{
    DebugSerial.write(c);
}
extern "C" int milliseconds()
{
    return millis();
}

extern "C" void show_state(const char *state)
{
    myState = state;
    updateDisplay();
}

extern "C" void show_limits(bool probe, const bool *limits, size_t n_axis)
{
    if (myProbe != probe)
    {
        myProbe = probe;
    }
    updateDisplay();
}

extern "C" void show_dro(const pos_t *axes, const pos_t *wcos, bool isMpos, bool *limits, size_t n_axis)
{
    my_n_axis = n_axis;
    for (int i = 0; i < n_axis; i++)
    {
        myAxes[i] = axes[i];
    }
    for (int i = 0; i < n_axis; i++)
    {
        myLimits[i] = limits[i];
    }
    updateDisplay(); 
}

void setup(void)
{
    FNCSerial.begin(115200);
    pinMode(PIN_LCD_BL, OUTPUT);
    digitalWrite(PIN_LCD_BL, HIGH);
    tft.begin();

    tft.fillScreen(BLACK);
    tft.setTextSize(4);
    tft.drawRGBBitmap(240, 155, logo, 320, 170);
    delay(2000);
    tft.fillScreen(BLACK);
    tft.drawRGBBitmap(380, 40, logo, 320, 170);

    updateDisplay();
    fnc_putchar('?'); 
}

void loop(void)
{
    fnc_poll();
}

void updateDisplay()
{
    if (myState == "Alarm")
    {
        tft.setTextColor(RED, BLACK);
    }
    else if (myState.startsWith("Hold"))
    {
        tft.setTextColor(YELLOW, BLACK);
    }
    else
    {
        tft.setTextColor(GREEN, BLACK);
    }
    tft.setCursor(0, 0);
    tft.print(myState);

    for (int i = 0; i < my_n_axis; i++)
    {
        drawUI(myAxes[i], myLimits[i], i);
    }
}

void drawUI(float axisValue, bool limit, int index)
{
    const char *axes[] = {"X", "Y", "Z", "A", "B", "C"};
    int yOffset = 40;                     // Abstand zwischen den Zeilen
    int yPosition = 40 + index * yOffset; // Berechnung der vertikalen Position basierend auf dem Index

    // Zeichne Achsenname und Wert
    tft.setCursor(0, yPosition);
    tft.print(axes[index]);
    tft.print(": ");
    if (use_mm)
    {
        tft.print(axisValue, 2); //  Nachkommastelle anzeigen
    }
    else
    {
        tft.print(axisValue, 3); //  Nachkommastelle anzeigen
    }

    // Zeichne Statusanzeige für Limit-Schalter
    int boxX = 250;       // X-Position des Rechtecks
    int boxY = yPosition; // Y-Position des Rechtecks
    if (limit)
    {
        tft.fillRect(boxX, boxY, 25, 25, GREEN);
    }
    else
    {
        tft.fillRect(boxX, boxY, 25, 25, BLACK);
        tft.drawRect(boxX, boxY, 25, 25, GREEN);
    }
}