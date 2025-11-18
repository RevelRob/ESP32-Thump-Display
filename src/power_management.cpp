#include "power_management.h"
#include "settings.h"
#include "display.h"
#include "ble_handler.h"

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
    digitalWrite(4, LOW); // Turn off backlight
    BLEDevice::deinit(true);
    
    Serial.println("Entering deep sleep... Press any button to wake");
    Serial.flush();
    
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_1, 0);
    esp_sleep_enable_ext1_wakeup(((uint64_t)(((uint64_t)1) << BUTTON_2)), ESP_EXT1_WAKEUP_ALL_LOW);
    
    isAsleep = true;
    esp_deep_sleep_start();
}

void wakeFromSleep() {
    Serial.println("Waking from deep sleep...");

    loadBrightness();
    loadStandbySetting();
    loadMirrorSetting();
    
    initializeDisplay();
    setupBLE();

    pinMode(BUTTON_1, INPUT_PULLUP);
    pinMode(BUTTON_2, INPUT_PULLUP);
    
    lastActivityTime = millis();
    isAsleep = false;
    
    currentPage = PAGE_MAIN_MENU;
    drawMainMenu();
    
    Serial.println("Woke from deep sleep - Ready for messages...");
}

// ============================================================================
// BATTERY FUNCTIONS
// ============================================================================
int readBatteryLevel() {
    float voltage = analogRead(BATTERY_PIN) * 2 * 3.3 / 4095.0;
    if (voltage < 3.5) return 0;
    if (voltage < 3.7) return 25;
    if (voltage < 3.9) return 50;
    if (voltage < 4.1) return 75;
    return 100;
}