#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <esp_sleep.h>

#include "globals.h"
#include "display.h"
#include "buttons.h"
#include "ble_handler.h"
#include "power_management.h"
#include "settings.h"

// ============================================================================
// GLOBAL VARIABLE DEFINITIONS (declared in globals.h)
// ============================================================================

TFT_eSPI tft = TFT_eSPI();
BLECharacteristic *pCharacteristic;
Page currentPage = PAGE_MAIN_MENU;
String currentScreenName = "BLE Ready";

bool isConnected = false;
int batteryLevel = -1;
bool isAsleep = false;

unsigned long lastActivityTime = 0;
unsigned long SLEEP_TIMEOUT = 900000; // Default: 15 minutes

int mainMenuSelection = 0;

bool inSettingsMenu = false;
bool inSubMenu = false;
int settingsMenuIndex = 0;
int subMenuIndex = 0;
int settingsScrollOffset = 0;
int subMenuScrollOffset = 0;

int displayMessageIndex = -1;
int totalMessages = 0;

// Message buffering
String messageBuffer = "";
unsigned long lastDataTime = 0;
unsigned long lastMessageReceivedTime = 0;
String messageHistory[MAX_MESSAGES];

// Button state
int lastButton1State = HIGH;
int lastButton2State = HIGH;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long button1PressTime = 0;
unsigned long button2PressTime = 0;
bool button1LongPress = false;
bool button2LongPress = false;

// Connection and battery status
unsigned long lastBatteryCheck = 0;

// Brightness control
int brightness = 100; // Default brightness (100%)
unsigned long brightnessDisplayTime = 0;
bool brightnessChanged = false;
bool showingBrightness = false;

// Main Menu
const char* mainMenuItems[] = {"Messages", "Settings", "Info"};
const int NUM_MAIN_MENU_ITEMS = 3;

// Visible items calculation
const int MAIN_MENU_VISIBLE_ITEMS = 5;    // Increased from 4 to 5
const int SUB_MENU_VISIBLE_ITEMS = 3;     // Start scrolling after 3rd item
const int INFO_MENU_VISIBLE_ITEMS = 3;    // For the main info screen

// Settings items
const char* settingsItems[] = {
  "Brightness",
  "Auto Standby", 
  "Rotate Screen",
  "Mirror Screen",
  "Button Actions",
  "Smart Text",
  "Exit"
};
const int NUM_SETTINGS_ITEMS = 7;

// Brightness options
const char* brightnessOptions[] = {
  "10%", "20%", "30%", "40%", "50%", 
  "60%", "70%", "80%", "90%", "100%"
};
const int NUM_BRIGHTNESS_OPTIONS = 10;

// Auto Standby options  
const char* standbyOptions[] = {
  "10 Seconds", "30 Seconds", "1 Minute", "5 Minutes", "10 Minutes", "15 Minutes", "20 Minutes",
  "25 Minutes", "30 Minutes", "45 Minutes", "1 Hour", "2 Hours"
};
const int NUM_STANDBY_OPTIONS = 12;
const unsigned long standbyValues[] = {
  10000, 30000, 60000, 300000, 600000, 900000, 1200000,
  1500000, 1800000, 2700000, 3600000, 7200000
};

// ============================================================================ 
void clearAllMessages() {
    Serial.println(">>> CLEARING ALL MESSAGES");
    for (int i = 0; i < MAX_MESSAGES; i++) {
        messageHistory[i] = "";
    }
    totalMessages = 0;
    displayMessageIndex = -1;
    
    // Clear entire screen and redraw header
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(1);
    
    // Update header first
    setScreenName("Cleared");
    
    // Then display content starting well below header
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_12);
    tft.setCursor(0, CONTENT_START_Y + 30);
    tft.println("All messages");
    tft.println("cleared!");
    tft.setFreeFont(FONT_SANS_9);
    tft.setCursor(0, CONTENT_START_Y + 100);
    tft.println("Waiting for");
    tft.println("new messages...");
}

// ============================================================================ 
// SETUP
// ============================================================================ 
void setup() {
    Serial.begin(115200);
    Serial.println("Starting TTGO BLE + Display with Message History and Settings");

    // Check if we're waking from deep sleep
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0 || wakeup_reason == ESP_SLEEP_WAKEUP_EXT1) {
        wakeFromSleep();
        return; // Skip the rest of setup since we're waking from sleep
    }

    // Normal startup (not waking from sleep)
    
    // Initialize buttons. BUTTON_1 has an external pull-up, BUTTON_2 needs an internal one.
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);

    // Initialize battery pin
    pinMode(BATTERY_PIN, INPUT);

    // Load settings from EEPROM
    loadBrightness();
    loadStandbySetting();

    // Initialize display
    initializeDisplay();
    
    // Draw initial header
    updateHeader();
    
    // Display initial content starting well below header
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_12);
    tft.setCursor(0, CONTENT_START_Y + 20);
    tft.println("Starting BLE...");
    
    // Initialize BLE
    setupBLE();

    currentPage = PAGE_MAIN_MENU;
    drawMainMenu();
    
    // Initialize activity timer
    lastActivityTime = millis();
    
    Serial.println("BLE advertising started");
    Serial.println("Ready for messages...");
}

// ============================================================================ 
// LOOP - Check for timeout to process final message
// ============================================================================ 
void loop() {
    handleButtons();
    brightnessDisplayTime = millis();

    
    // Check battery level periodically
    if (millis() - lastBatteryCheck > BATTERY_CHECK_INTERVAL) {
        int newBatteryLevel = readBatteryLevel();
        if (newBatteryLevel != batteryLevel) {
            batteryLevel = newBatteryLevel;
            updateHeader();
        }
        lastBatteryCheck = millis();
    }
    
    // Check if we have buffered data and no new data has arrived for a while
    if (messageBuffer.length() > 0 && (millis() - lastDataTime) > MESSAGE_TIMEOUT) {
        Serial.println("Timeout - processing remaining buffer");
        messageBuffer.trim();
        if (messageBuffer.length() > 0) {
            addMessageToHistory(messageBuffer);
        }
        messageBuffer = "";
    }
    
    // Save brightness if it changed
    if (brightnessChanged) {
        saveBrightness();
        brightnessChanged = false;
    }
    
    // Hide brightness display after 3 seconds
    if (showingBrightness && (millis() - brightnessDisplayTime) > 3000) {
        hideBrightnessChange();
    }
    
    // Check if we should enter deep sleep
    checkSleep();
    
    delay(10);
}
