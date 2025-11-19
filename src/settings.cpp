#include "settings.h"
#include "display.h"
#include <EEPROM.h>

// ============================================================================
// BRIGHTNESS CONTROL
// ============================================================================
void loadBrightness() {
    EEPROM.begin(EEPROM_SIZE);
    int savedBrightness = EEPROM.read(BRIGHTNESS_ADDR);
    
    if (savedBrightness >= MIN_BRIGHTNESS && savedBrightness <= MAX_BRIGHTNESS) {
        brightness = savedBrightness;
        Serial.print("Loaded brightness from EEPROM: ");
        Serial.println(brightness);
    } else {
        brightness = 100;
        saveBrightness();
        Serial.println("Using default brightness: 100%");
    }
    EEPROM.end();
    setBrightness(brightness);
}

void saveBrightness() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(BRIGHTNESS_ADDR, brightness);
    EEPROM.commit();
    EEPROM.end();
    Serial.print("Saved brightness to EEPROM: ");
    Serial.println(brightness);
}

void setBrightness(int level) {
    int oldBrightness = brightness;
    brightness = constrain(level, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    
    int pwmValue = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 25, 255);
    analogWrite(4, pwmValue);
    
    if (oldBrightness != brightness) {
        showBrightnessChange();
    }
    
    Serial.print("Brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
}

// ============================================================================
// STANDBY SETTINGS
// ============================================================================
void loadStandbySetting() {
    EEPROM.begin(EEPROM_SIZE);
    int savedStandbyIndex = EEPROM.read(STANDBY_ADDR);
    
    if (savedStandbyIndex >= 0 && savedStandbyIndex < NUM_STANDBY_OPTIONS) {
        SLEEP_TIMEOUT = standbyValues[savedStandbyIndex];
        Serial.print("Loaded standby time from EEPROM: ");
        Serial.println(SLEEP_TIMEOUT);
    } else {
        SLEEP_TIMEOUT = 900000; // Default: 15 mins
        saveStandbySetting();
        Serial.println("Using default standby: 15 minutes");
    }
    EEPROM.end();
}

void saveStandbySetting() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(STANDBY_ADDR, subMenuIndex);
    EEPROM.commit();
    EEPROM.end();
    SLEEP_TIMEOUT = standbyValues[subMenuIndex];
    Serial.print("Saved standby time to EEPROM: ");
    Serial.println(SLEEP_TIMEOUT);
}

// ============================================================================
// MIRROR SETTINGS
// ============================================================================
void loadMirrorSetting() {
    EEPROM.begin(EEPROM_SIZE);
    byte savedMirrorState = EEPROM.read(MIRROR_ADDR);
    
    if (savedMirrorState == 0 || savedMirrorState == 1) {
        mirrorMessages = (bool)savedMirrorState;
        Serial.print("Loaded mirror setting from EEPROM: ");
        Serial.println(mirrorMessages ? "On" : "Off");
    } else {
        mirrorMessages = false; // Default to Off
        EEPROM.write(MIRROR_ADDR, mirrorMessages); // Write default without calling saveMirrorSetting to avoid recursion
        Serial.println("Using default mirror setting: Off");
    }
    EEPROM.end();
}

void saveMirrorSetting() {
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.write(MIRROR_ADDR, mirrorMessages);
    EEPROM.commit();
    EEPROM.end();
    Serial.print("Saved mirror setting to EEPROM: ");
    Serial.println(mirrorMessages ? "On" : "Off");
}


// ============================================================================
// SETTINGS MENU NAVIGATION LOGIC
// ============================================================================
void enterSettingsMenu() {
    inSettingsMenu = true;
    settingsMenuIndex = 0;
    settingsScrollOffset = 0;
    subMenuScrollOffset = 0;
    Serial.println(">>> ENTERING SETTINGS MENU");
    setScreenName("Settings");
    drawSettingsMenu();
}

void exitSettingsMenu() {
    inSettingsMenu = false;
    inSubMenu = false;
    Serial.println(">>> EXITING SETTINGS MENU");
    currentPage = PAGE_MAIN_MENU;
    drawMainMenu();
}

void enterSubMenu() {
    inSubMenu = true;
    subMenuIndex = 0;
    subMenuScrollOffset = 0;
    if (settingsMenuIndex == 0) { // Brightness
        subMenuIndex = (brightness - MIN_BRIGHTNESS) / BRIGHTNESS_STEP;
    } else if (settingsMenuIndex == 1) { // Auto Standby
        EEPROM.begin(EEPROM_SIZE);
        subMenuIndex = EEPROM.read(STANDBY_ADDR);
        if (subMenuIndex < 0 || subMenuIndex >= NUM_STANDBY_OPTIONS) subMenuIndex = 5; // Default to 15 mins
        EEPROM.end();
    } else if (settingsMenuIndex == 3) { // Mirror Screen
        subMenuIndex = (int)mirrorMessages;
    }
    drawSubMenu();
}

void exitSubMenu() {
    inSubMenu = false;
    drawSettingsMenu();
}

void cycleSubMenuOption() {
    switch (settingsMenuIndex) {
        case 0: // Brightness
            subMenuIndex = (subMenuIndex + 1) % NUM_BRIGHTNESS_OPTIONS;
            setBrightness(MIN_BRIGHTNESS + (subMenuIndex * BRIGHTNESS_STEP));
            break;
        case 1: // Auto Standby
            subMenuIndex = (subMenuIndex + 1) % NUM_STANDBY_OPTIONS;
            saveStandbySetting();
            break;
        case 3: // Mirror Screen
            subMenuIndex = (subMenuIndex + 1) % NUM_MIRROR_OPTIONS;
            mirrorMessages = (bool)subMenuIndex;
            saveMirrorSetting();
            break;
        default:
            // Do nothing for unimplemented sub-menus
            break;
    }
}