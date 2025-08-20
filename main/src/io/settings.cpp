#include "settings.h"
#include "globals.h"
#include <EEPROM.h>

void loadSettings() {
    int version;
    EEPROM.get(SETTINGS_ADDRESS, version);
    if (version == SETTINGS_VERSION)
	    EEPROM.get(SETTINGS_ADDRESS, settings);
    else
        Serial.println("Settings version mismatch, using default settings.");
}

void saveSettings() {
	EEPROM.put(SETTINGS_ADDRESS, settings);
}
