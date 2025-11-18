#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <EEPROM.h>
#include <esp_sleep.h>

// BLE UUIDs
#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"

// Correct button pins for TTGO T-Display (using external pullups)
#define BUTTON_1 0   // Right button (Bottom)
#define BUTTON_2 35  // Left button (Top)

// Battery measurement
#define BATTERY_PIN 34  // ADC pin for battery voltage measurement

BLECharacteristic *pCharacteristic;
TFT_eSPI tft = TFT_eSPI();

// Free Fonts (you can change these to any Free Fonts available in TFT_eSPI)
// Consolidated Free Font Definitions
#define FONT_SANS_9 &FreeSans9pt7b
#define FONT_SANS_12 &FreeSans12pt7b
#define FONT_SANS_BOLD_12 &FreeSansBold12pt7b

// Header dimensions for Free Fonts
const int HEADER_HEIGHT = 28;  // Slightly reduced header height
const int CONTENT_START_Y = 32; // Content starts right below header

// Message history
const int MAX_MESSAGES = 20;
String messageHistory[MAX_MESSAGES];
int displayMessageIndex = -1;
int totalMessages = 0;

// Message buffering
String messageBuffer = "";
unsigned long lastDataTime = 0;
unsigned long lastMessageReceivedTime = 0;
unsigned long lastActivityTime = 0; // Track last user/BLE activity
const unsigned long MESSAGE_TIMEOUT = 100; // ms to wait for more data in a single message
const unsigned long CLEAR_TIMEOUT = 5000;  // 5 seconds to clear old messages when new ones arrive
unsigned long SLEEP_TIMEOUT = 900000; // 15 minutes (900,000 ms) to enter deep sleep - made variable

// Button state
int lastButton1State = HIGH;
int lastButton2State = HIGH;
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long button1PressTime = 0;
unsigned long button2PressTime = 0;
bool button1LongPress = false;
bool button2LongPress = false;
const unsigned long DEBOUNCE_DELAY = 50;
const unsigned long LONG_PRESS_TIME = 1000; // 1 second for long press
const unsigned long BRIGHTNESS_REPEAT_TIME = 500; // 500ms for brightness repeat

// Connection and battery status
bool isConnected = false;
unsigned long lastBatteryCheck = 0;
const unsigned long BATTERY_CHECK_INTERVAL = 5000; // Check battery every 5 seconds
int batteryLevel = -1; // -1 means no battery detected

// Brightness control
int brightness = 100; // Default brightness (100%)
const int MIN_BRIGHTNESS = 10; // Minimum brightness (10%)
const int MAX_BRIGHTNESS = 100; // Maximum brightness (100%)
const int BRIGHTNESS_STEP = 10; // Brightness change step (10%)
unsigned long lastBrightnessChange = 0;
unsigned long brightnessDisplayTime = 0;
bool brightnessChanged = false;
bool showingBrightness = false;

// Settings menu
#define SETTINGS_ENTRY_TIME 2000  // 2 seconds to enter settings
bool inSettingsMenu = false;
bool inSubMenu = false;
int settingsMenuIndex = 0;
int subMenuIndex = 0;
unsigned long bothButtonsPressTime = 0;
bool bothButtonsPressed = false;

// Scrolling variables
int settingsScrollOffset = 0;
int subMenuScrollOffset = 0;


// Page state management
enum Page {
  PAGE_MAIN_MENU,
  PAGE_INFO,
  PAGE_MESSAGES,
  PAGE_SETTINGS
};
Page currentPage = PAGE_MAIN_MENU;

// Main Menu
const char* mainMenuItems[] = {"Messages", "Settings", "Info"};
const int NUM_MAIN_MENU_ITEMS = 3;
int mainMenuSelection = 0;

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






// EEPROM addresses
const int EEPROM_SIZE = 2;  // Increased for new settings
const int BRIGHTNESS_ADDR = 0;
const int STANDBY_ADDR = 1;  // Store index of standby option

// Screen names
const char* screenNames[] = {"Messages", "No Messages", "Cleared", "BLE Ready"};
String currentScreenName = "BLE Ready";

// Sleep state
bool isAsleep = false;

// ============================================================================ 
// FUNCTION DECLARATIONS
// ============================================================================ 
void addMessageToHistory(String message);
void displayCurrentMessage();
bool doesTextFit(String message, const GFXfont* font, int maxWidth, int maxHeight);
void handleButtons();
void clearAllMessages();
void updateHeader();
void drawConnectionStatus();
void drawBatteryStatus();
int readBatteryLevel();
void setConnected(bool connected);
void setScreenName(String name);
void clearContentArea();
void setBrightness(int level);
void adjustBrightness(int delta);
void showBrightnessChange();
void hideBrightnessChange();
void checkAutoClear();
void saveBrightness();
void loadBrightness();
void checkSleep();
void enterDeepSleep();
void wakeFromSleep();
void setupBLE();
void initializeDisplay();
void drawMainMenu();
void drawInfoPage();

