#include "path-execution.h"

Point goal;
Position desPos;
bool prevChecks[4] = {false};

TrajectoryGenerator trajectory;
ActuationController actuator(desPos);

bool checkEndstops() {
	if (digitalRead(LIMIT_MACH_X0) == LOW) {
		stopStepperX();
		stepperZ.moveTo(ConvLead*restHeight);
		while (stepperZ.distanceToGo() != 0) {
			stepperZ.run();
		}
		Serial.println("X limit reached");
		encoderDesignType();
		return false;
	}

	if (digitalRead(LIMIT_MACH_Z0) == LOW) {
		stopStepperZ();
		stopStepperX();
		Serial.println("Z limit reached");
		encoderDesignType();
		return false;
	}

	return true;
}

void handleChickenHead() {
	// If the chicken head is pressed...lol jk
	// Using the pose information, I want to move the router so that it is always kept at (0,0,0)
	float xDes = -(pose.x*cosf(pose.yaw)+pose.y*sinf(pose.yaw));
	float yDes = pose.x*sinf(pose.yaw)-pose.y*cosf(pose.yaw);

	Serial.println("Chicken");

	desPos.set(xDes, yDes, 0.0f);
	cartesianToMotor(desPos);
}

void handleCutting(long deltaTime) {
	// Start of cutting Logic
	trajectory.update(deltaTime, goal);			// update goal point
	if (matThickness == 0.0 && designType == FROM_FILE && goal.z < 0.0) {
		// if matThickness is set to 0 (drawing), then don't pierce!
		goal.z = 0.0f;
	}

	// Update the desired position (local actuator frame)
	actuator.update(deltaTime, goal, pose);	

	// Conditions for cutting
	bool handle_buttons_pressed = (digitalRead(BUTT_HANDLE_L) == LOW) && (digitalRead(BUTT_HANDLE_R) == LOW);
	bool handle_buttons_debounce = (millis() - timeLastDebounce) < debounceDelay;
	if (handle_buttons_pressed) { timeLastDebounce = millis(); }
	bool handle_buttons_ok = handle_buttons_pressed || handle_buttons_debounce;
	// bool handle_buttons_ok = (digitalRead(BUTT_HANDLE_L) == LOW && digitalRead(BUTT_HANDLE_R) == LOW) || 
	// 					   ((millis() - timeLastDebounce) < debounceDelay);
	// if (handle_buttons_ok) timeLastDebounce = millis();
	// bool within_hole_tol = abs(signedDist(pose, goal)) < holeTolerance;

	// TODO: delete me
	bool newChecks[3] = {handle_buttons_ok, valid_sensors, actuator.validMotion};
	bool logged = false;
	for (int i=0; i<3; i++) {
		if (prevChecks[i] != newChecks[i] && !logged) {
		Serial.printf("Checks have changed t=%d:", millis());
		Serial.printf("\thandle_buttons_ok: %d\n", handle_buttons_ok);
		Serial.printf("\tvalid_sensors: %d\n", valid_sensors);
		Serial.printf("\tactuator.validMotion: %d\n", actuator.validMotion);
		logged = true;
		}
		prevChecks[i] = newChecks[i];
	}

	// TODO: handle valid_sensors better (want to prompt re-zeroing if sensors are bad)
	// if (handle_buttons_ok && valid_sensors && path.points[current_point_idx].feature == NORMAL) {
	if (!actuator.validMotion) {
		// Out of bounds...
		running = false;
		cutState = NOT_CUT_READY;

		stepperZ.moveTo(ConvLead*restHeight);
	} else if (!handle_buttons_ok && valid_sensors) {
		// User not ready...
		running = false;
		cutState = NOT_USER_READY;

		cartesianToMotor(desPos);
		stepperZ.moveTo(ConvLead*restHeight);
	} else if (valid_sensors){
		// All good to go!
		running = true;
		cutState = CUTTING;
		
		cartesianToMotor(desPos);
	} else {
		// ...catch all
		running = false;
		cutState = NOT_CUT_READY;

		stepperZ.moveTo(ConvLead*restHeight);
	}

	// Update UI
	updateUI(desPos, (float)current_point_idx/(float)path.numPoints);

	// Path logging
	// TODO: make this more clean (without pass by reference)
	float toolX, toolY, toolZ;
	motorToCartesian(toolX, toolY, toolZ);
	if (outputSDOn)	writeAuxData(goal, toolX, toolY, toolZ, desPos);

	// Debugging
	if (debuggingOn) debugging(goal, desPos);
}