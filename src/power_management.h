#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include "globals.h"

void checkSleep();
void enterDeepSleep();
void wakeFromSleep();
int readBatteryLevel();

#endif // POWER_MANAGEMENT_H