#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "globals.h"

void onCuttingMenuBack(void* ctx);
void onStartCalibrate(void* ctx);

void onClickCompassUI(EncoderButton &eb);
void onDoubleClickCompassUI(EncoderButton &eb);
void onEncoderCompassUI(EncoderButton &eb);

#endif