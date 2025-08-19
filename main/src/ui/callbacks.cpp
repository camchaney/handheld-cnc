#include "globals.h"
#include "callbacks.h"
#include "../path/path-generators.h"
#include "../path/path-execution.h"
#include "../sensors/sensors.h"

void onStartDummy(void* ctx) {
	// This is a dummy callback function that does nothing
	Serial.println("Dummy callback called");
}

void onCuttingMenuBack(void* ctx) {
	ui.showCompass(true);
}

void onStartCalibrate(void* ctx) {
	if (!ui.confirm("Start calibration?")) return;
	calibrate();
}

void onClickCompassUI(EncoderButton &eb) {
	ui.enter();
}

void onDoubleClickCompassUI(EncoderButton &eb) {
	if (ui.isConfirming())
		ui.enter(); // strange bug, first click in the confirm is detected as double click
	else
		ui.back();
}

void onEncoderCompassUI(EncoderButton &eb) {
	ui.adjust(eb.increment());
}

void onStartSelectFile(void* ctx) {
	ui.enable(false);
	designType = FROM_FILE;
	encoderDesignSelect();
}

void onStartStopCut(void* ctx) {
	if (!ui.confirm("Stop cutting?")) return;

	stopCutting();
}

void onStartFileCut(void* ctx) {
	// Hack for opensauce, auto-zero XY
	// TODO: remove this
	if (autoZeroXY) workspaceZeroXY();

	// Reset cutting path
	running = true;
	current_point_idx = 0;
	if (designType != SPEED_RUN) feedrate = feedrate_default;	// reset feedrate to default (NOTE: only RMRRF addition)
	feedrateBoost = 1.0;	// reset feedrate boost to default
	speedRunTimer = 0;

	state = READY;
	cutState = NOT_CUT_READY;
	ui.showCompass(true);
	
	// Clear out sensors in case we moved while in design mode
	for (int i = 0; i < 4; i++) {
		sensors[i].readBurst();
	}
}

void onStartPresetCut(void* ctx) {
    Menu* item = (Menu*)ctx;
    Serial.print("Start cutting with preset: ");
    Serial.println(item->calledFrom->label);
	designType = PRESET;
	state = DESIGN_SELECTED;
	selectedDesignPreset = item->calledFrom->label[0];
	ui.showCompass(true);
    makePresetPath(item->calledFrom->label[0]);

	// Hack for opensauce, auto-zero XY
	if (autoZeroXY) workspaceZeroXY();

	// Reset cutting path
	running = true;
	current_point_idx = 0;
	if (designType != SPEED_RUN) feedrate = feedrate_default;	// reset feedrate to default (NOTE: only RMRRF addition)
	feedrateBoost = 1.0;	// reset feedrate boost to default
	speedRunTimer = 0;

	state = READY;
	cutState = NOT_CUT_READY;

	// Clear out sensors in case we moved while in design mode
	for (int i = 0; i < 4; i++) {
		sensors[i].readBurst();
	}
}