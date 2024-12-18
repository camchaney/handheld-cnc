#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "globals.h"
#include "motors/motors.h"
#include "sensors/sensors.h"
#include "ui/display.h"
#include "ui/encoder.h"
#include "path/path-generators.h"
#include "math/geometry.h"
#include "io/logging.h"

void handleSerial() {
	if (Serial.available()) {
		char ch = Serial.read();
		if (ch == 'd') {
			debugMode ^= 1;
		} else if (ch == 'p') {
			plotting ^= 1;
		}
	}
}

bool performSafetyChecks() {
	if (digitalRead(LIMIT_MACH_X0) == LOW) {
		stopStepperX();
		stepperZ.moveTo(Conv*restHeight);
		while (stepperZ.distanceToGo() != 0) {
			stepperZ.run();
		}
		Serial.println("X limit reached");
		encoderDesignMode();
		return false;
	}

	if (digitalRead(LIMIT_MACH_Z0) == LOW) {
		stopStepperZ();
		stopStepperX();
		Serial.println("Z limit reached");
		encoderDesignMode();
		return false;
	}

	return true;
}

void handleCutting() {
	// Start of cutting Logic
	Point goal = paths[current_path_idx][current_point_idx];
	Point next = paths[current_path_idx][current_point_idx + 1];

	// If we have not started the path, and the first point is behind us
	// keep the tool raised and return. We wait here until the first point
	// is in front of us and ready to be cut
	if (!path_started && pathDir[current_path_idx] * signedDist(estPos[0], estPos[1], goal.x, goal.y, estYaw) > 0) {
		// Move tool closest to intersect with cutting path
		float desPos = desPosClosestToIntersect(estPos[0], estPos[1], estYaw, goal.x, goal.y, next.x, next.y);
		
		if (outputMode) {
			outputSerial(estPos[0], estPos[1], estYaw, goal, stepperX.currentPosition()*1.0f/Conv, desPos, false);
		}
		outputSD(estPos[0], estPos[1], estYaw, goal, stepperX.currentPosition()*1.0f/Conv, desPos, false);
		
		stepperX.moveTo(Conv*desPos);
		return;
	}

	// If we get here start the path
	// TODO: change w.r.t. naming convention (i.e. cluster, pass, path, point)
	path_started = true;

	// Desired position if we intersect
	float desPos = desPosIntersect(estPos[0], estPos[1], estYaw, goal.x, goal.y, next.x, next.y);
	float desZ = goal.z;
	// Desired position if we do not intersect
	float desPosClosest = desPosClosestToIntersect(estPos[0], estPos[1], estYaw, goal.x, goal.y, next.x, next.y);

	// Conditions for cutting
	// bool handle_buttons_pressed = (digitalRead(BUTT_HANDLE_L) == LOW) && (digitalRead(BUTT_HANDLE_R) == LOW);
	// bool handle_buttons_debounce = (millis() - timeLastDebounce) < debounceDelay;
	// if (handle_buttons_pressed) { timeLastDebounce = millis(); }
	// bool handle_buttons_ok = handle_buttons_pressed || handle_buttons_debounce;
	bool handle_buttons_ok = (digitalRead(BUTT_HANDLE_L) == LOW && digitalRead(BUTT_HANDLE_R) == LOW) || 
						   ((millis() - timeLastDebounce) < debounceDelay);
	if (handle_buttons_ok) timeLastDebounce = millis();

	bool gantry_intersects = !isnan(desPos);
	bool goal_behind_router = pathDir[current_path_idx] * signedDist(estPos[0], estPos[1], goal.x, goal.y, estYaw) > 0;
	bool gantry_angle_ok = angleFrom(goal, next) > (PI / 6);

	// TODO: delete me
  // bool newChecks[4] = {handle_buttons_ok, gantry_intersects, goal_behind_router, gantry_angle_ok};
  // bool logged = false;
  // for (int i=0; i<4; i++) {
  //   if (prevChecks[i] != newChecks[i] && !logged) {
  //     Serial.println("Checks have changed:");
  //     Serial.printf("\thandle_buttons_ok: %d\n", handle_buttons_ok);
  //     Serial.printf("\tgantry_intersects: %d\n", gantry_intersects);
  //     Serial.printf("\tgoal_behind_router: %d\n", goal_behind_router);
  //     Serial.printf("\tgantry_angle_ok: %d\n", gantry_angle_ok);
  //     logged = true;
  //   }
  //   prevChecks[i] = newChecks[i];
  // }

	if (handle_buttons_ok && gantry_intersects && goal_behind_router && gantry_angle_ok && valid_sensors) {
		// Path logging
		if (outputMode) {
			outputSerial(estPos[0], estPos[1], estYaw, goal, stepperX.currentPosition()*1.0f/Conv, desPos, true);
		}
		outputSD(estPos[0], estPos[1], estYaw, goal, stepperX.currentPosition()*1.0f/Conv, desPos, true);

		// We are good to cut
		stepperZ.moveTo(Conv*desZ);
		stepperX.moveTo(Conv*desPos);

		// Move through point indeces as needed
		if (pathDir[current_path_idx] * signedDist(estPos[0], estPos[1], next.x, next.y, estYaw) > 0) {
			// If next point is behind router, it becomes the new goal.
			current_point_idx++;

			// If we're at the end of the points, stop cutting so we can start the next path
			if (current_point_idx == num_points-1) {
				Serial.println("Current path finished");
				stepperZ.moveTo(Conv*restHeight);
				path_started = false;
				current_point_idx = 0;
				current_path_idx++;

				// If we're done all paths then go back to design mode.
				if (current_path_idx == num_paths) {
					// Make sure tool is raised after path is finished
					while (stepperZ.distanceToGo() != 0) {
						stepperZ.run();
					}
					Serial.println("All paths finished");
					encoderDesignMode();
				}
			}
		}
	} else {
		// Path logging
		if (outputMode) {
			outputSerial(estPos[0], estPos[1], estYaw, goal, stepperX.currentPosition()*1.0f/Conv, desPosClosest, false);
		}
		outputSD(estPos[0], estPos[1], estYaw, goal, stepperX.currentPosition()*1.0f/Conv, desPosClosest, false);
		
		// Stop cutting
		stepperZ.moveTo(Conv*restHeight);
		stepperX.moveTo(Conv*desPosClosest);
	}

	// Update UI
	updateUI(desPos, goal, next);
}

