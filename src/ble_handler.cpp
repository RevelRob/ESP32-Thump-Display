#include "ble_handler.h"
#include "display.h"
#include <BLE2902.h>
#include <Arduino.h>

bool expectingCardCode = false;

// Forward declaration
void clearAllMessages();
String getCardName(String code);

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

bool processSmartTextMessage(String message) {
    message.trim();
    if (message.equalsIgnoreCase("#")) {
        clearAllMessages();
        return true;
    }
    if (message.equalsIgnoreCase("#REBOOT")) {
        ESP.restart();
        return true;
    }
    if (message.equalsIgnoreCase("#CARDS")) {
        expectingCardCode = true;
        return true;
    }
    return false;
}

// The original addMessageToHistory, but renamed and declared static
static void addSingleMessageToHistory(String message) {
    if (smartTextEnabled) {
        if (processSmartTextMessage(message)) {
            return; // Smart text command was processed, so don't add to history
        }
        if (expectingCardCode) {
            expectingCardCode = false; // Consume the expectation
            String cardName = getCardName(message);
            if (cardName.length() > 0) {
                message = cardName; // The message to be added is now the card name.
            } else {
                return; // Invalid card code, do nothing.
            }
        }
    }

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

// The new public addMessageToHistory that splits messages by newline
void addMessageToHistory(String message) {
    int lastPos = 0;
    for (int i = 0; i < message.length(); i++) {
        if (message.charAt(i) == '\n') {
            String line = message.substring(lastPos, i);
            line.trim();
            if (line.length() > 0) {
                addSingleMessageToHistory(line);
            }
            lastPos = i + 1;
        }
    }
    // Process the last part of the message (or the whole message if no newline)
    String remaining = message.substring(lastPos);
    remaining.trim();
    if (remaining.length() > 0) {
        addSingleMessageToHistory(remaining);
    }
}

void checkAutoClear() {
    if (totalMessages > 0 && (millis() - lastMessageReceivedTime) > CLEAR_TIMEOUT) {
        Serial.println(">>> AUTO-CLEARING OLD MESSAGES");
        for (int i = 0; i < MAX_MESSAGES; i++) messageHistory[i] = "";
        totalMessages = 0;
        displayMessageIndex = -1;
    }
}

String getCardName(String code) {
    code.trim();
    if (code.length() < 2 || code.length() > 3) return "";

    String rankStr, suitStr;
    String rankPart, suitPart;

    if (code.length() == 2) {
        rankPart = code.substring(0, 1);
        suitPart = code.substring(1, 2);
    } else { // length is 3
        rankPart = code.substring(0, 2);
        suitPart = code.substring(2, 3);
    }

    // Get Rank
    if (rankPart == "1") rankStr = "Ace";
    else if (rankPart == "2") rankStr = "2";
    else if (rankPart == "3") rankStr = "3";
    else if (rankPart == "4") rankStr = "4";
    else if (rankPart == "5") rankStr = "5";
    else if (rankPart == "6") rankStr = "6";
    else if (rankPart == "7") rankStr = "7";
    else if (rankPart == "8") rankStr = "8";
    else if (rankPart == "9") rankStr = "9";
    else if (rankPart == "10") rankStr = "10";
    else if (rankPart == "11") rankStr = "Jack";
    else if (rankPart == "12") rankStr = "Queen";
    else if (rankPart == "13") rankStr = "King";
    else return ""; // Invalid rank

    // Get Suit
    if (suitPart == "1") suitStr = "Spades";
    else if (suitPart == "2") suitStr = "Hearts";
    else if (suitPart == "3") suitStr = "Clubs";
    else if (suitPart == "4") suitStr = "Diamonds";
    else return ""; // Invalid suit

    return rankStr + " of " + suitStr;
}