// Settings menu functions
void handleSettingsMenu();
void enterSettingsMenu();
void exitSettingsMenu();
void drawSettingsMenu();
void drawSubMenu();
void handleSettingsNavigation();
void handleSubMenuNavigation();
void saveStandbySetting();
void loadStandbySetting();
void updateSettingsScroll();
void updateSubMenuScroll();

// ============================================================================ 
// BLE CALLBACK CLASSES
// ============================================================================ 
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *characteristic) {
        String value = characteristic->getValue().c_str();
        
        if (value.length() > 0) {
            Serial.print("Received: '");
            Serial.print(value);
            Serial.println("'");
            
            // Update activity time on BLE data
            lastActivityTime = millis();
            
            // Add all data to buffer
            messageBuffer += value;
            lastDataTime = millis();
            
            // Check for newlines in the entire buffer (not just new data)
            int newlinePos;
            while ((newlinePos = messageBuffer.indexOf('\n')) >= 0) {
                // Extract message up to newline
                String completeMessage = messageBuffer.substring(0, newlinePos);
                completeMessage.trim();
                
                if (completeMessage.length() > 0) {
                    addMessageToHistory(completeMessage);
                }
                
                // Remove processed part from buffer
                messageBuffer = messageBuffer.substring(newlinePos + 1);
            }
        }
    }
};

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected");
        setConnected(true);
        messageBuffer = "";
        lastDataTime = 0;
        lastActivityTime = millis(); // Update activity on connection
    }

    void onDisconnect(BLEServer* pServer) {
        Serial.println("Client disconnected");
        setConnected(false);
        // Process any remaining data in buffer
        if (messageBuffer.length() > 0) {
            messageBuffer.trim();
            if (messageBuffer.length() > 0) {
                addMessageToHistory(messageBuffer);
            }
            messageBuffer = "";
        }
        delay(500);
        BLEDevice::startAdvertising();  
    }
};

// ============================================================================ 
// DISPLAY INITIALIZATION
// ============================================================================ 
void initializeDisplay() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    
    // Apply brightness immediately
    setBrightness(brightness);
}

// ============================================================================ 
// SLEEP FUNCTIONS
// ============================================================================ 
void checkSleep() {
    if (!isAsleep && (millis() - lastActivityTime) > SLEEP_TIMEOUT) {
        Serial.println(">>> ENTERING DEEP SLEEP - INACTIVITY");
        enterDeepSleep();
    }
}

void enterDeepSleep() {
    // Turn off display backlight
    digitalWrite(4, LOW);
    
    // Stop BLE to save power
    BLEDevice::deinit(true);
    
    Serial.println("Entering deep sleep... Press any button to wake");
    Serial.flush(); // Ensure all serial data is sent before sleep
    
    // Configure wake-up sources
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, 0); // Wake on BUTTON_1 (LOW)
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_2)), ESP_EXT1_WAKEUP_ALL_LOW); // Wake on BUTTON_2
    
    isAsleep = true;
    
    // Enter deep sleep
    esp_deep_sleep_start();
}

void wakeFromSleep() {
    // This function runs after waking from deep sleep
    Serial.println("Waking from deep sleep...");

    // Load settings from EEPROM first, as they are lost during deep sleep
    loadBrightness();
    loadStandbySetting();
    
    // Reinitialize display first
    initializeDisplay();
    
    // Reinitialize other components
    setupBLE();

    // Re-initialize button pins after waking from sleep
    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    
    // Reset activity timer
    lastActivityTime = millis();
    isAsleep = false;
    
    // Show the main screen
    currentPage = PAGE_MAIN_MENU;
    drawMainMenu();
    
    Serial.println("Woke from deep sleep - Ready for messages...");
}

void setupBLE() {
    // Initialize BLE
    BLEDevice::init("TTGO-BLE-Display");

    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_WRITE_NR
    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(true);
    BLEDevice::startAdvertising();
}

// ============================================================================ 
// BRIGHTNESS CONTROL FUNCTIONS
// ============================================================================ 
void loadBrightness() {
    EEPROM.begin(EEPROM_SIZE);
    int savedBrightness = EEPROM.read(BRIGHTNESS_ADDR);
    
    // Validate saved brightness value
    if (savedBrightness >= MIN_BRIGHTNESS && savedBrightness <= MAX_BRIGHTNESS) {
        brightness = savedBrightness;
        Serial.print("Loaded brightness from EEPROM: ");
        Serial.println(brightness);
    } else {
        // Invalid value, use default
        brightness = 100;
        saveBrightness(); // Save the default value
        Serial.println("Using default brightness: 100%");
    }
    EEPROM.end();
    
    // Apply the loaded brightness immediately
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
    
    // TTGO T-Display uses PWM on pin 4 for backlight control
    // Map brightness percentage to PWM value (0-255)
    int pwmValue = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 25, 255);
    analogWrite(4, pwmValue);
    
    // Only save if brightness actually changed
    if (oldBrightness != brightness) {
        brightnessChanged = true;
    }
    
    Serial.print("Brightness set to: ");
    Serial.print(brightness);
    Serial.println("%");
}

