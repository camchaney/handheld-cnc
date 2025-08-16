#ifndef MOTORS_H
#define MOTORS_H

#include "../types.h"
#include "../config.h"
#include "../globals.h"
#include "actuator.h"

void motorSetup();
void driverSetup();
void enableStepperZ();
void enableStepperRL();
void disableStepperZ();
void disableStepperRL();
void stopStepperX();
void stopStepperZ();
void machineZeroXY();
void workspaceZeroZ();
void autoTouchWorkspaceZ();
void acceptAutoTouchWorkspaceZ();
void workspaceZeroXY();
void cartesianToMotor(Position pos);
void motorToCartesian(float &x, float &y, float &z);
void zStepISR();

#endif
