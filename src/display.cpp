#include "display.h"

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

void displayCurrentMessage() {
    clearContentArea();
    tft.setRotation(1);

    currentPage = PAGE_MESSAGES;
    setScreenName("Messages");

    // Define positions and dimensions
    int topY = HEADER_HEIGHT + 16; // Pushed up to be right below the header
    int bottomY = tft.height() - 6; // Pushed down to the bottom of the display
    int contentStartY = topY + 8; // Position sprite just below the top nav text

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


    // Create a sprite for the message content area
    int contentWidth = tft.width();
    int contentHeight = tft.height() - contentStartY - 20;
    TFT_eSprite messageSprite = TFT_eSprite(&tft);
    messageSprite.createSprite(contentWidth, contentHeight);
    messageSprite.fillSprite(TFT_BLACK); // Revert background to black

    // Set text properties for the sprite
    messageSprite.setTextColor(TFT_GREEN, TFT_BLACK);

    if (totalMessages == 0) {
        messageSprite.setFreeFont(FONT_SANS_9);
        // Set cursor y to font height to prevent clipping at the top
        messageSprite.setCursor(5, messageSprite.fontHeight() - 8);
        messageSprite.println("Waiting for messages..."); // println still works with datum
    } else {
        if (displayMessageIndex < 0) displayMessageIndex = 0;
        if (displayMessageIndex >= totalMessages) displayMessageIndex = totalMessages - 1;

        String message = messageHistory[displayMessageIndex];

        // Determine the best font size
        const GFXfont* font = FONT_SANS_9; // Default to smallest
        if (doesTextFit(message, FONT_SANS_BOLD_12, contentWidth - 10, contentHeight)) {
            font = FONT_SANS_BOLD_12;
        } else if (doesTextFit(message, FONT_SANS_12, contentWidth - 10, contentHeight)) {
            font = FONT_SANS_12;
            }

        messageSprite.setFreeFont(font);
        // Set cursor y to font height to prevent clipping at the top
        messageSprite.setCursor(5, messageSprite.fontHeight() - 8);

        // Simple word wrap implementation onto the sprite
        String currentLine = "";
        String word = "";
        for (int i = 0; i < message.length(); i++) {
            char c = message[i];
            if (c == ' ' || c == '\n' || i == message.length() - 1) {
                if (i == message.length() - 1 && c != ' ' && c != '\n') {
                    word += c;
                }

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

    // Push the sprite to the screen (mirrored or normal)
    if (mirrorMessages) {
        messageSprite.pushSprite(0, contentStartY, true); // The 'true' flag enables mirroring
    } else {
        messageSprite.pushSprite(0, contentStartY);
    }

    // Delete the sprite to free up memory
    messageSprite.deleteSprite();
}

bool doesTextFit(String message, const GFXfont* font, int maxWidth, int maxHeight) {
    // This function now uses a temporary sprite to measure text, to avoid screen flicker
    TFT_eSprite tempSprite = TFT_eSprite(&tft);
    tempSprite.createSprite(1, 1); // Minimal sprite
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
                currentLine = word;
                currentY += lineHeight;
                if (currentY + lineHeight > maxHeight) {
                    tempSprite.deleteSprite();
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

    tempSprite.deleteSprite();
    return currentY <= maxHeight;
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

        if (i == settingsMenuIndex) {
            tft.setTextColor(TFT_GREEN, TFT_BLACK);
        } else {
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
        }

        tft.setCursor(10, y);
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

    // Default start position for options
    int startY = HEADER_HEIGHT + 50;

    // Add description for Mirror Screen sub-menu
    if (settingsMenuIndex == 3) { // Mirror Screen
        tft.setFreeFont(FONT_SANS_9);
        tft.setCursor(5, startY);
        tft.print("Mirror messages received");
        startY += 20; // Move options down to make space for the description
    }

    // New 3-phase scrolling logic for sub-menus
    const int SUB_VISIBLE_ITEMS = 3;
    const int SUB_PIN_ROW = SUB_VISIBLE_ITEMS / 2; // Middle row (index 1 for 3 items)
    int num_options = 0;
    if (settingsMenuIndex == 0) num_options = NUM_BRIGHTNESS_OPTIONS;
    else if (settingsMenuIndex == 1) num_options = NUM_STANDBY_OPTIONS;
    else if (settingsMenuIndex == 3) num_options = NUM_MIRROR_OPTIONS;

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
}

void hideBrightnessChange() {
    if (showingBrightness) {
        showingBrightness = false;
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