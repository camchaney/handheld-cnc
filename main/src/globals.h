#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>
#include <AccelStepper.h>
#include <TMCStepper.h>
#include <PMW3360.h>
#include <SdFat.h>
#include <Arduino_GFX_Library.h>
#include <EncoderButton.h>
#include "config.h"
#include "types.h"

// Global object declarations
extern AccelStepper stepperR;
extern AccelStepper stepperL;
extern AccelStepper stepperZ;
extern TMC2209Stepper driverR;
extern TMC2209Stepper driverL;
extern TMC2209Stepper driverZ;
extern PMW3360 sensors[4];
extern EncoderButton encoder;
extern Arduino_GFX *screen;
extern SdFat sd;

// State variables
extern State state;
extern CutState cutState;
extern bool running;
extern bool valid_sensors;
extern float motorPosX;
extern DesignType designType;

// Path data
extern Path path;
extern Path pathBuffer;
extern int current_point_idx;

// SD stuff
extern FsFile logFile;
extern FsFile root;
extern FsFile currentDir;
extern String currentPath;
extern int current_file_idx;
extern int totalFiles;
extern String fileList[MAX_FILES];

// Position tracking
extern RouterPose pose;
extern float measVel[2][4];
extern float calPos[2][4];
extern float dXY;
extern float distanceTraveled;

// Calibration data
extern CalParams cal[4];

// Kinematics
extern float feedrate;						// speed of tracking (mm/s)
extern float feedrateBoost;

// Mode flags
extern bool plottingOn;
extern bool debuggingOn;
extern bool stopwatchOn;
extern bool outputSerialOn;
extern bool outputSDOn;
extern int designOrCalibrate;
extern int acceptCal;
extern int designPreset;
extern int pauseSelection;

// Material properties
extern float matThickness;

// Timing variables
extern long unsigned totalLoopTime;
extern long unsigned sensingTime_debug;
extern long unsigned stepperTime;
extern long unsigned serialTime;
extern long unsigned safetyTime;
extern long unsigned cuttingTime;
extern long unsigned SDLogTime;
extern long unsigned timeLoopStart;
extern long unsigned timeLastDebounce;
extern long unsigned lastDraw;
extern long unsigned timeLastPoll;
extern long unsigned sensingTime;
extern elapsedMicros runTimer;
extern elapsedMicros filemicros;
extern elapsedMillis speedRunTimer;
extern elapsedMillis bufferTimer;
extern uint8_t iter;

#endif