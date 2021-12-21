#include "display.h"

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

void initDisplay() {
  display.init();
  display.setRotation(1);
  display.fillScreen(GxEPD_BLACK);
  display.setTextColor(GxEPD_WHITE);
  display.setFullWindow();
}

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

void drawBotd(const char *name, const char *tag, const char *price) {
  // Display BOTD bitmap
  do {
    display.drawBitmap(0, 0, Bitmap800x480_1, display.width(), NAME_MARGIN_TOP, GxEPD_WHITE);

    uint16_t finish_y = displayBurgerName(name);
    displayTag(tag, finish_y);
    displayPrice(price);
  }
  while (display.nextPage());

  display.hibernate();
}