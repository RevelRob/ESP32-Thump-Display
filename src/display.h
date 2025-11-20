#ifndef DISPLAY_H
#define DISPLAY_H

#include "card_suits.h"
#include "globals.h"
#include <TFT_eSPI.h>
#include "globals.h"

// Function declarations for display-related tasks
void initializeDisplay();
void updateHeader();
void drawConnectionStatus();
void drawBatteryStatus();
void clearContentArea();
void setScreenName(String name);

void drawMainMenu();
void drawInfoPage();
void displayCard(String rank, String suit);
void drawCardSymbol(String rank, String suitChar);
void drawSuitBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color);
void displayCurrentMessage();
void drawMessageContent();
int calculateWrappedTextHeight(String message, const GFXfont* font, int maxWidth);
void updateDisplay();

void drawSettingsMenu();
void drawSubMenu();

void showBrightnessChange();
void handleBrightnessDisplay();

#endif // DISPLAY_H