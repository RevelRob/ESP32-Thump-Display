#include "ble_handler.h"
#include "display.h"
#include <BLE2902.h>

// These are defined in main.cpp but used here for message buffering
extern String messageBuffer;
extern unsigned long lastDataTime;
extern unsigned long lastMessageReceivedTime;

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
            
            lastActivityTime = millis();
            messageBuffer += value;
            lastDataTime = millis();
        }
    }
};

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        Serial.println("Client connected");
        setConnected(true);
        messageBuffer = "";
        lastDataTime = 0;
        lastActivityTime = millis();
    }

    void onDisconnect(BLEServer* pServer) {
        Serial.println("Client disconnected");
        setConnected(false);
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
// BLE & MESSAGE FUNCTIONS
// ============================================================================

void setupBLE() {
    BLEDevice::init("TTGO-BLE-Display");
    BLEServer *pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);

    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );

    pCharacteristic->setCallbacks(new MyCallbacks());
    pCharacteristic->addDescriptor(new BLE2902());
    pService->start();

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(true);
    BLEDevice::startAdvertising();
}

void setConnected(bool connected) {
    if (isConnected != connected) {
        isConnected = connected;
        updateHeader();
    }
}

void addMessageToHistory(String message) {
    message.trim();
    if (message.length() == 0) return;
    
    Serial.print("Adding to history: '");
    Serial.print(message);
    Serial.println("'");
    
    lastActivityTime = millis();
    checkAutoClear();
    lastMessageReceivedTime = millis();
    
    if (totalMessages >= MAX_MESSAGES) {
        for (int i = 0; i < MAX_MESSAGES - 1; i++) {
            messageHistory[i] = messageHistory[i + 1];
        }
        totalMessages = MAX_MESSAGES - 1;
    }
    
    messageHistory[totalMessages] = message;
    if (totalMessages == 0) displayMessageIndex = 0;
    totalMessages++;
    
    setScreenName("Messages");
    displayCurrentMessage();
}

void checkAutoClear() {
    if (totalMessages > 0 && (millis() - lastMessageReceivedTime) > CLEAR_TIMEOUT) {
        Serial.println(">>> AUTO-CLEARING OLD MESSAGES");
        for (int i = 0; i < MAX_MESSAGES; i++) messageHistory[i] = "";
        totalMessages = 0;
        displayMessageIndex = -1;
    }
}