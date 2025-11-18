#ifndef SETTINGS_H
#define SETTINGS_H

#include "globals.h"

// Brightness
void loadBrightness();
void saveBrightness();
void setBrightness(int level);
void adjustBrightness(int delta);

// Standby
void loadStandbySetting();
void saveStandbySetting();

// Menu Navigation
void enterSettingsMenu();
void exitSettingsMenu();
void enterSubMenu();
void exitSubMenu();
void cycleSubMenuOption();

#endif // SETTINGS_H