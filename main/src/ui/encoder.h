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
void onClickResetState(EncoderButton &eb);

// Zeroing handlers
void onClickZeroMachineXY(EncoderButton &eb);
void startZeroWorkspace();
void onClickZeroWorkspaceZ(EncoderButton &eb);
void onClickZeroWorkspaceXY(EncoderButton &eb);
void onHoldAutoTouch(HandleButtons &btn);
void onClickAcceptAutoTouchE(EncoderButton &eb);
void onClickAcceptAutoTouchH(HandleButtons &btn);
void onClickCancelAutoTouch(HandleButtons &eb);

// Thickness handlers
void onEncoderUpdateThickness(EncoderButton &eb);
void onClickSetThickness(EncoderButton &eb);

// Design mode handlers
void onEncoderDesignOrCalibrate(EncoderButton &eb);
void onClickSetDoC(EncoderButton &eb);
void onEncoderUpdateDesign(EncoderButton &eb);
void onClickMakePath(EncoderButton &eb);

// Path execution handlers
void onEncoderSetSpeed(EncoderButton &eb);
void onEncoderSetBoost(EncoderButton &eb);
void onClickSetSpeed(EncoderButton &eb);
void onClickPauseCut(EncoderButton &eb);
void onEncoderPauseMenu(EncoderButton &eb);
void onClickPauseSelect(EncoderButton &eb);

// Calibration handlers
void onClickCalibrationAdvance(EncoderButton &eb);
void onEncoderAcceptCalibration(EncoderButton &eb);
void onClickAcceptCalibration(EncoderButton &eb);

// Mode switching functions
void encoderSetThickness();
void encoderDesignOrCalibrate();
void encoderDesignType();
void encoderDesignSelect();
void encoderZeroWorkspaceXY();
void encoderHandlePause();
void encoderEndScreen();

// Zeroing functions
void startZeroWorkspace();
void acceptAutoTouch();

#endif
