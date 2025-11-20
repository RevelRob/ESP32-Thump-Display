#ifndef SETTINGS_H
#define SETTINGS_H

#include "globals.h"

// Brightness
void loadBrightness();
void saveBrightness();
void setBrightness(int level);

// Standby
void loadStandbySetting();
void saveStandbySetting();

// Mirror
void loadMirrorSetting();
void saveMirrorSetting();

// Smart Text
void loadSmartTextSetting();
void saveSmartTextSetting();

// Card Type
void loadCardTypeSetting();
void saveCardTypeSetting();

// Menu Navigation
void enterSettingsMenu();
void exitSettingsMenu();
void enterSubMenu();
void exitSubMenu();
void cycleSubMenuOption();

#endif // SETTINGS_H