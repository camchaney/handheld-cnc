#ifndef SENSORS_H
#define SENSORS_H

#include <EEPROM.h>
#include "../config.h"
#include "../globals.h"
#include "../ui/display.h"
#include "../ui/encoder.h"
#include "../math/geometry.h"

void sensorSetup();
void setupSensorTimer();
void sensorTimerISR();
void readAllSensors();
void doSensing();
void doSensingLinear();
void sensorPlotting();
void readEepromCalibration();
void writeEepromCalibration();
void calibrate();
int16_t convTwosComp(int16_t value);

#endif
