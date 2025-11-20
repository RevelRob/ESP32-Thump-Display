#include "display.h"
#include "settings.h"
#include "DejaVuSans_Bold28pt7b.h"
#include "DejaVuSans_Bold36pt7b.h"
#include <vector>

static bool showingBrightness = false;
static bool brightnessChanged = false;
static unsigned long brightnessDisplayTime = 0;

// ============================================================================
// DISPLAY INITIALIZATION
// ============================================================================
void initializeDisplay() {
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);

    // Apply brightness immediately (brightness is loaded from EEPROM in main setup)
    int pwmValue = map(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS, 25, 255);
    analogWrite(4, pwmValue);
}

// ============================================================================
// HEADER & STATUS DRAWING FUNCTIONS
// ============================================================================

void setScreenName(String name) {
    if (currentScreenName != name) {
        currentScreenName = name;
        updateHeader();
    }
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
}

void clearContentArea() {
    // Clear only the content area below the header
    tft.fillRect(0, HEADER_HEIGHT + 1, tft.width(), tft.height() - HEADER_HEIGHT - 1, TFT_BLACK);
}

// ============================================================================
// PAGE DRAWING FUNCTIONS
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
            // Highlighted item is green
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
            // Non-highlighted items are white
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }
        tft.setCursor(10, y);
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

void pushSpriteMirrored(TFT_eSprite* sprite, int32_t x, int32_t y) {
    static uint16_t* mirroredBuffer = nullptr;
    static int32_t bufferWidth = 0;
    static int32_t bufferHeight = 0;

    uint16_t* originalBuffer = (uint16_t*)sprite->getPointer();
    int32_t w = sprite->width();
    int32_t h = sprite->height();

    // Check if buffer needs to be (re)allocated. This is to prevent memory
    // allocation on every frame of the scrolling animation.
    if (mirroredBuffer == nullptr || bufferWidth != w || bufferHeight != h) {
        if (mirroredBuffer) free(mirroredBuffer);
        mirroredBuffer = (uint16_t*)malloc(w * h * sizeof(uint16_t));
        if (!mirroredBuffer) {
            Serial.println("Failed to allocate memory for mirrored buffer");
            bufferWidth = 0;
            bufferHeight = 0;
            return;
        }
        bufferWidth = w;
        bufferHeight = h;
    }

    // Copy and mirror pixels from the original buffer to the static mirrored buffer.
    // The byte-swap operation that was here previously was causing the color to change
    // from green to red, and was unnecessary.
    for (int32_t j = 0; j < h; j++) {
        for (int32_t i = 0; i < w; i++) {
            // Get color from original and place it in the mirrored position
            mirroredBuffer[(w - 1 - i) + j * w] = originalBuffer[i + j * w];
        }
    }

    // Push the entire mirrored buffer to the screen at once
    tft.pushImage(x, y, w, h, mirroredBuffer);
}


void displayCard(String rank, String suit) {
    clearContentArea();
    setScreenName("Card");

    // Navigation hint
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    String backHint = "Back->";
    int backWidth = tft.textWidth(backHint.c_str());
    tft.setCursor(tft.width() - backWidth - 5, tft.height() - 10);
    tft.print(backHint);

    // Display card
    String cardText = rank + suit;
    tft.setFreeFont(FONT_SANS_BOLD_12); // Using a large bold font
    
    // Calculate text width to center it
    int textWidth = tft.textWidth(cardText.c_str());
    int x = (tft.width() - textWidth) / 2;
    int y = (tft.height() - HEADER_HEIGHT) / 2 + HEADER_HEIGHT; // Center vertically in content area

    tft.setCursor(x, y);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(cardText);
}

void drawSuitBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
    const int16_t bytesPerRow = (w + 7) / 8; // Correct stride calculation
    uint16_t *buffer = (uint16_t *)malloc(w * h * sizeof(uint16_t));
    if (!buffer) {
        Serial.println("Failed to allocate memory for suit bitmap buffer");
        return;
    }

    for (int16_t j = 0; j < h; j++) {
        for (int16_t i = 0; i < w; i++) {
            uint8_t byte = pgm_read_byte(&bitmap[j * bytesPerRow + i / 8]);
            bool pixelOn = byte & (0x80 >> (i % 8));
            if (pixelOn) {
                // Byte-swap the color for pushImage to prevent red from becoming blue.
                buffer[j * w + i] = (color >> 8) | (color << 8);
            } else {
                buffer[j * w + i] = TFT_BLACK;
            }
        }
    }

    tft.pushImage(x, y, w, h, buffer);
    free(buffer);
}


