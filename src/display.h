#ifndef DISPLAY_H
#define DISPLAY_H

#include "GxEPD2_GFX.h"
#include <SPI.h>                      // Built-in 
#define  ENABLE_GxEPD2_display 1
#include <GxEPD2_BW.h>
#include "fonts/DK_Lemon_Yellow_Sun40pt7b.h"
#include "fonts/A_little_sunshine15pt7b.h"
#include "fonts/A_little_sunshine20pt7b.h"
#include "fonts/A_little_sunshine30pt7b.h"
#include "fonts/A_little_sunshine40pt7b.h"
#include "assets/bitmaps.h"

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

void initDisplay();
uint16_t displayBurgerName(const char *_name);
void displayTag(const char *_tag, uint16_t y);
void displayPrice(const char *_price);
void drawBotd(const char *name, const char *tag, const char *price);

#endif // DISPLAY_H_