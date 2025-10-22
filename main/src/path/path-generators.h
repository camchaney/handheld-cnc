#ifndef PATH_GENERATORS_H
#define PATH_GENERATORS_H

#include "../config.h"
#include "../globals.h"
#include "../io/logging.h"

Point lineGenerator(int i);
Point sinGenerator(int i);
Point zigZagGenerator(int i);
Point doubleLineGenerator(int i);
Point circleGenerator(int i);
Point diamondGenerator(int i);
Point squareGeneratorSine(int i);
// Point squareGeneratorWave(int i);
// Point squareGeneratorMake(int i);
// Point drillSquareGenerator(int i);
void makePresetPath();
void updatePresetPathBuffer();

// Path properties
extern const float sinAmp;
extern const float sinPeriod;
extern const float pathMax_y;
extern const float circleDiameter;

#endif