void drawCardSymbol(String rank, String suitChar) {
    clearContentArea();
    setScreenName("Card");
    
    // Navigation hint
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);
    String backHint = "Back->";
    int backWidth = tft.textWidth(backHint.c_str());
    tft.setCursor(tft.width() - backWidth - 5, tft.height() - 10);
    tft.print(backHint);
    
    // --- Display card rank and suit bitmap ---
    const GFXfont* rankFont = &DejaVuSans_Bold36pt7b;
    tft.setFreeFont(rankFont);
    
    int rankHeight = rankFont->yAdvance;
    int rankWidth = tft.textWidth(rank);
    int spacing = 8;
    
    const unsigned char* suit_bits = nullptr;
    int suit_width = 0;
    int suit_height = 0;
    uint16_t suitColor;
    
    if (suitChar == "H") {
        suit_bits = heart_bits; suit_width = heart_width; suit_height = heart_height;
        suitColor = TFT_RED;
    } else if (suitChar == "D") {
        suit_bits = diamond_bits; suit_width = diamond_width; suit_height = diamond_height;
        suitColor = TFT_RED;
    } else if (suitChar == "S") {
        suit_bits = spade_bits; suit_width = spade_width; suit_height = spade_height;
        suitColor = TFT_WHITE;
    } else if (suitChar == "C") {
        suit_bits = club_bits; suit_width = club_width; suit_height = club_height;
        suitColor = TFT_WHITE;
    }
    
    int totalWidth = rankWidth + spacing + suit_width;
    int startX = (tft.width() - totalWidth) / 2;

    // Find the max height for vertical alignment calculations
    int maxHeight = max(rankHeight, suit_height);

    // Calculate the top Y position for the combined element, then move it up by 6 pixels
    int startY = ((tft.height() - HEADER_HEIGHT - maxHeight) / 2) + HEADER_HEIGHT;
    
    // Draw Rank
    // The rank should always be white.
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setCursor(startX, startY + (maxHeight - rankHeight) / 2 + 58);
    tft.print(rank);
    
    // Draw Suit
    if (suit_bits) {
        drawSuitBitmap(startX + rankWidth + spacing, startY + (maxHeight - suit_height) / 2 - 8, suit_bits, suit_width, suit_height, suitColor);
    }
}

