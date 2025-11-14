#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "../globals.h"
#include "../config.h"
#include "../types.h"

// Basic geometric operations
float myDist(float x1, float y1, float x2, float y2);
float clamp(float val, float min, float max);
float principalAngleRad(float x);
float mapF(float x, float in_min, float in_max, float out_min, float out_max);

// Path and angle calculations
float signedDist(RouterPose rPose, Point g);
float angleFrom(RouterPose pose, Point a, Point b);
int direction(RouterPose pose, Point g, Point n);

#endif
