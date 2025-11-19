# Project To-Do List

This file tracks planned features and necessary bug fixes for the TTGO BLE Message Display project.

## Features & Enhancements

- [x] **Fix Mirror Setting:** The "Mirror Screen" feature now correctly renders message text as a mirror image when set to "On".
- [ ] **Implement Smart Text:** Add the "Smart Text" setting to intelligently format or process incoming messages.
- [ ] **OTA Updates:** Integrate Over-the-Air (OTA) update capabilities to allow for wireless firmware updates.
- [ ] **Unique Device Identification:**
    - [ ] Develop a method to flash multiple devices with unique BLE connection names and device-specific information.
    - [ ] Add a settings option to control message reception behavior:
        - **Broadcast Mode:** Allow multiple devices to receive the same message simultaneously.
        - **Unique Receiver Mode:** Ensure only one specific device receives a message.
    - [ ] Implement a prefix code system. A message will only be displayed if it is preceded by a specific code that matches a value set on the device. This could be part of the unique receiver setting.
- [ ] **Companion iOS App:**
    - [ ] Develop a companion iPhone app using React Native with Expo Go for rapid prototyping and testing.
    - [ ] The app should be able to send messages to the device via BLE.
    - [ ] Eventually, package the app for submission to the Apple App Store using a developer account.
- [ ] **Responsive UI:** Adapt the on-screen display logic to be compatible with other devices that have different screen sizes and resolutions.