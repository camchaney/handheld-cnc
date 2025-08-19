#ifndef CALLBACKS_H
#define CALLBACKS_H

#include "globals.h"

void onCuttingMenuBack(void* ctx);
void onStartPresetCut(void* ctx);
void onStartFileCut(void* ctx);
void onStartSpeedRunCut(void* ctx);
void onStartSelectFile(void* ctx);
void onStartCancelCut(void* ctx);
void onStartCalibrate(void* ctx);
void onStartRezeroXY(void* ctx);

void onClickCompassUI(EncoderButton &eb);
void onDoubleClickCompassUI(EncoderButton &eb);
void onEncoderCompassUI(EncoderButton &eb);

void startCutting();

#endif