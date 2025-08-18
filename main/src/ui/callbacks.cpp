#include "globals.h"
#include "callbacks.h"
#include "../path/path-generators.h"

void onCuttingMenuBack(void* ctx) {
	ui.displayState = DisplayState::Compass;
	ui.showCompass(true);
}

void onStartCalibrate(void* ctx) {
  Serial.println("Start calibration...");
}

void onClickCompassUI(EncoderButton &eb) {
	ui.enter();
}

void onEncoderCompassUI(EncoderButton &eb) {
	ui.adjust(eb.increment());
}

void onStartPresetCut(void* ctx) {
    Menu* item = (Menu*)ctx;
    Serial.print("Start cutting with preset: ");
    Serial.println(item->calledFrom->label);
	designType = PRESET;
	state = DESIGN_SELECTED;
	selectedDesignPreset = item->calledFrom->label[0];
	ui.displayState = DisplayState::Compass;
	ui.showCompass(true);
    makePresetPath(item->calledFrom->label[0]);
	workspaceZeroXY();

	// Hack for opensauce, auto-zero XY
	if (autoZeroXY)	workspaceZeroXY();

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