void displayCurrentMessage() {
    // This function is now an initializer for the message screen.
    // It sets up the static elements and determines if scrolling is needed.
    
    // First, check if the message is a card symbol message
    if (totalMessages > 0 && messageHistory[displayMessageIndex].startsWith("[CARD:")) {
        String message = messageHistory[displayMessageIndex];
        int rankEnd = message.indexOf(',');
        int suitEnd = message.indexOf(']');
        if (rankEnd != -1 && suitEnd != -1) {
            String rank = message.substring(6, rankEnd);
            String suit = message.substring(rankEnd + 1, suitEnd);
            drawCardSymbol(rank, suit);
            return;
        }
    }
    
    clearContentArea();
    tft.setRotation(1);

    currentPage = PAGE_MESSAGES;
    setScreenName("Messages");

    // Reset scrolling state for the new message
    messageScroll.isLong = false;
    messageScroll.offset = 0;
    messageScroll.state = 0; // 0 = paused at top
    messageScroll.lastTime = millis();

    // --- Draw static UI elements ---
    int topY = HEADER_HEIGHT + 16;
    int bottomY = tft.height() - 6;
    
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.setFreeFont(FONT_SANS_9);

    String msgCountText = "Msg " + (totalMessages > 0 ? String(displayMessageIndex + 1) : "0") + "/" + String(totalMessages);
    tft.setCursor(5, topY);
    tft.print(msgCountText);

    String topRightText = (totalMessages > 0) ? "Prev->" : "Exit->";
    int topRightWidth = tft.textWidth(topRightText.c_str());
    tft.setCursor(tft.width() - topRightWidth - 5, topY);
    tft.print(topRightText);

    tft.setCursor(5, bottomY);
    tft.print("Hold Bttn: Main");

    String bottomRightText = "Next->";
    int bottomRightWidth = tft.textWidth(bottomRightText.c_str());
    tft.setCursor(tft.width() - bottomRightWidth - 5, bottomY);
    tft.print(bottomRightText);

    // --- Calculate message height and determine if scrolling is needed ---
    if (totalMessages > 0) {
        int contentWidth = tft.width();
        int contentHeight = tft.height() - (topY + 8) - 20;
        String message = messageHistory[displayMessageIndex];

        // Find best font size that fits horizontally
        messageScroll.font = FONT_SANS_9; // Default to smallest
        if (calculateWrappedTextHeight(message, FONT_SANS_BOLD_12, contentWidth - 10) <= contentHeight) {
            messageScroll.font = FONT_SANS_BOLD_12;
        } else if (calculateWrappedTextHeight(message, FONT_SANS_12, contentWidth - 10) <= contentHeight) {
            messageScroll.font = FONT_SANS_12;
        }

        // Now get the full height with the chosen font
        messageScroll.totalHeight = calculateWrappedTextHeight(message, messageScroll.font, contentWidth - 10);

        if (messageScroll.totalHeight > contentHeight) {
            messageScroll.isLong = true;
        }
    }

    // Perform the initial draw of the message content
    drawMessageContent();
}

void drawMessageContent() {
    // This function draws the message content itself, using the current scrollOffset
    int topY = HEADER_HEIGHT + 16;
    int contentStartY = topY + 8;
    int contentWidth = tft.width();
    int contentHeight = tft.height() - contentStartY - 20;

    TFT_eSprite messageSprite = TFT_eSprite(&tft);
    messageSprite.createSprite(contentWidth, contentHeight);
    messageSprite.fillSprite(TFT_BLACK);
    messageSprite.setTextColor(TFT_GREEN, TFT_BLACK);

    if (totalMessages == 0) {
        messageSprite.setFreeFont(FONT_SANS_9);
        messageSprite.setCursor(5, messageSprite.fontHeight() - 8);
        messageSprite.println("Waiting for messages...");
    } else {
        if (displayMessageIndex < 0) displayMessageIndex = 0;
        if (displayMessageIndex >= totalMessages) displayMessageIndex = totalMessages - 1;

        String message = messageHistory[displayMessageIndex];
        
        messageSprite.setFreeFont(messageScroll.font);
        // Set cursor with scroll offset
        messageSprite.setCursor(5, messageSprite.fontHeight() - 8 - messageScroll.offset);

        // Simple word wrap implementation onto the sprite
        String currentLine = "";
        String word = "";
        for (int i = 0; i < message.length(); i++) {
            char c = message[i];
            if (c == ' ' || c == '\n' || i == message.length() - 1) {
                if (i == message.length() - 1 && c != ' ' && c != '\n') word += c;

                String testLine = currentLine + (currentLine == "" ? word : " " + word);
                if (messageSprite.textWidth(testLine.c_str()) > contentWidth - 10 && currentLine != "") {
                    messageSprite.println(currentLine);
                    currentLine = word;
                } else {
                    currentLine = testLine;
                }
                word = "";
                if (c == '\n') {
                    messageSprite.println(currentLine);
                    currentLine = "";
                }
            } else {
                word += c;
            }
        }
        if (currentLine != "") {
            messageSprite.println(currentLine);
        }
    }

    if (mirrorMessages) {
        pushSpriteMirrored(&messageSprite, 0, contentStartY);
    } else {
        messageSprite.pushSprite(0, contentStartY);
    }

    messageSprite.deleteSprite();
}

