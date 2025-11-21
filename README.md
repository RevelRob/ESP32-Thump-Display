# ESP32 BLE Message Display

This project transforms an ESP32 board with an ST7789 display into a versatile, portable Bluetooth Low Energy (BLE) message display. It receives and shows text messages from a connected client (like a smartphone) and includes a user-friendly interface for message navigation and device configuration.

![Project Demo GIF](https://via.placeholder.com/400x240.png?text=Project+Demo+GIF)
*(Add a GIF or image of the display in action!)*

## Hardware

This project is designed for the **TTGO T-Display ESP32** board, utilizing its integrated 1.35" TFT screen and two onboard user buttons.

*   **Board:** TTGO T-Display ESP32
*   **Button 1 (Bottom/Right):** GPIO 35
*   **Button 2 (Top/Left):** GPIO 0

> **Note:** While the `platformio.ini` uses the generic `esp32dev` board definition, the pin configuration and button logic are specific to the TTGO T-Display.

## Features

*   **Wireless Message Display:** Receives text data over a BLE UART service and displays it on the screen.
*   **Message History:** Keeps a history of the last 20 messages, allowing you to scroll back and forth.
*   **Auto-scrolling:** Messages that are too long to fit on the screen will automatically scroll vertically.
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

The interface is controlled using the two onboard buttons. Their function changes based on the current screen.

| Screen | Button 1 (Bottom/Right) | Button 2 (Top/Left) | Long Press (Either) |
| :--- | :--- | :--- | :--- |
| **Main Menu** | `Nxt->` (Cycle options) | `Sel->` (Select option) | - |
| **Messages** | `Next->` (Next message) | `Prev->` (Previous message) | Return to Menu |
| **Settings** | `Nxt->` (Cycle settings) | `Sel->` (Enter sub-menu) | - |
| **Settings Sub-Menu** | `Nxt->` (Cycle values) | `Sel->` (Save and exit) | - |
| **Info Page** | Return to Menu | Return to Menu | - |

---

## Smart Text Commands

The device supports special "Smart Text" commands to perform actions beyond just displaying text. All commands are prefixed with a `#`.

| Command | Description | Example |
| :--- | :--- | :--- |
| `#` | Clears the current message from the screen. | Send `#` |
| `#CARDS` | Prepares the device to display a playing card sent on the next line. | Send `#CARDS`, then send `122` to display "Queen of Hearts". |

### Card Code Format
The card code is a 2 or 3-digit number.

*   **Card Value:**
    *   `1` - `9`: Ace - 9
    *   `10`: 10
    *   `11`: Jack
    *   `12`: Queen
    *   `13`: King
*   **Suit Value:**
    *   `1`: Spades (♠)
    *   `2`: Hearts (♥)
    *   `3`: Clubs (♣)
    *   `4`: Diamonds (♦)

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