void setup() {
	Serial.begin(115200);  

	if (outputMode) { while(!Serial); }
	delay(100);

	if (!screen->begin()) { Serial.println("screen->begin() failed!"); }

	screen->fillScreen(BLACK);

	radius = screen->width()*0.95 / 4;
	centerX = screen->width() / 2;
	centerY = screen->width() / 2;

	drawCenteredText("Initializing...", 1);

	Serial.println("Loading calibration coefficients:");
	readEepromCalibration(cVal);

	Serial.print("Cx values: ");
	for (int j = 0; j < 2; j++) {
		for (int i = 0; i < ns; i++) {
			Serial.print(cVal[j][i], 4);
			if (i < ns - 1) {
				Serial.print(", ");
			}
		}
		Serial.println();
		if (j == 0) {
			Serial.print("Cy values: ");
		}
	}

	// Initialize buttons
	pinMode(LIMIT_MACH_X0, INPUT);
	pinMode(LIMIT_MACH_Z0, INPUT);
	pinMode(BUTT_HANDLE_L, INPUT);
	pinMode(BUTT_HANDLE_R, INPUT);

	// Setup systems
	sensorSetup();
	motorSetup(); 
	driverSetup();
	
	Serial.print("Initializing SD card...");
	if (!sd.begin(SdioConfig(FIFO_SDIO))) {
		Serial.println("Initialization failed!");
		return;
	}
	Serial.println("Initialization done.");

	encoder.setClickHandler(onClickZeroMachineX);
	encoder.setTripleClickHandler(onClickGoToSetThickness);
	onClickResetState(encoder);
}

void loop() {
	// Sensing
	if(micros() - timeLastPoll >= dt) {
		sensingTime = micros() - timeLastPoll;
		doSensing();
	}

	// Run steppers
	stepperX.run();
	stepperZ.run();
	encoder.update();

	handleSerial();

	// --Break here until we are ready to cut--
	if (state != READY) {
		return;
	}

	// Safety stuff
	if (!performSafetyChecks()) {
		return;
	}

	// Debugging
	if (debugMode) {
		debugging();
	}

	handleCutting();
}