#include "encoder.h"

// #define NUM_DESIGNS 		9
#define NUM_DESIGNS 		8


void nullHandler(EncoderButton &eb) {
	Serial.println("null handler called");
	return;
}

// CLICK HANDLERS ----------------------------------------
void onClickGoToDesignMode(EncoderButton &eb) {
	encoderDesignType();
}

// Reset design (from triple click)
void onClickResetState(EncoderButton &eb) {
	state = ZEROED;
	// TODO: reset goal point after triple click

	encoderSetThickness();
}

void onClickZeroMachineXY(EncoderButton &eb) {
	drawCenteredText("Zeroing Machine XY...", 2);
	machineZeroXY();
	state = MACHINE_XY_ZERO;
    StartZeroWorkspace();
}

void StartZeroWorkspace()
{
    drawCenteredText("Zero Workspace Z\nHold L+R for\nauto touch", 2);
    encoder.setClickHandler(onClickZeroWorkspaceZ);
    handleButtons.setButtonsHoldHandler(onHoldAutoTouch);
    handleButtons.enable(true);
}

void onHoldAutoTouch(HandleButtons &btn) {
	drawCenteredText("Auto touch...", 2);
	if (autoTouchWorkspaceZ()) {
		handleButtons.setLeftClickHandler(onClickCancelAutoTouch);
		handleButtons.setRightClickHandler(onClickAcceptAutoTouchH);
		encoder.setClickHandler(onClickAcceptAutoTouchE);
		drawCenteredText("Touched!\nL to cancel\nR to accept", 2);
	} else {
		handleButtons.enable(false);
		handleButtons.clearHandlers();
		StartZeroWorkspace();
	}
}

void onClickZeroWorkspaceZ(EncoderButton &eb) {
	handleButtons.enable(false);
	handleButtons.clearHandlers();
	drawCenteredText("Zeroing Workspace Z...", 2);
	workspaceZeroZ();
	state = WORKSPACE_Z_ZERO;
	encoderSetThickness();
}

void onClickCancelAutoTouch(HandleButtons &btn) {
	handleButtons.enable(false);
	handleButtons.clearHandlers();
	StartZeroWorkspace();
}

void onClickAcceptAutoTouch() {
	handleButtons.enable(false);
	acceptAutoTouchWorkspaceZ();
	state = WORKSPACE_Z_ZERO;
	encoderSetThickness();
}

void onClickAcceptAutoTouchE(EncoderButton &eb) {
	onClickAcceptAutoTouch();
}

void onClickAcceptAutoTouchH(HandleButtons &btn) {
	onClickAcceptAutoTouch();
}

void onClickZeroWorkspaceXY(EncoderButton &eb) {
	drawCenteredText("Zeroing Workspace XY...", 1);
	workspaceZeroXY();
	state = WORKSPACE_XY_ZERO;
}

