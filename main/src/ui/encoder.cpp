#include "encoder.h"
#include "callbacks.h"

void nullHandler(EncoderButton &eb) {
	Serial.println("null handler called");
	return;
}

// CLICK HANDLERS ----------------------------------------
void onClickZeroMachineXY(EncoderButton &eb) {
	ui.drawCenteredText("Zeroing Machine XY...", 2);
	machineZeroXY();
	state = MACHINE_XY_ZERO;
	ui.drawCenteredText("Zero Workspace Z", 2);
	encoder.setClickHandler(onClickZeroWorkspaceZ);
}

void onClickZeroWorkspaceZ(EncoderButton &eb) {
	ui.drawCenteredText("Zeroing Workspace Z...", 2);
	workspaceZeroZ();
	state = WORKSPACE_Z_ZERO;
	setCompassHandler();
	ui.enable();
}

void onClickCalibrationAdvance(EncoderButton &eb) {
	state = CALIBRATION_ADVANCE;
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
void onEncoderUpdateDesign(EncoderButton &eb) {
	if (designType != PRESET) {
		current_file_idx = (totalFiles + current_file_idx + eb.increment()) % totalFiles;
		Serial.printf("File index: %i\n", current_file_idx);
		listFiles();
	}
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

	mainMenuRoot.current = &fileCutMenu;
	mainMenuRoot.focusIndex = 0;
	mainMenuRoot.changed = true;
	setCompassHandler();
	ui.enable();
}

void encoderEndScreen() {
	if (designType != SPEED_RUN) {
		ui.drawCenteredText("WOOO nice job!", 2);
	} else {
		char text2send[50];
		sprintf(text2send, "WOW!\nYour time was:\n%.2fs", speedRunTimer/1000.0);
		ui.drawCenteredText(text2send, 2);
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
	
	//TODO Set handle buttons to compass UI
}