void updateDisplay() {
    if (currentPage != PAGE_MESSAGES || !messageScroll.isLong) {
        return; // Only scroll on the message page for long messages
    }

    unsigned long time = millis();

    // State 0: Paused at the top
    if (messageScroll.state == 0) {
        if (time - messageScroll.lastTime > SCROLL_PAUSE) {
            messageScroll.state = 1; // Start scrolling down
            messageScroll.lastTime = time;
        }
    }
    // State 1: Scrolling down
    else if (messageScroll.state == 1) {
        if (time - messageScroll.lastTime > SCROLL_DELAY) {
            messageScroll.offset++;
            messageScroll.lastTime = time;

            int contentHeight = tft.height() - (HEADER_HEIGHT + 16 + 8) - 20;
            if (messageScroll.offset >= messageScroll.totalHeight - contentHeight) {
                messageScroll.offset = messageScroll.totalHeight - contentHeight;
                messageScroll.state = 2; // Reached bottom, start pause
                messageScroll.lastTime = time;
            }
            drawMessageContent();
        }
    }
    // State 2: Paused at the bottom
    else if (messageScroll.state == 2) {
        if (time - messageScroll.lastTime > SCROLL_PAUSE) {
            messageScroll.offset = 0; // Reset to top
            messageScroll.state = 0; // Pause at top
            messageScroll.lastTime = time;
            drawMessageContent();
        }
    }
}

int calculateWrappedTextHeight(String message, const GFXfont* font, int maxWidth) {
    // This function uses a temporary sprite to measure text height without drawing to the screen
    TFT_eSprite tempSprite = TFT_eSprite(&tft);
    tempSprite.createSprite(1, 1); // Minimal sprite to access font properties
    tempSprite.setFreeFont(font);

    int lineHeight = tempSprite.fontHeight();
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
            int textWidth = tempSprite.textWidth(testLine.c_str());

            if (textWidth > maxWidth && currentLine != "") {
                currentY += lineHeight;
                currentLine = word;
            } else {
                currentLine = testLine;
            }

            word = "";
            if (c == '\n') {
                currentY += lineHeight;
                currentLine = "";
            }
        } else {
            word += c;
        }
    }

    if (currentLine != "") {
        currentY += lineHeight;
    }

    tempSprite.deleteSprite();
    return currentY;
}