void adjustBrightness(int delta) {
    int newBrightness = brightness + delta;
    setBrightness(newBrightness);
    showBrightnessChange();
}

void showBrightnessChange() {
    // Temporarily show brightness level
    String brightnessText = "Brightness: " + String(brightness) + "%" ;
    
    // Save current content area
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_12);
    int textWidth = tft.textWidth(brightnessText.c_str());
    int x = (tft.width() - textWidth) / 2;
    int y = tft.height() / 2;
    
    // Draw brightness indicator
    tft.fillRect(0, y - 25, tft.width(), 50, TFT_BLACK);
    tft.setCursor(x, y);
    tft.print(brightnessText);
    
    showingBrightness = true;
    brightnessDisplayTime = millis();
}

void hideBrightnessChange() {
    if (showingBrightness) {
        showingBrightness = false;
        // Redisplay current content
        displayCurrentMessage();
    }
}

// ============================================================================ 
// STANDBY SETTINGS FUNCTIONS
// ============================================================================ 
void loadStandbySetting() {
    EEPROM.begin(EEPROM_SIZE);
    int savedStandbyIndex = EEPROM.read(STANDBY_ADDR);
    
    // Validate saved standby index
    if (savedStandbyIndex >= 0 && savedStandbyIndex < NUM_STANDBY_OPTIONS) {
        SLEEP_TIMEOUT = standbyValues[savedStandbyIndex];
        Serial.print("Loaded standby time from EEPROM: ");
        Serial.println(SLEEP_TIMEOUT);
    } else {
        // Invalid value, use default (15 minutes)
        SLEEP_TIMEOUT = 900000;
        saveStandbySetting(); // Save the default value
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
// SETTINGS MENU FUNCTIONS
// ============================================================================ 


void enterSettingsMenu() {
    inSettingsMenu = true;
    settingsMenuIndex = 0; // Start with Brightness selected
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
    
    // On exit, go back to the main menu
    currentPage = PAGE_MAIN_MENU;
    drawMainMenu();
}

void updateSettingsScroll() {
    if (settingsMenuIndex < settingsScrollOffset) {
        settingsScrollOffset = settingsMenuIndex;
    } else if (settingsMenuIndex >= settingsScrollOffset + MAIN_MENU_VISIBLE_ITEMS) {
        settingsScrollOffset = settingsMenuIndex - MAIN_MENU_VISIBLE_ITEMS + 1;
    }
}

void updateSubMenuScroll() {
    if (subMenuIndex < subMenuScrollOffset) {
        subMenuScrollOffset = subMenuIndex;
    } else if (subMenuIndex >= subMenuScrollOffset + SUB_MENU_VISIBLE_ITEMS) {
        subMenuScrollOffset = subMenuIndex - SUB_MENU_VISIBLE_ITEMS + 1;
    }
}

void drawSettingsMenu() {
    clearContentArea();
    tft.setRotation(1);
    
    // Draw navigation hints
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    String nextHint = "Sel->";
    String selectHint = "Nxt->";
    int nextWidth = tft.textWidth(nextHint.c_str());
    int selectWidth = tft.textWidth(selectHint.c_str());
    
    tft.setCursor(tft.width() - nextWidth - 5, HEADER_HEIGHT + 25);
    tft.print(nextHint);
    tft.setCursor(tft.width() - selectWidth - 5, tft.height() - 10);
    tft.print(selectHint);
    
    // Update scroll position
    updateSettingsScroll();
    
    // Draw settings items at the top of content area - adjusted spacing
    int startY = CONTENT_START_Y + 15;  // Moved up to just below header
    int lineHeight = 18;              // Reduced line height to fit 5 items
    
    for (int i = settingsScrollOffset; i < min(NUM_SETTINGS_ITEMS, settingsScrollOffset + MAIN_MENU_VISIBLE_ITEMS); i++) {
        int y = startY + ((i - settingsScrollOffset) * lineHeight);
        
        tft.setFreeFont(FONT_SANS_9);
        
        if (i == settingsMenuIndex) {
            // Highlight only the text (not the whole line)
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
            int textWidth = tft.textWidth(settingsItems[i]);
            tft.fillRect(10, y - 12, textWidth + 4, 16, TFT_WHITE); // Reduced height
            tft.setCursor(12, y);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setCursor(10, y);
        }
        
        tft.print(settingsItems[i]);
    }
}

void drawSubMenu() {
    clearContentArea();
    tft.setRotation(1);
    
    // Draw submenu title
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_BOLD_12);
    tft.setCursor(5, HEADER_HEIGHT + 25);
    tft.print(settingsItems[settingsMenuIndex]);
    
    // Draw navigation hints
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    String nextHint = "Sel->";
    String selectHint = "Nxt->";
    int nextWidth = tft.textWidth(nextHint.c_str());
    int selectWidth = tft.textWidth(selectHint.c_str());
    
    tft.setCursor(tft.width() - nextWidth - 5, HEADER_HEIGHT + 25);
    tft.print(nextHint);
    tft.setCursor(tft.width() - selectWidth - 5, tft.height() - 10);
    tft.print(selectHint);
    
    // Update scroll position
    updateSubMenuScroll();
    
    // Draw submenu items with adjusted spacing
    int startY = HEADER_HEIGHT + 50;
    int lineHeight = 20; // Keep original spacing for sub-menus
    
    if (settingsMenuIndex == 0) { // Brightness
        for (int i = subMenuScrollOffset; i < min(NUM_BRIGHTNESS_OPTIONS, subMenuScrollOffset + SUB_MENU_VISIBLE_ITEMS); i++) {
            int y = startY + ((i - subMenuScrollOffset) * lineHeight);
            
            tft.setFreeFont(FONT_SANS_9);
            
            if (i == subMenuIndex) {
                // Highlight only the text (not the whole line)
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
                int textWidth = tft.textWidth(brightnessOptions[i]);
                tft.fillRect(10, y - 12, textWidth + 4, 18, TFT_WHITE);
                tft.setCursor(12, y);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setCursor(10, y);
            }
            
            tft.print(brightnessOptions[i]);
        }
    } else if (settingsMenuIndex == 1) { // Auto Standby
        for (int i = subMenuScrollOffset; i < min(NUM_STANDBY_OPTIONS, subMenuScrollOffset + SUB_MENU_VISIBLE_ITEMS); i++) {
            int y = startY + ((i - subMenuScrollOffset) * lineHeight);
            
            tft.setFreeFont(FONT_SANS_9);
            
            if (i == subMenuIndex) {
                // Highlight only the text (not the whole line)
                tft.setTextColor(TFT_BLACK, TFT_WHITE);
                int textWidth = tft.textWidth(standbyOptions[i]);
                tft.fillRect(10, y - 12, textWidth + 4, 18, TFT_WHITE);
                tft.setCursor(12, y);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
                tft.setCursor(10, y);
            }
            
            tft.print(standbyOptions[i]);
        }
    }
}

void handleSettingsNavigation() {
    int button1State = digitalRead(BUTTON_1);
    int button2State = digitalRead(BUTTON_2);
    
    // Button 1 - Next item (reversed)
    if (button1State == LOW && lastButton1State == HIGH) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {
            settingsMenuIndex = (settingsMenuIndex + 1) % NUM_SETTINGS_ITEMS;
            drawSettingsMenu();
            lastDebounceTime1 = millis();
        }
    }
    
    // Button 2 - Select item (reversed)
    if (button2State == LOW && lastButton2State == HIGH) {
        if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
            if (settingsMenuIndex == NUM_SETTINGS_ITEMS - 1) { // Exit
                exitSettingsMenu();
            } else {
                inSubMenu = true;
                subMenuIndex = 0;
                subMenuScrollOffset = 0;
                
                // Load current setting for this submenu
                if (settingsMenuIndex == 0) { // Brightness
                    subMenuIndex = (brightness - MIN_BRIGHTNESS) / BRIGHTNESS_STEP;
                } else if (settingsMenuIndex == 1) { // Auto Standby
                    EEPROM.begin(EEPROM_SIZE);
                    subMenuIndex = EEPROM.read(STANDBY_ADDR);
                    if (subMenuIndex < 0 || subMenuIndex >= NUM_STANDBY_OPTIONS) {
                        subMenuIndex = 3; // Default to 15 minutes
                    }
                    EEPROM.end();
                }
                
                drawSubMenu();
            }
            lastDebounceTime2 = millis();
        }
    }
    
    lastButton1State = button1State;
    lastButton2State = button2State;
}

