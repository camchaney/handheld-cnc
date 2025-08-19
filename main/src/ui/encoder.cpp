#include "encoder.h"
#include "callbacks.h"

// #define NUM_DESIGNS 		9
#define NUM_DESIGNS 		8


void nullHandler(EncoderButton &eb) {
	Serial.println("null handler called");
	return;
}

// CLICK HANDLERS ----------------------------------------
void onClickZeroMachineXY(EncoderButton &eb) {
	drawCenteredText("Zeroing Machine XY...", 2);
	machineZeroXY();
	state = MACHINE_XY_ZERO;
	drawCenteredText("Zero Workspace Z", 2);
	encoder.setClickHandler(onClickZeroWorkspaceZ);
}

void onClickZeroWorkspaceZ(EncoderButton &eb) {
	drawCenteredText("Zeroing Workspace Z...", 2);
	workspaceZeroZ();
	state = WORKSPACE_Z_ZERO;
	setCompassHandler();
	ui.enable();
}

void onClickZeroWorkspaceXY(EncoderButton &eb) {
	drawCenteredText("Zeroing Workspace XY...", 1);
	workspaceZeroXY();
	state = WORKSPACE_XY_ZERO;
}

void onClickSetDoC(EncoderButton &eb) {
	state = DOC_SELECTED;
}

void onClickCalibrationAdvance(EncoderButton &eb) {
	state = CALIBRATION_ADVANCE;
}

void onClickAcceptCalibration(EncoderButton &eb) {
	state = CALIBRATION_ADVANCE;
}

void onClickSetType(EncoderButton &eb) {
	state = TYPE_SELECTED;
}

void onClickMakePath(EncoderButton &eb) {
	if (designType == FROM_FILE) {
		handleFileSelection();
		if (state != DESIGN_SELECTED) {
			updateFileList();
			listFiles();
		}
	} else {
		handleSpeedRun();
		state = DESIGN_SELECTED;
	}
}

void onClickEndScreen(EncoderButton &eb) {
	state = POWER_ON;
}

// ENCODER HANDLERS ----------------------------------------
void onEncoderAcceptCalibration(EncoderButton &eb) {
	acceptCal = (2 + acceptCal + eb.increment()) % 2;
	const char* options[] = {"Exit", "Save!"};
	drawMenu(options, 2, acceptCal);
}

void onEncoderUpdateDesign(EncoderButton &eb) {
	if (designType != PRESET) {
		current_file_idx = (totalFiles + current_file_idx + eb.increment()) % totalFiles;
		Serial.printf("File index: %i\n", current_file_idx);
		listFiles();
	}
}

void onEncoderSetBoost(EncoderButton &eb) {
	float incrScalar = 0.1;
	float tempBoost = feedrateBoost + eb.increment()*incrScalar;

	if (tempBoost >= 0.1 && tempBoost <= 5.0) {
		feedrateBoost = tempBoost;
	}

	Serial.printf("Feedrate boost set to: %.2f\n", feedrateBoost);
}

// FUNCTIONS ------------------------------------------------
void encoderDesignSelect() {
	state = SELECTING_DESIGN;
	encoder.setEncoderHandler(onEncoderUpdateDesign);
	encoder.setClickHandler(onClickMakePath);

	if (designType == FROM_FILE){
		updateFileList();
		listFiles();
	}
	
	closeSDFile();

	while (state != DESIGN_SELECTED) {
		encoder.update();
		if (designType == FROM_FILE) listFiles();
	}

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

void encoderZeroWorkspaceXY() {
	drawCenteredText("Zero workspace XY", 2);
	encoder.setClickHandler(onClickZeroWorkspaceXY);

	while (state != WORKSPACE_XY_ZERO  && state != READY) {
		encoder.update();
	}

	if (!running) {
		// TODO: look into this. Not really sure the use case here. Maybe re-zeroing mid cut?
		// Reset cutting path
		running = true;
		current_point_idx = 0;
	}

	state = READY;
	encoder.setEncoderHandler(nullHandler);
	encoder.setClickHandler(nullHandler);

	screen->fillScreen(BLACK);
	drawFixedUI();
}

void encoderEndScreen() {
	if (designType != SPEED_RUN) {
		drawCenteredText("WOOO nice job!", 2);
	} else {
		char text2send[50];
		sprintf(text2send, "WOW!\nYour time was:\n%.2fs", speedRunTimer/1000.0);
		drawCenteredText(text2send, 2);
		feedrate = feedrate_default;	// reset feedrate to default (NOTE: only RMRRF addition)
	}

	// stay here until encoder is clicked
	encoder.setClickHandler(onClickEndScreen); // prevent accidental state changes
	while (state != POWER_ON) {
		encoder.update();
	}
}

void setCompassHandler() {
	encoder.setClickHandler(onClickCompassUI);
	encoder.setDoubleClickHandler(onDoubleClickCompassUI);
	encoder.setEncoderHandler(onEncoderCompassUI);
	
	//TODO Set handle button to compass UI
}
