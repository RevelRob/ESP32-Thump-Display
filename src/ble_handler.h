#ifndef BLE_HANDLER_H
#define BLE_HANDLER_H

#include "globals.h"

void setupBLE();
void setConnected(bool connected);
void addMessageToHistory(String message);
bool processSmartTextMessage(String message);
void checkAutoClear();

#endif // BLE_HANDLER_H