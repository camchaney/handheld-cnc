#include "globals.h"
#include "callbacks.h"
#include "../path/path-generators.h"
#include "../path/path-execution.h"
#include "../sensors/sensors.h"
#include "../io/settings.h"

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

void onStartRezeroXY(void* ctx) {
	if (!ui.confirm("Rezero XY?")) return;
	workspaceZeroXY();
}

void onClickCompassUI(EncoderButton &eb) {
	ui.enter();
}

void onDoubleClickCompassUI(EncoderButton &eb) {
	ui.back();
}

void onEncoderCompassUI(EncoderButton &eb) {
	ui.adjust(eb.increment());
}

void onStartSaveSettings(void* ctx) {
	if (!ui.confirm("Save settings?")) return;
	saveSettings();
}

void onStartSelectFile(void* ctx) {
	ui.enable(false);
	designType = FROM_FILE;
	encoderDesignSelect();
}

void onStartCancelCut(void* ctx) {
	if (!ui.confirm("Cancel cutting?")) return;

	stopCutting();
}

void onStartFileCut(void* ctx) {
	startCutting();
}

void onStartPresetCut(void* ctx) {
    Menu* item = (Menu*)ctx;
    Serial.print("Start cutting with preset: ");
    Serial.println(item->calledFrom->label);
	designType = PRESET;
	state = DESIGN_SELECTED;
	selectedDesignPreset = item->calledFrom->label[0];
    makePresetPath(item->calledFrom->label[0]);
	startCutting();
}

void onStartSpeedRunCut(void* ctx) {
	handleSpeedRun();
	state = DESIGN_SELECTED;
	startCutting();
}

void startCutting() {
	if (settings.autoZeroXY)
		workspaceZeroXY();

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