void handleSubMenuNavigation() {
    int button1State = digitalRead(BUTTON_1);
    int button2State = digitalRead(BUTTON_2);
    
    // Button 1 - Cycle through options (reversed)
    if (button1State == LOW && lastButton1State == HIGH) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {
            if (settingsMenuIndex == 0) { // Brightness
                subMenuIndex = (subMenuIndex + 1) % NUM_BRIGHTNESS_OPTIONS;
                // Apply brightness immediately
                int newBrightness = MIN_BRIGHTNESS + (subMenuIndex * BRIGHTNESS_STEP);
                setBrightness(newBrightness);
                brightnessChanged = true;
            } else if (settingsMenuIndex == 1) { // Auto Standby
                subMenuIndex = (subMenuIndex + 1) % NUM_STANDBY_OPTIONS;
                saveStandbySetting();
            }
            drawSubMenu();
            lastDebounceTime1 = millis();
        }
    }
    
    // Button 2 - Go back to main settings (reversed)
    if (button2State == LOW && lastButton2State == HIGH) {
        if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
            inSubMenu = false;
            // Save brightness setting on exit from sub-menu
            if (settingsMenuIndex == 0 && brightnessChanged) {
                saveBrightness();
            }
            drawSettingsMenu();
            lastDebounceTime2 = millis();
        }
    }
    
    lastButton1State = button1State;
    lastButton2State = button2State;
}

