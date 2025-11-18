# Project Changelog

This file tracks the major changes and decisions made during the development of the TTGO BLE Message Display project.

## November 2025

### Messages Page UI/UX Refinements
*   **Reverted Viewport Logic:** Removed the complex wrap-and-scroll viewport from the Messages page to resolve text cutoff and alignment issues, returning to a simpler display method.
*   **Layout and Color Correction:**
    *   Adjusted the vertical alignment of all navigation text ("Prev->", "Next->", "Msg x/x", etc.) for consistency.
    *   Changed the navigation text color to `TFT_CYAN` to match other screens.
    *   Moved the top navigation line directly under the header and the bottom line to the absolute bottom of the display.
    *   Restored the message text color to `TFT_GREEN`.

### Settings Menu Functionality
*   **Restored Auto Standby:** Re-implemented the selection logic in the "Auto Standby" sub-menu, allowing the user to cycle through and save different inactivity timers.
*   **Improved Brightness Saving:** Modified the brightness sub-menu so that the selected brightness level is saved instantly upon exiting the sub-menu, rather than after a delay.

### Project Documentation
*   **Created `README.md`:** Added a comprehensive README file detailing the project's features, hardware, usage instructions, and build process.
*   **Created `context.md`:** Initialized this changelog to track future development history.

### UI and Stability Fixes
*   **Refined Menu Scrolling:** Implemented a more intuitive three-phase scrolling logic for settings menus (highlight moves, list scrolls, highlight moves again).
*   **Corrected Menu Highlighting:** Changed the highlight style in all menus from a white background block to green text for a cleaner, more consistent look.
*   **Fixed Message Display:**
    *   Resolved a persistent issue where the top line of message text was clipped. The final fix involved adjusting the cursor's Y-position within the drawing sprite to `fontHeight() - 8`.
    *   Corrected the vertical alignment of the entire message block to remove the unwanted gap below the navigation hints.
    *   Re-enabled the dynamic font sizing feature to ensure messages are always displayed as large as possible.
*   **Prevented Crashes:**
    *   Fixed a `LoadProhibited` crash when entering sub-menus with fewer items than the visible area (e.g., "Mirror Screen") by improving the scrolling logic.
    *   Made settings navigation more robust by preventing entry into unimplemented menu items, which previously caused crashes.
*   **Added Mirror Screen Feature:** Implemented a "Mirror Screen" setting that horizontally flips the message text display. This setting is saved to and loaded from EEPROM.

### Major Code Refactoring
*   **Modularized Codebase:** Broke down the monolithic `main.cpp` file into multiple, single-responsibility modules to improve organization, readability, and maintainability.
*   **Created New Modules:**
    *   `display`: Manages all screen drawing and UI updates.
    *   `buttons`: Handles all user input logic.
    *   `ble_handler`: Encapsulates all Bluetooth Low Energy functionality.
    *   `power_management`: Contains deep sleep and battery monitoring logic.
    *   `settings`: Manages loading, saving, and applying all user configurations.
*   **Centralized Globals:** Introduced a `globals.h` file to declare shared variables, constants, and enums, making state management clearer across modules.