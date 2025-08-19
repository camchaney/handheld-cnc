#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "globals.h"

void onCuttingMenuBack(void* ctx);
void onStartPresetCut(void* ctx);
void onStartSelectFile(void* ctx);
void onStartFileCut(void* ctx);
void onStartStopCut(void* ctx);
void onStartCalibrate(void* ctx);
void onStartDummy(void* ctx);

void onClickCompassUI(EncoderButton &eb);
void onDoubleClickCompassUI(EncoderButton &eb);
void onEncoderCompassUI(EncoderButton &eb);

#endif