void handleSettingsMenu() {
    if (inSubMenu) {
        handleSubMenuNavigation();
    } else {
        handleSettingsNavigation();
    }
}

// ============================================================================ 
// CONNECTION AND BATTERY FUNCTIONS
// ============================================================================ 
void setConnected(bool connected) {
    if (isConnected != connected) {
        isConnected = connected;
        updateHeader();
    }
}

void setScreenName(String name) {
    if (currentScreenName != name) {
        currentScreenName = name;
        updateHeader();
    }
}

int readBatteryLevel() {
    // Read battery voltage from ADC pin
    int rawValue = analogRead(BATTERY_PIN);
    
    // Convert ADC reading to voltage (assuming 3.3V reference and voltage divider)
    // TTGO T-Display typically uses a 100k/100k voltage divider, so actual voltage = reading * 2 * 3.3 / 4095
    float voltage = rawValue * 2 * 3.3 / 4095.0;
    
    // Simple battery level estimation (adjust these values based on your battery)
    if (voltage < 3.5) return 0;  // Empty
    if (voltage < 3.7) return 25; // Low
    if (voltage < 3.9) return 50; // Medium
    if (voltage < 4.1) return 75; // High
    return 100; // Full
}

void updateHeader() {
    tft.setRotation(1);
    
    // Clear header area
    tft.fillRect(0, 0, tft.width(), HEADER_HEIGHT, TFT_BLACK);
    
    // Draw connection status (left)
    drawConnectionStatus();
    
    // Draw screen name (center) - use bold font for all headers
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_BOLD_12);
    int textWidth = tft.textWidth(currentScreenName.c_str());
    int centerX = (tft.width() - textWidth) / 2;
    tft.setCursor(centerX, 18); // Vertically centered in header
    tft.print(currentScreenName);
    
    // Draw battery status (right)
    drawBatteryStatus();
    
    // Draw separator line below header
    tft.drawLine(0, HEADER_HEIGHT, tft.width(), HEADER_HEIGHT, TFT_DARKGREY);
}

void drawConnectionStatus() {
    int size = 12; // 1.5x bigger (was 8)
    int x = 5;
    int y = (HEADER_HEIGHT - size) / 2 - 1; // Center vertically in header
    
    if (isConnected) {
        tft.fillRect(x, y, size, size, TFT_GREEN);
    } else {
        tft.fillRect(x, y, size, size, TFT_RED);
    }
}

void drawBatteryStatus() {
    int batteryWidth = 20;
    int batteryHeight = 10;
    int batteryX = tft.width() - batteryWidth - 5; // As far right as possible
    int batteryY = (HEADER_HEIGHT - batteryHeight) / 2; // Center vertically in header
    
    // Draw battery outline
    tft.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, TFT_WHITE);
    tft.fillRect(batteryX + batteryWidth, batteryY + 2, 2, batteryHeight - 4, TFT_WHITE); // battery tip
    
    if (batteryLevel >= 0) {
        // Calculate fill width based on battery level
        int fillWidth = (batteryWidth - 2) * batteryLevel / 100;
        
        // Choose color based on battery level
        uint16_t fillColor;
        if (batteryLevel < 20) fillColor = TFT_RED;
        else if (batteryLevel < 50) fillColor = TFT_YELLOW;
        else fillColor = TFT_GREEN;
        
        // Draw battery level
        if (fillWidth > 0) {
            tft.fillRect(batteryX + 1, batteryY + 1, fillWidth, batteryHeight - 2, fillColor);
        }
        
        // Show percentage using built-in font (much smaller)
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextFont(1);  // Smallest built-in font
        tft.setTextSize(1);  // Size 1 is the smallest
        
        String batteryText = String(batteryLevel) + "%";
        int textWidth = tft.textWidth(batteryText.c_str());
        int textX = batteryX - textWidth - 6; // Space between text and battery
        
        // Calculate vertical center for built-in font
        int fontHeight = 8; // Built-in font 1 size 1 is 8 pixels tall
        int textY = (HEADER_HEIGHT - fontHeight) / 2 + 1; // +1 to account for baseline
        
        tft.setCursor(textX, textY);
        tft.print(batteryText);
        
    } else {
        // No battery detected - show X using built-in font
        tft.setTextColor(TFT_RED, TFT_BLACK);
        tft.setTextFont(1);
        tft.setTextSize(1);
        
        String noBatteryText = "X";
        int textWidth = tft.textWidth(noBatteryText.c_str());
        int textX = batteryX - textWidth - 6;
        
        // Calculate vertical center for built-in font
        int fontHeight = 8; // Built-in font 1 size 1 is 8 pixels tall
        int textY = (HEADER_HEIGHT - fontHeight) / 2 + 1; // +1 to account for baseline
        
        tft.setCursor(textX, textY);
        tft.print(noBatteryText);
    }
    
    // Remember to set back to Free Font if you do other text drawing after this
    // tft.setFreeFont(FONT_SANS_9);
}

