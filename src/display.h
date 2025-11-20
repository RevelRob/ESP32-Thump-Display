#ifndef DISPLAY_H
#define DISPLAY_H

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
void displayCurrentMessage();
void drawMessageContent();
int calculateWrappedTextHeight(String message, const GFXfont* font, int maxWidth);
void updateDisplay();

void drawSettingsMenu();
void drawSubMenu();

void showBrightnessChange();
void handleBrightnessDisplay();

#endif // DISPLAY_H