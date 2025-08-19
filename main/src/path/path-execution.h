#ifndef PATH_EXECUTION_H
#define PATH_EXECUTION_H

#include "../types.h"
#include "../config.h"
#include "../globals.h"
#include "../actuation/motors.h"
#include "../actuation/actuator.h"
#include "planner.h"
#include "../math/geometry.h"
#include "../io/logging.h"
#include "../ui/display.h"
#include "../ui/encoder.h"
#include "../actuation/actuator.h"

// Safety check functions
bool checkEndstops();

// Path advancement and execution functions
void handleChickenHead();
void handleCutting(long deltaTime);
void stopCutting();

#endif