void clearContentArea() {
    // Clear only the content area below the header
    tft.fillRect(0, HEADER_HEIGHT + 1, tft.width(), tft.height() - HEADER_HEIGHT - 1, TFT_BLACK);
}

void checkAutoClear() {
    // Check if we should clear old messages when new data arrives
    if (totalMessages > 0 && (millis() - lastMessageReceivedTime) > CLEAR_TIMEOUT) {
        Serial.println(">>> AUTO-CLEARING OLD MESSAGES - NEW MESSAGES AFTER TIMEOUT");
        for (int i = 0; i < MAX_MESSAGES; i++) {
            messageHistory[i] = "";
        }
        totalMessages = 0;
        displayMessageIndex = -1;
    }
}

// ============================================================================ 
// MESSAGE HISTORY MANAGEMENT
// ============================================================================ 
void addMessageToHistory(String message) {
    message.trim();
    
    if (message.length() == 0) {
        return;
    }
    
    Serial.print("Adding to history: '");
    Serial.print(message);
    Serial.println("'");
    
    // Update activity time
    lastActivityTime = millis();
    
    // Check if we should clear old messages (5 seconds since last message)
    checkAutoClear();
    
    // Update the last message received time
    lastMessageReceivedTime = millis();
    
    // Shift existing messages if we've reached max capacity
    if (totalMessages >= MAX_MESSAGES) {
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            messageHistory[i] = messageHistory[i + 1];
        }
        totalMessages = MAX_MESSAGES - 1;
    }
    
    // Add new message
    messageHistory[totalMessages] = message;
    
    // Always display the FIRST message when new ones arrive
    if (totalMessages == 0) {
        displayMessageIndex = 0;
    }
    
    totalMessages++;
    
    setScreenName("Messages");
    displayCurrentMessage();
}

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
// DISPLAY FUNCTIONS
// ============================================================================ 
void drawMainMenu() {
    clearContentArea();
    setScreenName("Main"); // This will call updateHeader

    // Draw "BLE Ready!" as a sub-header
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_BOLD_12);
    tft.setCursor(5, HEADER_HEIGHT + 25);
    tft.print("BLE Ready!");

    // Draw navigation hints
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    String selHint = "Sel->";
    String nxtHint = "Nxt->";
    int selWidth = tft.textWidth(selHint.c_str());
    int nxtWidth = tft.textWidth(nxtHint.c_str());
    
    // Button 2 is select (top right)
    tft.setCursor(tft.width() - selWidth - 5, HEADER_HEIGHT + 25);
    tft.print(selHint);
    // Button 1 is next (bottom right)
    tft.setCursor(tft.width() - nxtWidth - 5, tft.height() - 10);
    tft.print(nxtHint);

    // Draw menu items
    int startY = HEADER_HEIGHT + 50; // Move menu items down
    int lineHeight = 22;

    for (int i = 0; i < NUM_MAIN_MENU_ITEMS; i++) {
        int y = startY + (i * lineHeight);
        tft.setFreeFont(FONT_SANS_12);
        
        if (i == mainMenuSelection) {
            tft.setTextColor(TFT_BLACK, TFT_WHITE);
            int textWidth = tft.textWidth(mainMenuItems[i]);
            tft.fillRect(10, y - 16, textWidth + 8, 20, TFT_WHITE);
            tft.setCursor(14, y);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setCursor(10, y);
        }
        tft.print(mainMenuItems[i]);
    }
}

void drawInfoPage() {
    clearContentArea();
    setScreenName("Info");

    // Navigation hints
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    String exitHint = "Exit->";
    String nxtHint = "Nxt->"; // Per request, both buttons will exit
    int exitWidth = tft.textWidth(exitHint.c_str());
    int nxtWidth = tft.textWidth(nxtHint.c_str());

    tft.setCursor(tft.width() - exitWidth - 5, HEADER_HEIGHT + 25);
    tft.print(exitHint);
    tft.setCursor(tft.width() - nxtWidth - 5, tft.height() - 10);
    tft.print(nxtHint);

    // Info content
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    int startY = CONTENT_START_Y + 20;
    int lineHeight = 18;
    tft.setCursor(10, startY);
    tft.println("For use with Toxic+");
    tft.setCursor(10, startY + lineHeight);
    tft.println("By Ian Pidgeon");
    tft.setCursor(10, startY + 2 * lineHeight);
    tft.println("Created by Rob Testa");
    tft.setCursor(10, startY + 3 * lineHeight);
    tft.println("November 2025");
}