void onClickSetThickness(EncoderButton &eb) {
	state = THICKNESS_SET;
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
	if (designType == PRESET) {
		makePresetPath();
		state = DESIGN_SELECTED;
	} else if (designType == FROM_FILE) {
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

void onClickPauseCut(EncoderButton &eb) {
	if (cutState == CUTTING) {
		cutState = NOT_USER_READY;
	} else if (cutState == NOT_USER_READY) {
		cutState = CUT_READY;
	}

	state = STANDBY;
	encoderHandlePause();
}

void onClickPauseSelect(EncoderButton &eb) {
	if (pauseSelection == 0) {
		// Re-zero XY
		workspaceZeroXY();
		state = WORKSPACE_XY_ZERO;
	} else {
		// Go to main menu
		state = ZEROED;
		encoderSetThickness();
	}
}

void onClickEndScreen(EncoderButton &eb) {
	state = POWER_ON;
}

// ENCODER HANDLERS ----------------------------------------
void onEncoderUpdateThickness(EncoderButton &eb) {
	float incrScalar = 0.1;
	float tempThickness = matThickness + eb.increment()*incrScalar;

	if (tempThickness <= maxThickness && tempThickness >= 0.0) {
		matThickness = tempThickness;
	}
	
	char text2send[50];
	sprintf(text2send, "Turn to\nset thickness\n%.2f mm", matThickness);
	drawCenteredText(text2send, 2);
}

void onEncoderDesignOrCalibrate(EncoderButton &eb) {
	designOrCalibrate = (2 + designOrCalibrate + eb.increment()) % 2;
	const char* options[] = {"Cut Design!", "Calibrate"};
	drawMenu(options, 2, designOrCalibrate);
}

void onEncoderAcceptCalibration(EncoderButton &eb) {
	acceptCal = (2 + acceptCal + eb.increment()) % 2;
	const char* options[] = {"Exit", "Save!"};
	drawMenu(options, 2, acceptCal);
}

void onEncoderSwitchType(EncoderButton &eb) {
	designType = (DesignType)((3 + designType + eb.increment()) % 3);
	const char* options[] = {"Preset", "From File", "Speed Run"};
	drawMenu(options, 3, designType);
}

void onEncoderUpdateDesign(EncoderButton &eb) {
	if (designType == PRESET) {
		designPreset = (NUM_DESIGNS + designPreset + eb.increment()) % NUM_DESIGNS;
		drawShape();
	} else {
		current_file_idx = (totalFiles + current_file_idx + eb.increment()) % totalFiles;
		Serial.printf("File index: %i\n", current_file_idx);
		listFiles();
	}

}

void onEncoderPauseMenu(EncoderButton &eb) {
	designOrCalibrate = (2 + designOrCalibrate + eb.increment()) % 2;
	const char* options[] = {"Re-zero XY", "Main Menu"};
	drawMenu(options, 2, designOrCalibrate);
}

void onEncoderSetSpeed(EncoderButton &eb) {
	float maxSpeed = 0.5 * maxSpeedAB / ConvBelt;
	float incrScalar = 1.0;
	float tempSpeed = feedrate + eb.increment()*incrScalar;

	if (tempSpeed <= maxSpeed && tempSpeed >= 1.0) {
		feedrate = tempSpeed;
	}
	
	if (state != READY) {
		char text2send[50];
		sprintf(text2send, "Turn to\nset speed\n%.2f mm/s", feedrate);
		drawCenteredText(text2send, 2);
	} else {
		Serial.printf("Feedrate set to: %.2f mm/s\n", feedrate);
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
void encoderSetThickness() {
	char text2send[50];
	sprintf(text2send, "Turn to\nset thickness\n%.2f mm", matThickness);
	drawCenteredText(text2send, 2);

	encoder.setEncoderHandler(onEncoderUpdateThickness);
	encoder.setClickHandler(onClickSetThickness);

	while (state != THICKNESS_SET  && state != READY) {
		encoder.update();
	}

	encoderDesignOrCalibrate();
}

void encoderDesignOrCalibrate() {
	const char* options[] = {"Cut Design!", "Calibrate"};
	drawMenu(options, 2, designOrCalibrate);

	encoder.setEncoderHandler(onEncoderDesignOrCalibrate);
	encoder.setClickHandler(onClickSetDoC);

	while (state != DOC_SELECTED  && state != READY) {
		encoder.update();
	}

	if (designOrCalibrate == 0) {
		encoderDesignType();
	} else {
		calibrate();
	}
}

void encoderDesignType() {
	const char* options[] = {"Preset", "From File", "Speed Run"};
	drawMenu(options, 3, designType);

	encoder.setEncoderHandler(onEncoderSwitchType);
	encoder.setClickHandler(onClickSetType);

	while (state != TYPE_SELECTED  && state != READY) {
		encoder.update();
	}
		
	encoderDesignSelect();
}

void encoderDesignSelect() {
	state = SELECTING_DESIGN;
	encoder.setEncoderHandler(onEncoderUpdateDesign);
	encoder.setClickHandler(onClickMakePath);

	if (designType == PRESET) {
		drawShape();
	} else if (designType == FROM_FILE){
		updateFileList();
		listFiles();
	} else {
		char text2send[50];
		sprintf(text2send, "Turn to\nset speed\n%.2f mm/s", feedrate);
		drawCenteredText(text2send, 2);

		encoder.setEncoderHandler(onEncoderSetSpeed);
	}
	
	closeSDFile();

	while (state != DESIGN_SELECTED) {
		encoder.update();
		if (designType == FROM_FILE) listFiles();
	}

	// Hack for opensauce, auto-zero XY
	// TODO: remove this
	workspaceZeroXY();

	// Reset cutting path
	running = true;
	current_point_idx = 0;
	if (designType != SPEED_RUN) feedrate = feedrate_default;	// reset feedrate to default (NOTE: only RMRRF addition)
	feedrateBoost = 1.0;	// reset feedrate boost to default
	speedRunTimer = 0;

	state = READY;
	cutState = NOT_CUT_READY;
	encoder.setEncoderHandler(onEncoderSetBoost);
	encoder.setClickHandler(nullHandler);

	screen->fillScreen(BLACK);
	drawFixedUI();

	// Clear out sensors in case we moved while in design mode
	for (int i = 0; i < 4; i++) {
		sensors[i].readBurst();
	}
}

void encoderHandlePause() {
	const char* options[] = {"Re-zero XY", "Main Menu"};
	drawMenu(options, 2, designOrCalibrate);

	encoder.setEncoderHandler(onEncoderPauseMenu);
	encoder.setClickHandler(onClickPauseSelect);

	while (state != DOC_SELECTED) {
		encoder.update();
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