// ============================================================================
// SETTINGS MENU DRAWING
// ============================================================================

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

    // New 3-phase scrolling logic
    const int VISIBLE_ITEMS = 5;
    const int PIN_ROW = VISIBLE_ITEMS / 2; // Middle row (index 2 for 5 items)

    if (settingsMenuIndex < PIN_ROW) {
        // Phase 1: Highlight moves down from top
        settingsScrollOffset = 0;
    } else if (settingsMenuIndex >= (NUM_SETTINGS_ITEMS - (VISIBLE_ITEMS - PIN_ROW))) {
        // Phase 3: Highlight moves down at the bottom
        settingsScrollOffset = NUM_SETTINGS_ITEMS - VISIBLE_ITEMS;
    } else {
        // Phase 2: Highlight is pinned to the middle, list scrolls
        settingsScrollOffset = settingsMenuIndex - PIN_ROW;
    }

    // Draw settings items at the top of content area - adjusted spacing
    int startY = CONTENT_START_Y + 15;  // Moved up to just below header
    int lineHeight = 18;              // Reduced line height to fit 5 items

    for (int i = settingsScrollOffset; i < min(NUM_SETTINGS_ITEMS, settingsScrollOffset + VISIBLE_ITEMS); i++) {
        int y = startY + ((i - settingsScrollOffset) * lineHeight);

        tft.setFreeFont(FONT_SANS_9);

        // If the item is "Card Type" and smart text is disabled, draw it in gray and make it unselectable.
        if (strcmp(settingsItems[i], "Card Type") == 0 && !smartTextEnabled) {
            tft.setTextColor(TFT_DARKGREY, TFT_BLACK);
        } else {
            if (i == settingsMenuIndex) {
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
        }

        tft.setCursor(10, y);
        if (strcmp(settingsItems[i], "Card Type") == 0) {
            tft.print("- Card Type");
        } else {
            tft.print(settingsItems[i]);
        }
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

    // Default start position for options
    int startY = HEADER_HEIGHT + 50;

    // Add description for Mirror Screen sub-menu
    if (settingsMenuIndex == 3) { // Mirror Screen
        tft.setFreeFont(FONT_SANS_9);
        tft.setCursor(5, startY);
        tft.print("Mirror received messages");
        startY += 20; // Move options down to make space for the description
    } else if (settingsMenuIndex == 5) { // Smart Text
        tft.setFreeFont(FONT_SANS_9);
        tft.setCursor(5, startY);
        tft.print("Enable commands like #");
        startY += 20; // Move options down to make space for the description
    }

    // New 3-phase scrolling logic for sub-menus
    const int SUB_VISIBLE_ITEMS = 3;
    const int SUB_PIN_ROW = SUB_VISIBLE_ITEMS / 2; // Middle row (index 1 for 3 items)
    int num_options = 0;
    if (settingsMenuIndex == 0) num_options = NUM_BRIGHTNESS_OPTIONS;
    else if (settingsMenuIndex == 1) num_options = NUM_STANDBY_OPTIONS;
    else if (settingsMenuIndex == 3) num_options = NUM_MIRROR_OPTIONS;
    else if (settingsMenuIndex == 5) num_options = NUM_SMART_TEXT_OPTIONS;
    else if (settingsMenuIndex == 6) num_options = NUM_CARD_TYPE_OPTIONS;

    // Scrolling logic
    if (num_options <= SUB_VISIBLE_ITEMS) {
        // Simple case: The list is short and doesn't need to scroll.
        subMenuScrollOffset = 0;
    } else {
        // Use 3-phase scrolling for lists that are longer than the visible area.
        if (subMenuIndex < SUB_PIN_ROW) {
            // Phase 1: Highlight moves down from top
            subMenuScrollOffset = 0;
        } else if (subMenuIndex >= (num_options - (SUB_VISIBLE_ITEMS - SUB_PIN_ROW))) {
            // Phase 3: Highlight moves down at the bottom
            subMenuScrollOffset = num_options - SUB_VISIBLE_ITEMS;
        } else {
            // Phase 2: Highlight is pinned to the middle, list scrolls
            subMenuScrollOffset = subMenuIndex - SUB_PIN_ROW;
        }
    }

    // Draw submenu items with adjusted spacing
    int lineHeight = 20;

    auto draw_items = [&](const char* items[], int count) {
        for (int i = subMenuScrollOffset; i < min(count, subMenuScrollOffset + SUB_VISIBLE_ITEMS); i++) {
            int y = startY + ((i - subMenuScrollOffset) * lineHeight);

            tft.setFreeFont(FONT_SANS_9);

            if (i == subMenuIndex) {
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
            } else {
                tft.setTextColor(TFT_WHITE, TFT_BLACK);
            }
            tft.setCursor(10, y);
            tft.print(items[i]);
        }
    };

    if (settingsMenuIndex == 0) { // Brightness
        draw_items(brightnessOptions, NUM_BRIGHTNESS_OPTIONS);
    } else if (settingsMenuIndex == 1) { // Auto Standby
        draw_items(standbyOptions, NUM_STANDBY_OPTIONS);
    } else if (settingsMenuIndex == 3) { // Mirror Screen
        draw_items(mirrorOptions, NUM_MIRROR_OPTIONS);
    } else if (settingsMenuIndex == 5) { // Smart Text
        draw_items(smartTextOptions, NUM_SMART_TEXT_OPTIONS);
    } else if (settingsMenuIndex == 6) { // Card Type
        draw_items(cardTypeOptions, NUM_CARD_TYPE_OPTIONS);
    }
}

// ============================================================================
// BRIGHTNESS OVERLAY
// ============================================================================

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
    brightnessChanged = true;
    brightnessDisplayTime = millis();
}

void handleBrightnessDisplay() {
    if (showingBrightness && (millis() - brightnessDisplayTime) > 3000) {
        showingBrightness = false;
        if (brightnessChanged) {
            saveBrightness();
            brightnessChanged = false;
        }
        // Redisplay current content based on the current page
        switch(currentPage) {
            case PAGE_MESSAGES:
                displayCurrentMessage();
                break;
            // Add other cases if brightness can be changed from other pages
            default:
                // Default action: clear the content area
                clearContentArea();
                break;
        }
    }
}