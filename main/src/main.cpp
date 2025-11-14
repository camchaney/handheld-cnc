#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "globals.h"
#include "actuation/motors.h"
#include "sensors/sensors.h"
#include "ui/display.h"
#include "ui/encoder.h"
#include "path/path-generators.h"
#include "path/path-execution.h"
#include "math/geometry.h"
#include "io/logging.h"

void setup() {
	Serial.begin(115200);  

	if (outputSerialOn) { while(!Serial); }
	delay(100);

	if (!screen->begin()) { Serial.println("screen->begin() failed!"); }

	screen->fillScreen(BLACK);

	drawCenteredText("Initializing...", 2);
	delay(200);

	Serial.println("Loading calibration coefficients:");
	readEepromCalibration();

	for (int i = 0; i < ns; i++) {
		Serial.printf("Sensor %i:\tCx:%.4f, Cy:%.4f, Cr:%.4f\n", i, cal[i].x, cal[i].y, cal[i].r);
	}

	// Initialize buttons
	pinMode(LIMIT_MACH_X0, INPUT);
	pinMode(LIMIT_MACH_Z0, INPUT);
	pinMode(BUTT_HANDLE_L, INPUT);
	pinMode(BUTT_HANDLE_R, INPUT);

	// Set up systems
	sensorSetup();
	motorSetup(); 
	driverSetup();
	
	Serial.print("Initializing SD card...");
	if (!sd.begin(SdioConfig(FIFO_SDIO))) {
		Serial.println("Initialization failed!");
		outputSDOn = false;
	} else {
		Serial.println("Initialization done.");
	}

	drawCenteredText("Zero Machine XY", 2);
	encoder.setClickHandler(onClickZeroMachineXY);
	encoder.setTripleClickHandler(onClickResetState);
}

void loop() {
	RouterPose pose_backup;
	float distanceTraveled_backup;
	bool valid_sensors_backup;

	// Get sensor and pose information from interrupts
	noInterrupts();
	memcpy(&pose_backup, (const void*)&pose_isr, sizeof(pose_backup));
	distanceTraveled_backup = distanceTraveled_isr;
	valid_sensors_backup = valid_sensors_isr;
	interrupts();

	// Run motors and update inputs
	stepperR.run();
	stepperL.run();
	stepperZ.run();
	encoder.update();
	handleButtons.update();

	// Serial handling
	handleSerial();

	// --Break here until we are ready to cut--
	if (state != READY) {
		totalLoopTime = micros() - timeLoopStart;
		return;
	}

	// Safety stuff
	if (!checkEndstops()) {
		// safetyTime = micros() - startSafetyTime;
		// totalLoopTime = micros() - timeLoopStart;
		return;
	}

	// Cutting
	// handleChickenHead();
	if (runTimer >= dtControl) {
		// TODO: tighten this control loop
		runTimer = 0;
		handleCutting(pose_backup, dtControl, distanceTraveled_backup, valid_sensors_backup);		// TODO: make deltaTime a variable
	}

	// Debugging
	if (stopwatchOn) {
		stopwatch();
	}
}