void displayCurrentMessage() {
    clearContentArea();
    tft.setRotation(1);
    
    currentPage = PAGE_MESSAGES;
    setScreenName("Messages");

    // Define positions and dimensions
    int topY = HEADER_HEIGHT + 16; // Pushed up to be right below the header
    int bottomY = tft.height() - 8; // Pushed down to the bottom of the display
    int contentStartY = topY + 28; // Messages start right under the top nav line

    // Draw navigation hints
    tft.setTextColor(TFT_CYAN, TFT_BLACK); // Changed to CYAN to match other nav hints
    tft.setFreeFont(FONT_SANS_9);

    // Top-left: "Msg x/x"
    String msgCountText = "Msg " + (totalMessages > 0 ? String(displayMessageIndex + 1) : "0") + "/" + String(totalMessages);
    tft.setCursor(5, topY);
    tft.print(msgCountText);

    // Top-right: "Prev->" or "Exit->" (Button 2)
    String topRightText = (totalMessages > 0) ? "Prev->" : "Exit->";
    int topRightWidth = tft.textWidth(topRightText.c_str());
    tft.setCursor(tft.width() - topRightWidth - 5, topY);
    tft.print(topRightText);

    // Bottom-left: "Hold Bttn: Main"
    tft.setCursor(5, bottomY);
    tft.print("Hold Bttn: Main");

    // Bottom-right: "Next->" (Button 1)
    String bottomRightText = "Next->";
    int bottomRightWidth = tft.textWidth(bottomRightText.c_str());
    tft.setCursor(tft.width() - bottomRightWidth - 5, bottomY);
    tft.print(bottomRightText);


    if (totalMessages == 0) {
        // Display "Waiting for messages..."
        tft.setFreeFont(FONT_SANS_9);
        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.setCursor(5, contentStartY);
        tft.println("Waiting for messages...");
    } else {
        if (displayMessageIndex < 0) displayMessageIndex = 0;
        if (displayMessageIndex >= totalMessages) displayMessageIndex = totalMessages - 1;

        String message = messageHistory[displayMessageIndex];
        Serial.print("Displaying message ");
        Serial.print(displayMessageIndex + 1);
        Serial.print("/");
        Serial.print(totalMessages);
        Serial.print(": '");
        Serial.print(message);
        Serial.println("'");
        
        // Determine the best font size
        const GFXfont* font = FONT_SANS_BOLD_12;
        if (!doesTextFit(message, font, tft.width() - 10, tft.height() - contentStartY - 20)) {
            font = FONT_SANS_12;
            if (!doesTextFit(message, font, tft.width() - 10, tft.height() - contentStartY - 20)) {
                font = FONT_SANS_9;
            }
        }

        tft.setFreeFont(font);
        tft.setTextColor(TFT_GREEN, TFT_BLACK); // Changed message text color to GREEN
        tft.setCursor(5, contentStartY);

        // Simple word wrap implementation
        String currentLine = "";
        String word = "";
        for (int i = 0; i < message.length(); i++) {
            char c = message[i];
            if (c == ' ' || c == '\n' || i == message.length() - 1) {
                if (i == message.length() - 1 && c != ' ' && c != '\n') {
                    word += c;
                }

                String testLine = currentLine + (currentLine == "" ? word : " " + word);
                if (tft.textWidth(testLine.c_str()) > tft.width() - 10 && currentLine != "") {
                    tft.println(currentLine);
                    currentLine = word;
                } else {
                    currentLine = testLine;
                }
                word = "";
                if (c == '\n') {
                    tft.println(currentLine);
                    currentLine = "";
                }
            } else {
                word += c;
            }
        }
        if (currentLine != "") {
            tft.println(currentLine);
        }
    }
}



bool doesTextFit(String message, const GFXfont* font, int maxWidth, int maxHeight) {
    tft.setFreeFont(font);
    int lineHeight = tft.fontHeight();
    int currentY = 0;
    String currentLine = "";
    String word = "";
    
    for (int i = 0; i < message.length(); i++) {
        char c = message[i];
        
        if (c == ' ' || c == '\n' || i == message.length() - 1) {
            if (i == message.length() - 1 && c != ' ' && c != '\n') {
                word += c;
            }
            
            String testLine = currentLine + (currentLine == "" ? word : " " + word);
            int textWidth = tft.textWidth(testLine.c_str());
            
            if (textWidth > maxWidth && currentLine != "") {
                currentLine = word;
                currentY += lineHeight;
                if (currentY + lineHeight > maxHeight) {
                    return false;
                }
            } else {
                currentLine = testLine;
            }
            
            word = "";
        } else {
            word += c;
        }
    }
    
    if (currentLine != "") {
        currentY += lineHeight;
    }
    
    return currentY <= maxHeight;
}

