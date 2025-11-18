#ifndef GLOBALS_H
#define GLOBALS_H

#include <TFT_eSPI.h>
#include <BLEDevice.h>

// ============================================================================
// CONSTANTS & DEFINES
// ============================================================================

// BLE UUIDs
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

// Button pins
#define BUTTON_1 0
#define BUTTON_2 35

// Battery measurement
#define BATTERY_PIN 34

// Fonts
#define FONT_SANS_9 &FreeSans9pt7b
#define FONT_SANS_12 &FreeSans12pt7b
#define FONT_SANS_BOLD_12 &FreeSansBold12pt7b

// Dimensions
const int HEADER_HEIGHT = 28;
const int CONTENT_START_Y = 32;

// Message history
const int MAX_MESSAGES = 20;

// Timers
const unsigned long MESSAGE_TIMEOUT = 100;
const unsigned long CLEAR_TIMEOUT = 5000;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long LONG_PRESS_TIME = 1000;
const unsigned long BATTERY_CHECK_INTERVAL = 5000;

// Brightness
const int MIN_BRIGHTNESS = 10;
const int MAX_BRIGHTNESS = 100;
const int BRIGHTNESS_STEP = 10;

// EEPROM addresses
const int EEPROM_SIZE = 3;
const int BRIGHTNESS_ADDR = 0;
const int STANDBY_ADDR = 1;
const int MIRROR_ADDR = 2;

// ============================================================================
// ENUMS & STRUCTS
// ============================================================================

enum Page {
  PAGE_MAIN_MENU,
  PAGE_INFO,
  PAGE_MESSAGES,
  PAGE_SETTINGS
};

// ============================================================================
// GLOBAL VARIABLES (declared here, defined in main.cpp)
// ============================================================================

// Hardware objects
extern TFT_eSPI tft;
extern BLECharacteristic *pCharacteristic;

// Page state
extern Page currentPage;
extern String currentScreenName;

// Status
extern bool isConnected;
extern int batteryLevel;
extern bool isAsleep;

// Message History
extern String messageHistory[MAX_MESSAGES];
extern int displayMessageIndex;
extern int totalMessages;

// Timers
extern unsigned long lastActivityTime;
extern unsigned long SLEEP_TIMEOUT;

// Main Menu
extern const char* mainMenuItems[];
extern const int NUM_MAIN_MENU_ITEMS;
extern int mainMenuSelection;

// Settings Menu
extern bool inSettingsMenu;
extern bool inSubMenu;
extern int settingsMenuIndex;
extern int subMenuIndex;
extern int settingsScrollOffset;
extern int subMenuScrollOffset;

extern const char* settingsItems[];
extern const int NUM_SETTINGS_ITEMS;

// Brightness
extern int brightness;
extern bool showingBrightness;
extern bool brightnessChanged;
extern const char* brightnessOptions[];
extern const int NUM_BRIGHTNESS_OPTIONS;

// Standby
extern const char* standbyOptions[];
extern const int NUM_STANDBY_OPTIONS;
extern const unsigned long standbyValues[];

// Mirror Setting
extern bool mirrorMessages;
extern const char* mirrorOptions[];
extern const int NUM_MIRROR_OPTIONS;

// Button states
extern int lastButton1State;
extern int lastButton2State;
extern unsigned long lastDebounceTime1;
extern unsigned long lastDebounceTime2;
extern unsigned long button1PressTime;
extern unsigned long button2PressTime;
extern bool button1LongPress;
extern bool button2LongPress;

#endif // GLOBALS_H