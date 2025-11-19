# TTGO BLE Message Display

This project transforms a TTGO T-Display ESP32 board into a versatile, portable Bluetooth Low Energy (BLE) message display. It is designed to receive and show text messages from a connected client, such as a smartphone, and includes a user-friendly interface for message navigation and device configuration.

## Hardware

*   **TTGO T-Display ESP32:** This project is specifically tailored for this board, utilizing its integrated TFT screen and the two onboard user buttons (BUTTON_1 and BUTTON_2).

## Features

*   **Wireless Message Display:** Receives text data over a BLE UART service and displays it on the screen.
*   **Message History:** Keeps a history of the last 20 messages, allowing you to scroll back and forth.
*   **Auto-scrolling for long messages:** Messages that are too long to fit on the screen will automatically scroll.
*   **Dynamic User Interface:**
    *   A persistent header displays critical status information: BLE connection status (green/red), battery level, and the current screen name.
    *   Message text automatically resizes to best fit the display area.
    *   A multi-page interface separates Messages, Settings, and general Info.
*   **Main Menu:** A simple, navigable main menu to switch between the device's core functions.
*   **Comprehensive Settings Menu:**
    *   **Brightness Control:** Adjust the screen backlight brightness from 10% to 100% in 10% increments.
    *   **Auto Standby:** Configure an inactivity timer (from 10 seconds to 2 hours) to automatically enter a power-saving deep sleep mode.
    *   **Persistent Settings:** Brightness and standby settings are saved to EEPROM, so they are retained through restarts and wake-ups.
    *   **Mirror Screen:** Option to horizontally mirror the displayed message text.
*   **Advanced Power Management:**
    *   Utilizes ESP32's deep sleep to significantly extend battery life.
    *   The device can be woken from sleep by pressing either of the two buttons.

## How to Use

The interface is controlled using the two buttons on the TTGO board: **Button 1 (Bottom/Right)** and **Button 2 (Top/Left)**. The button functions change based on the current screen.

### Main Menu
*   **Nxt-> (Button 1):** Cycles through the menu options (Messages, Settings, Info).
*   **Sel-> (Button 2):** Enters the selected menu page.

### Messages Page
*   **Next-> (Button 1):** Displays the next message in the history.
*   **Prev-> (Button 2):** Displays the previous message in the history.
*   **Hold Either Button:** A long press on either button will return you to the Main Menu.
*   If no messages have been received, pressing any button will return you to the Main Menu.

### Settings Menu
*   **Nxt-> (Button 1):** Cycles through the available settings (e.g., Brightness, Auto Standby).
*   **Sel-> (Button 2):** Enters the sub-menu for the selected setting.

#### In a Sub-Menu (e.g., Brightness):
*   **Nxt-> (Button 1):** Cycles through the available options (e.g., 10%, 20%, 30%). The setting is applied immediately.
*   **Sel-> (Button 2):** Exits the sub-menu and returns to the main settings list. Your last selected option is saved.

### Info Page
*   Displays credits and project information.
*   Pressing any button will return you to the Main Menu.

## BLE Service

To send messages to the device, you can use any BLE mobile app that supports a serial or UART profile.

*   **Service UUID:** `6E400001-B5A3-F393-E0A9-E50E24DCCA9E`
*   **Characteristic UUID (Write):** `6E400002-B5A3-F393-E0A9-E50E24DCCA9E`

Messages should be terminated with a newline character (`\n`) to be processed and displayed individually.

## Building the Project

This project is configured for **PlatformIO** within Visual Studio Code.

1.  Install Visual Studio Code.
2.  Install the PlatformIO IDE extension from the VS Code marketplace.
3.  Clone this repository.
4.  Open the project folder in VS Code.
5.  PlatformIO will automatically detect the `platformio.ini` file and download the necessary libraries (like `TFT_eSPI`).
6.  Connect your TTGO T-Display board and use the PlatformIO controls to build and upload the firmware.

## Credits

*   For use with Toxic+
*   By Ian Pidgeon
*   Created by Rob Testa
*   November 2025