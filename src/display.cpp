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

    // Update scroll position
    if (settingsMenuIndex < settingsScrollOffset) {
        settingsScrollOffset = settingsMenuIndex;
    } else if (settingsMenuIndex >= settingsScrollOffset + 5) { // MAIN_MENU_VISIBLE_ITEMS is 5
        settingsScrollOffset = settingsMenuIndex - 5 + 1;
    }

    // Draw settings items at the top of content area - adjusted spacing
    int startY = CONTENT_START_Y + 15;  // Moved up to just below header
    int lineHeight = 18;              // Reduced line height to fit 5 items

    for (int i = settingsScrollOffset; i < min(NUM_SETTINGS_ITEMS, settingsScrollOffset + 5); i++) {
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
    if (subMenuIndex < subMenuScrollOffset) {
        subMenuScrollOffset = subMenuIndex;
    } else if (subMenuIndex >= subMenuScrollOffset + 3) { // SUB_MENU_VISIBLE_ITEMS is 3
        subMenuScrollOffset = subMenuIndex - 3 + 1;
    }

    // Draw submenu items with adjusted spacing
    int startY = HEADER_HEIGHT + 50;
    int lineHeight = 20; // Keep original spacing for sub-menus

    if (settingsMenuIndex == 0) { // Brightness
        for (int i = subMenuScrollOffset; i < min(NUM_BRIGHTNESS_OPTIONS, subMenuScrollOffset + 3); i++) {
            int y = startY + ((i - subMenuScrollOffset) * lineHeight);

            tft.setFreeFont(FONT_SANS_9);

            if (i == subMenuIndex) {
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
        for (int i = subMenuScrollOffset; i < min(NUM_STANDBY_OPTIONS, subMenuScrollOffset + 3); i++) {
            int y = startY + ((i - subMenuScrollOffset) * lineHeight);

            tft.setFreeFont(FONT_SANS_9);

            if (i == subMenuIndex) {
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