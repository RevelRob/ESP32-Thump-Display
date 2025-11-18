#ifndef DISPLAY_H
#define DISPLAY_H

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
void displayCurrentMessage();
bool doesTextFit(String message, const GFXfont* font, int maxWidth, int maxHeight);

void drawSettingsMenu();
void drawSubMenu();

void showBrightnessChange();
void hideBrightnessChange();

#endif // DISPLAY_H