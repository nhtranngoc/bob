#include <ArduinoJson.h>              // https://github.com/bblanchon/ArduinoJson needs version v6 or above
#include <WiFi.h>                     // Built-in
#include "time.h"                     // Built-in
#include <SPI.h>                      // Built-in 
#define  ENABLE_GxEPD2_display 1
#include <GxEPD2_BW.h>
#include <U8g2_for_Adafruit_GFX.h>
#include "fonts/DK_Lemon_Yellow_Sun40pt7b.h"
#include "fonts/A_little_sunshine15pt7b.h"
#include "fonts/A_little_sunshine20pt7b.h"
#include "fonts/A_little_sunshine30pt7b.h"
#include "fonts/A_little_sunshine40pt7b.h"
#include "assets/bitmaps.h"
#include "GxEPD2_GFX.h"

// Constant values for name/tag/price text boxes
#define NAME_H 68
#define TAG_H 30
#define PRICE_H 74
#define PRICE_X 171

// Connections for e.g. LOLIN D32
static const uint8_t EPD_BUSY = 4;  // to EPD BUSY
static const uint8_t EPD_CS   = 5;  // to EPD CS
static const uint8_t EPD_RST  = 22; // to EPD RST
static const uint8_t EPD_DC   = 21; // to EPD DC
static const uint8_t EPD_SCK  = 18; // to EPD CLK
static const uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 23; // to EPD DIN

// Connections for e.g. Waveshare ESP32 e-Paper Driver Board
//static const uint8_t EPD_BUSY = 25;
//static const uint8_t EPD_CS   = 15;
//static const uint8_t EPD_RST  = 26; 
//static const uint8_t EPD_DC   = 27; 
//static const uint8_t EPD_SCK  = 13;
//static const uint8_t EPD_MISO = 12; // Master-In Slave-Out not used, as no data from display
//static const uint8_t EPD_MOSI = 14;

GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RST, /*BUSY=*/ EPD_BUSY));   // B/W display

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // Select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
// Using fonts:
// u8g2_font_helvB08_tf
// u8g2_font_helvB10_tf
// u8g2_font_helvB12_tf
// u8g2_font_helvB14_tf
// u8g2_font_helvB18_tf
// u8g2_font_helvB24_tf

// We want to split a burger name into lines if it's too long
// If a line is longer than X characters, dictated by font size (const) & screen size (const)
// In this case, 800x480, X = 14
#define MAX_CHAR_PER_LINE 14
void renderBurgerName(char *name) {
  // Divide string length by X we get number of lines Y
  // We can allocate at most Z lines where Z = 4. Anything over that will have to suffice
  const uint8_t lines = 1 + (strlen(name) / MAX_CHAR_PER_LINE);

  // Since text height is fixed, we adjust starting y position based on number of lines
  const uint16_t starting_y = ((display.height() - 40) / 2);
}

// Height and width of the price is constant, so we're hardcoding this
void displayPrice(const char *_price) {
  display.setFont(&A_little_sunshine40pt7b);
  display.setCursor(PRICE_X, display.height() - PRICE_H);
  display.print(_price);
}

const char burgerName[] = "New Bacon-ings";
const char tagLine[] = "(Comes on Rye w/ Mustard, Cheese & Avocado)";
const char price[] = "$5.95";

void drawBitmaps800x480() {
  // Display BOTD bitmap
  display.drawBitmap(0, 0, Bitmap800x480_1, 480, 223, GxEPD_WHITE);

  // Display burger name and tag
  display.setFont(&DK_Lemon_Yellow_Sun40pt7b);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(burgerName, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  Serial.println(tbh);
  display.setCursor(x, y - 15);
  display.print(burgerName);

  display.setFont(&A_little_sunshine15pt7b);
  display.getTextBounds(tagLine, 0, 0, &tbx, &tby, &tbw, &tbh);
  x = ((display.width() - tbw) / 2) - tbx;
  display.setCursor(x, y + 85);
  Serial.println(tbh);
  display.print(tagLine);

  displayPrice(price);
}

void initDisplay() {
  display.init();
  display.setRotation(1);
  display.fillScreen(GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFullWindow();
}

//#########################################################################################
void setup() {
  Serial.begin(115200);
  initDisplay();
  do {
    drawBitmaps800x480();
  }
  while (display.nextPage());
  display.hibernate();
}
//#########################################################################################
void loop() { // this will never run!
}