// ============================================================================ 
// BUTTON HANDLING WITH SETTINGS (REMOVED BRIGHTNESS LONG PRESS)
// ============================================================================ 
void handleMainMenuButtons() {
    int button1State = digitalRead(BUTTON_1);
    int button2State = digitalRead(BUTTON_2);

    // Button 1 (Nxt->) - Cycle through menu items
    if (button1State == LOW && lastButton1State == HIGH) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {
            mainMenuSelection = (mainMenuSelection + 1) % NUM_MAIN_MENU_ITEMS;
            drawMainMenu();
            lastDebounceTime1 = millis();
        }
    }

    // Button 2 (Sel->) - Select a menu item
    if (button2State == LOW && lastButton2State == HIGH) {
        if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
            if (mainMenuSelection == 0) { // Messages
                currentPage = PAGE_MESSAGES;
                displayCurrentMessage();
            } else if (mainMenuSelection == 1) { // Settings
                currentPage = PAGE_SETTINGS;
                enterSettingsMenu();
            } else if (mainMenuSelection == 2) { // Info
                currentPage = PAGE_INFO;
                drawInfoPage();
            }
            lastDebounceTime2 = millis();
        }
    }

    lastButton1State = button1State;
    lastButton2State = button2State;
}

void handleInfoPageButtons() {
    int button1State = digitalRead(BUTTON_1);
    int button2State = digitalRead(BUTTON_2);

    // Any button press returns to the main menu
    if ((button1State == LOW && lastButton1State == HIGH) || (button2State == LOW && lastButton2State == HIGH)) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY && (millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
            currentPage = PAGE_MAIN_MENU;
            mainMenuSelection = 0; // Reset selection
            drawMainMenu();
            lastDebounceTime1 = millis();
            lastDebounceTime2 = millis();
        }
    }

    lastButton1State = button1State;
    lastButton2State = button2State;
}

void handleMessageButtons() {

    int button1State = digitalRead(BUTTON_1);

    int button2State = digitalRead(BUTTON_2);



    // Long press on either button to return to main menu

    if (button1State == LOW) {

        if (button1PressTime == 0) {

            button1PressTime = millis();

        } else if (!button1LongPress && (millis() - button1PressTime > LONG_PRESS_TIME)) {

            Serial.println(">>> BUTTON 1 LONG PRESS - MAIN MENU");

            currentPage = PAGE_MAIN_MENU;

            drawMainMenu();

            button1LongPress = true; // Prevent repeated trigger

            return; // Exit to avoid short press detection

        }

    } else {

        button1PressTime = 0;

        button1LongPress = false;

    }



    if (button2State == LOW) {

        if (button2PressTime == 0) {

            button2PressTime = millis();

        } else if (!button2LongPress && (millis() - button2PressTime > LONG_PRESS_TIME)) {

            Serial.println(">>> BUTTON 2 LONG PRESS - MAIN MENU");

            currentPage = PAGE_MAIN_MENU;

            drawMainMenu();

            button2LongPress = true; // Prevent repeated trigger

            return; // Exit to avoid short press detection

        }

    } else {

        button2PressTime = 0;

        button2LongPress = false;

    }



    // Short press logic (if no long press was detected)

    

    // Button 1 (Next->) - Next message (short press)

    if (button1State == LOW && lastButton1State == HIGH) {

        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {

            Serial.println(">>> BUTTON 1 PRESSED - NEXT MESSAGE");

            if (totalMessages > 0) {

                displayMessageIndex = (displayMessageIndex + 1) % totalMessages;

                displayCurrentMessage();

            } else {

                // If no messages, button press should exit to main menu

                currentPage = PAGE_MAIN_MENU;

                drawMainMenu();

            }

            lastDebounceTime1 = millis();

        }

    }

    

    // Button 2 (Prev->) - Previous message (short press)

    if (button2State == LOW && lastButton2State == HIGH) {

        if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {

            Serial.println(">>> BUTTON 2 PRESSED - PREVIOUS MESSAGE");

            if (totalMessages > 0) {

                displayMessageIndex--;

                if (displayMessageIndex < 0) displayMessageIndex = totalMessages - 1;

                displayCurrentMessage();

            } else {

                // If no messages, button press should exit to main menu

                currentPage = PAGE_MAIN_MENU;

                drawMainMenu();

            }

            lastDebounceTime2 = millis();

        }

    }



    lastButton1State = button1State;

    lastButton2State = button2State;

}


void handleButtons() {
    // Update activity time on any button activity
    if (digitalRead(BUTTON_1) == LOW || digitalRead(BUTTON_2) == LOW) {
        lastActivityTime = millis();
    }

    switch (currentPage) {
        case PAGE_MAIN_MENU:
            handleMainMenuButtons();
            break;
        case PAGE_INFO:
            handleInfoPageButtons();
            break;
        case PAGE_MESSAGES:
            handleMessageButtons();
            break;
        case PAGE_SETTINGS:
            handleSettingsMenu();
            break;
    }
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
