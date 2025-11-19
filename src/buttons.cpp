#include "buttons.h"
#include "display.h"
#include "settings.h"

// Button state variables are now encapsulated in this file
static int lastButton1State = HIGH;
static int lastButton2State = HIGH;
static unsigned long lastDebounceTime1 = 0;
static unsigned long lastDebounceTime2 = 0;
static unsigned long button1PressTime = 0;
static unsigned long button2PressTime = 0;
static bool button1LongPress = false;
static bool button2LongPress = false;

// ============================================================================
// MAIN BUTTON DISPATCHER
// ============================================================================
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
// PAGE-SPECIFIC BUTTON HANDLERS
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

    // Button 1 (Next->) - Next message (short press)
    if (button1State == LOW && lastButton1State == HIGH) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {
            Serial.println(">>> BUTTON 1 PRESSED - NEXT MESSAGE");
            if (totalMessages > 0) {
                displayMessageIndex = (displayMessageIndex + 1) % totalMessages;
                displayCurrentMessage();
            } else {
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
                currentPage = PAGE_MAIN_MENU;
                drawMainMenu();
            }
            lastDebounceTime2 = millis();
        }
    }

    lastButton1State = button1State;
    lastButton2State = button2State;
}

void handleSettingsNavigation() {
    int button1State = digitalRead(BUTTON_1);
    int button2State = digitalRead(BUTTON_2);
    
    if (button1State == LOW && lastButton1State == HIGH) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {
            settingsMenuIndex = (settingsMenuIndex + 1) % NUM_SETTINGS_ITEMS;
            drawSettingsMenu();
            lastDebounceTime1 = millis();
        }
    }
    
    if (button2State == LOW && lastButton2State == HIGH) {
        if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
            if (settingsMenuIndex == NUM_SETTINGS_ITEMS - 1) { // Exit
                exitSettingsMenu();
            } else {
                enterSubMenu();
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
    
    if (button1State == LOW && lastButton1State == HIGH) {
        if ((millis() - lastDebounceTime1) > DEBOUNCE_DELAY) {
            cycleSubMenuOption();
            drawSubMenu();
            lastDebounceTime1 = millis();
        }
    }
    
    if (button2State == LOW && lastButton2State == HIGH) {
        if ((millis() - lastDebounceTime2) > DEBOUNCE_DELAY) {
            exitSubMenu();
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