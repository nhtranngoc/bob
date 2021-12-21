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

// MARGINS!
#define NAME_MARGIN_TOP 223
#define LINE_MARGIN 15
#define TAG_MARGIN_TOP 75

// We want to split a burger name into lines if it's too long
// If a line is longer than X characters, dictated by font size (const) & screen size (const)
// In this case, 800x480, X = 14
#define MAX_CHAR_PER_NAME_LINE 14
#define MAX_CHAR_PER_TAG_LINE 23
#define MAX_LINES 4

// Connections for e.g. LOLIN D32
static const uint8_t EPD_BUSY = 4;  // to EPD BUSY
static const uint8_t EPD_CS   = 5;  // to EPD CS
static const uint8_t EPD_RST  = 22; // to EPD RST
static const uint8_t EPD_DC   = 21; // to EPD DC
static const uint8_t EPD_SCK  = 18; // to EPD CLK
static const uint8_t EPD_MISO = 19; // Master-In Slave-Out not used, as no data from display
static const uint8_t EPD_MOSI = 23; // to EPD DIN

// Connections for e.g. Waveshare ESP32 e-Paper Driver Board
// static const uint8_t EPD_BUSY = 25;
// static const uint8_t EPD_CS   = 15;
// static const uint8_t EPD_RST  = 26; 
// static const uint8_t EPD_DC   = 27; 
// static const uint8_t EPD_SCK  = 13;
// static const uint8_t EPD_MISO = 12; // Master-In Slave-Out not used, as no data from display
// static const uint8_t EPD_MOSI = 14;

GxEPD2_BW<GxEPD2_750_T7, GxEPD2_750_T7::HEIGHT> display(GxEPD2_750_T7(/*CS=*/ EPD_CS, /*DC=*/ EPD_DC, /*RST=*/ EPD_RST, /*BUSY=*/ EPD_BUSY));   // B/W display

U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;  // Select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall

uint16_t displayBurgerName(const char *_name) {
  display.setFont(&DK_Lemon_Yellow_Sun40pt7b);
  // Divide string length by X we get number of lines Y
  // We can allocate at most Z lines where Z = 4. Anything over that will have to suffice
  String name = String(_name);

  // lines is a naively calculated value between 1 and 4, inclusively. This is used to calculate starting Y postition relative to the image header.
  uint8_t lines = 1 + (name.length() / MAX_CHAR_PER_NAME_LINE);
  if (lines > MAX_LINES) {
    lines = MAX_LINES;
  }
  uint16_t start_y = NAME_MARGIN_TOP + ((NAME_H /2 + LINE_MARGIN) * abs(lines - MAX_LINES - 1));
  uint16_t finish_y = start_y;
  
  uint8_t last_space = 0, counter = 0, copy_start = 0, current_line = 0;
  int16_t tbx, tby; uint16_t tbw, tbh;
  for (uint8_t i = 0; _name[i] != '\0'; i++, counter++) {
    if (isspace(_name[i])) {
      last_space = i;
    }

    if (counter >= MAX_CHAR_PER_NAME_LINE) {
      String sub = name.substring(copy_start, last_space);
      display.getTextBounds(sub, 0, 0, &tbx, &tby, &tbw, &tbh);
      uint16_t x = ((display.width() - tbw) / 2) - tbx;
      display.setCursor(x, start_y + current_line * NAME_H);
      display.print(sub);
      counter = 0;
      copy_start = last_space;
      current_line++;
    }
  }

  finish_y = start_y + current_line * NAME_H;
  String sub = name.substring(copy_start);
  display.getTextBounds(sub, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  display.setCursor(x, finish_y);
  display.print(sub);

  return finish_y;
}

void displayTag(const char *_tag, uint16_t y) {
  int16_t tbx, tby; uint16_t tbw, tbh;
  
  // A lil naive text scaling for good measure
  if (strlen(_tag) > MAX_CHAR_PER_TAG_LINE) {
    display.setFont(&A_little_sunshine15pt7b);
  } else {
    display.setFont(&A_little_sunshine30pt7b);
  }
  display.getTextBounds(_tag, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  display.setCursor(x, y + TAG_MARGIN_TOP);
  Serial.println(tbh);
  display.print(_tag);
}

// Height and width of the price is constant, so we're hardcoding this
void displayPrice(const char *_price) {
  display.setFont(&A_little_sunshine40pt7b);
  display.setCursor(PRICE_X, display.height() - PRICE_H);
  display.print(_price);
}

const char burgerName[] = "It's fun to eat at the RyeMCA Burger WHOOPE asdf asdf asldfkjasdf as";
const char tagLine[] = "(Comes on Rye with avo";
const char price[] = "$5.95";

void drawBotd() {
  // Display BOTD bitmap
  display.drawBitmap(0, 0, Bitmap800x480_1, 480, 223, GxEPD_WHITE);

  uint16_t finish_y = displayBurgerName(burgerName);
  displayTag(tagLine, finish_y);
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
    drawBotd();
  }
  while (display.nextPage());
  display.hibernate();
}
//#########################################################################################
void loop() { // this will never run!
}
