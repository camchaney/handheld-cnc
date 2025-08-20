#ifndef ENCODER_CALLBACKS_H
#define ENCODER_CALLBACKS_H

#include <EncoderButton.h>
#include "../config.h"
#include "../globals.h"
#include "../actuation/motors.h"
#include "fileUI.h"
#include "../path/path-generators.h"
#include "../io/logging.h"
#include "../sensors/sensors.h"

// Basic handlers
void nullHandler(EncoderButton &eb);

// Zeroing handlers
void onClickZeroMachineXY(EncoderButton &eb);
void onClickZeroWorkspaceZ(EncoderButton &eb);

// Calibration handlers
void onClickCalibrationAdvance(EncoderButton &eb);

// Mode switching functions
void encoderDesignSelect();
void encoderEndScreen();

// Compass UI Functions
void setCompassHandler();
#endif
