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