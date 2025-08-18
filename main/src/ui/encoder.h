#ifndef ENCODER_CALLBACKS_H
#define ENCODER_CALLBACKS_H

#include <EncoderButton.h>
#include "../config.h"
#include "../globals.h"
#include "../actuation/motors.h"
#include "display.h"
#include "../path/path-generators.h"
#include "../io/logging.h"
#include "../sensors/sensors.h"

// Basic handlers
void nullHandler(EncoderButton &eb);
void onClickGoToSetThickness(EncoderButton &eb);

// Zeroing handlers
void onClickZeroMachineXY(EncoderButton &eb);
void onClickZeroWorkspaceZ(EncoderButton &eb);
void onClickZeroWorkspaceXY(EncoderButton &eb);

// Path execution handlers
void onEncoderSetSpeed(EncoderButton &eb);
void onEncoderSetBoost(EncoderButton &eb);

// Calibration handlers
void onClickCalibrationAdvance(EncoderButton &eb);
void onEncoderAcceptCalibration(EncoderButton &eb);
void onClickAcceptCalibration(EncoderButton &eb);

// Mode switching functions
void encoderDesignSelect();
void encoderZeroWorkspaceXY();
void encoderEndScreen();

#endif
