# Project Changelog

This file tracks the major changes and decisions made during the development of the TTGO BLE Message Display project.

## November 2025

### Message Display Enhancements
*   **Implemented Auto-Scrolling:** Long messages that exceed the display height now scroll automatically from top to bottom, loop, and pause, ensuring full readability.
*   **Fixed Mirrored Text Scroll Speed:** Resolved an issue where mirrored text would scroll at a much slower rate than non-mirrored text.
*   **Corrected Mirrored Text Color:** Fixed a bug that caused mirrored text to appear red instead of the standard green.

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
*   **Perfected Message Display:** After several iterations, resolved the text clipping and positioning issues on the Messages page. The final fix involved adjusting the Y-cursor position within the drawing sprite to `fontHeight() - 8` and correcting the content area's start position. Dynamic font sizing was also restored. Message processing logic was centralized in `main.cpp`'s `loop()` to ensure all fragments are displayed correctly and to prevent race conditions.
*   **Prevented Crashes:**
    *   Fixed a `LoadProhibited` crash when entering sub-menus with fewer items than the visible area (e.g., "Mirror Screen") by improving the scrolling logic.
    *   Made settings navigation more robust to prevent crashes when entering unimplemented menu items.
*   **Implemented Mirror Screen Feature:** The "Mirror Screen" setting now correctly horizontally flips only the message text display. This setting is saved to and loaded from EEPROM.
*   **Stabilized Message Reception:** Resolved issues with message splitting and the last message not displaying by centralizing message buffer processing in the `main.cpp` loop and removing the problematic mutex, ensuring reliable message delivery and display.