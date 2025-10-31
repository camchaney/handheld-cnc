#include "motors.h"
#include <TimerOne.h>

float maxHeight = 0.0f;
volatile bool zZeroed = false;
volatile long zStepCount = 0;
volatile long zStepTarget = 0;
volatile bool zStepActive = false;

void motorSetup() {
	// Set up motors
	pinMode(MOT_EN_R, OUTPUT);
	pinMode(MOT_EN_L, OUTPUT);
	pinMode(MOT_EN_Z, OUTPUT);

	// Enable motors
	digitalWrite(MOT_EN_R, LOW);
	digitalWrite(MOT_EN_L, LOW);
	digitalWrite(MOT_EN_Z, LOW);

	delay(100);

	disableStepperZ();
	disableStepperRL();

	// Set motor properties
	stepperR.setMinPulseWidth(stepPulseWidth);
	stepperR.setMaxSpeed(zeroSpeed_0 * ConvBelt);
	stepperR.setAcceleration(zeroAccel * ConvBelt);
	stepperR.setCurrentPosition(0);

	stepperL.setMinPulseWidth(stepPulseWidth);
	stepperL.setMaxSpeed(zeroSpeed_0 * ConvBelt);
	stepperL.setAcceleration(zeroAccel * ConvBelt);
	stepperL.setCurrentPosition(0);

	stepperZ.setMinPulseWidth(stepPulseWidth);
	stepperZ.setMaxSpeed(zeroSpeed_0 * ConvLead);
	stepperZ.setAcceleration(zeroAccel * ConvLead);
	stepperZ.setCurrentPosition(0);
	stepperZ.setPinsInverted(true, false, false);		// Invert Z motor direction
}

void driverSetup() {
	SERIAL_PORT.begin(115200);
	delay(100);

	driverR.begin(); driverR.toff(5);
	driverL.begin(); driverL.toff(5);
	driverZ.begin(); driverZ.toff(5);

	driverR.rms_current(maxCurrent_RMS);
	driverL.rms_current(maxCurrent_RMS);
	driverZ.rms_current(maxCurrent_RMS);
	
	driverR.microsteps(uSteps);
	driverL.microsteps(uSteps);
	driverZ.microsteps(uSteps);

	driverR.pwm_autoscale(true);
	driverR.en_spreadCycle(false);
	driverR.TPWMTHRS(0x753);

	driverL.pwm_autoscale(true);
	driverL.en_spreadCycle(false);
	driverL.TPWMTHRS(0x753);
	
	driverZ.pwm_autoscale(true);
	driverZ.en_spreadCycle(false);
	driverZ.TPWMTHRS(0x753);
}

void enableStepperZ() {
	digitalWrite(MOT_EN_Z, LOW);
}

void enableStepperRL() {
	digitalWrite(MOT_EN_R, LOW);
	digitalWrite(MOT_EN_L, LOW);
}

void disableStepperZ() {
	digitalWrite(MOT_EN_Z, HIGH);
}

void disableStepperRL() {
	digitalWrite(MOT_EN_R, HIGH);
	digitalWrite(MOT_EN_L, HIGH);
}

void stopStepperX() {
	stepperR.setSpeed(0);
	stepperL.runSpeed();
}

void stopStepperZ() {
	stepperZ.setSpeed(0);
	stepperZ.runSpeed();
}

void machineZeroXY() {
	// Do the homing cycles for the XY axes
	// The X axis limit switch is on the left side of the machine (-X)
	// The Y axis limit switch is on the back of the machine (+Y)
	// TODO: add mask property for different limit switch configurations
	int limitDirectionX = -1;
	int limitDirectionY = 1;

	// Enable motors
	enableStepperRL();
	// stepperR.setCurrentPosition(0);			// NOTE: for calculating xOffset
	// stepperL.setCurrentPosition(0);

	// X axis homing (negative direction)
	stepperR.setSpeed(zeroSpeed_0 * ConvBelt * limitDirectionX);
	stepperL.setSpeed(zeroSpeed_0 * ConvBelt * limitDirectionX);
	while (digitalRead(LIMIT_MACH_X0) == HIGH) {
		stepperR.runSpeed();
		stepperL.runSpeed();
	}

	stepperR.move(-retract * ConvBelt * limitDirectionX);
	stepperL.move(-retract * ConvBelt * limitDirectionX);
	while (stepperR.distanceToGo() != 0 && stepperL.distanceToGo() != 0) {
		if (stepperR.distanceToGo() != 0) stepperR.run();
		if (stepperL.distanceToGo() != 0) stepperL.run();
	}

	stepperR.setSpeed(zeroSpeed_1 * ConvBelt * limitDirectionX);
	stepperL.setSpeed(zeroSpeed_1 * ConvBelt * limitDirectionX);
	while (digitalRead(LIMIT_MACH_X0) == HIGH) {
		stepperR.runSpeed();
		stepperL.runSpeed();
	}

	stepperR.setMaxSpeed(maxSpeedAB/2);
	stepperL.setMaxSpeed(maxSpeedAB/2);
	stepperR.setAcceleration(maxAccelAB/2);
	stepperL.setAcceleration(maxAccelAB/2);

	// float currentPos = stepperR.currentPosition() * 1.0f / ConvBelt;		// NOTE: for calculating xOffset
	// Serial.printf("Current X position: %.2f\n", currentPos);
	
	stepperR.move(((xRangeHard/2) - xLimitOffset) * ConvBelt * (-limitDirectionX));
	stepperL.move(((xRangeHard/2) - xLimitOffset) * ConvBelt * (-limitDirectionX));
	while (stepperR.distanceToGo() != 0 && stepperL.distanceToGo() != 0) {
		if (stepperR.distanceToGo() != 0) stepperR.run();
		if (stepperL.distanceToGo() != 0) stepperL.run();
	}

	// Y axis homing (positive direction)
	// stepperR.setCurrentPosition(0);			// NOTE: for calculating yOffset
	// stepperL.setCurrentPosition(0);

	stepperR.setSpeed(zeroSpeed_0 * ConvBelt * limitDirectionY);
	stepperL.setSpeed(-zeroSpeed_0 * ConvBelt * limitDirectionY);
	while (digitalRead(LIMIT_MACH_Y0) == HIGH) {
		stepperR.runSpeed();
		stepperL.runSpeed();
	}

	stepperR.move(-retract * ConvBelt * limitDirectionY);
	stepperL.move(retract * ConvBelt * limitDirectionY);
	while (stepperR.distanceToGo() != 0 && stepperL.distanceToGo() != 0) {
		if (stepperR.distanceToGo() != 0) stepperR.run();
		if (stepperL.distanceToGo() != 0) stepperL.run();
	}

	stepperR.setSpeed(zeroSpeed_1 * ConvBelt * limitDirectionY);
	stepperL.setSpeed(-zeroSpeed_1 * ConvBelt * limitDirectionY);
	while (digitalRead(LIMIT_MACH_Y0) == HIGH) {
		stepperR.runSpeed();
		stepperL.runSpeed();
	}

	stepperR.setMaxSpeed(maxSpeedAB/2);
	stepperL.setMaxSpeed(maxSpeedAB/2);
	stepperR.setAcceleration(maxAccelAB/2);
	stepperL.setAcceleration(maxAccelAB/2);

	// float currentPos = stepperR.currentPosition() * 1.0f / ConvBelt;		// NOTE: for calculating yOffset
	// Serial.printf("Current X position: %.2f\n", currentPos);
	
	stepperR.move(((yRangeHard/2) - yLimitOffset) * ConvBelt * (-limitDirectionY));
	stepperL.move(-((yRangeHard/2) - yLimitOffset) * ConvBelt * (-limitDirectionY));
	while (stepperR.distanceToGo() != 0 && stepperL.distanceToGo() != 0) {
		if (stepperR.distanceToGo() != 0) stepperR.run();
		if (stepperL.distanceToGo() != 0) stepperL.run();
	}

	// Reset stepper configurations and zero position
	stepperR.setMaxSpeed(maxSpeedAB);
	stepperL.setMaxSpeed(maxSpeedAB);
	stepperR.setAcceleration(maxAccelAB);
	stepperL.setAcceleration(maxAccelAB);
	stepperR.setCurrentPosition(0);
	stepperL.setCurrentPosition(0);

	pose = {0.0f};
}

void workspaceZeroZ() {
	// Manually zero the Z axis to the workpiece using the knob

	enableStepperZ();
	stepperZ.setCurrentPosition(0);

	stepperZ.setSpeed(zeroSpeed_0 * ConvLead);
	while (digitalRead(LIMIT_MACH_Z0) == HIGH) {
		stepperZ.runSpeed();
	}

	stepperZ.move(-retract * ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	stepperZ.setSpeed(zeroSpeed_1 * ConvLead);
	while (digitalRead(LIMIT_MACH_Z0) == HIGH) {
		stepperZ.runSpeed();
	}

	maxHeight = stepperZ.currentPosition()*1.0f / ConvLead;
	// Serial.printf("Current Z position: %.2f\n", maxHeight);

	stepperZ.setMaxSpeed(maxSpeedZ/2);
	stepperZ.moveTo(restHeight * ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	stepperZ.setMaxSpeed(maxSpeedZ);
	stepperZ.setAcceleration(maxAccelZ);
}

bool autoTouchWorkspaceZ() {
	enableStepperZ();

	// Move to limit switch
	stepperZ.setSpeed(zeroSpeed_0 * ConvLead);
	while (digitalRead(LIMIT_MACH_Z0) == HIGH) {
		stepperZ.runSpeed();
	}

	stepperZ.move(-retract * ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	stepperZ.setSpeed(zeroSpeed_1 * ConvLead);
	while (digitalRead(LIMIT_MACH_Z0) == HIGH) {
		stepperZ.runSpeed();
	}

	stepperZ.move(-retract * ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	// Set up auto touch
	driverZ.rms_current(maxAutoTouchCurrent_RMS);
	driverZ.TCOOLTHRS(0xFFFFF);
	driverZ.TPWMTHRS(0);

	long maxSteps = zRange * ConvLead;
	zZeroed = false;
	zStepCount = 0;
	zStepTarget = maxSteps;
	zStepActive = true;
	digitalWrite(MOT_DIR_Z, HIGH);
	Timer1.initialize(autoTouchStepInterval);
	Timer1.attachInterrupt(zStepISR);

	uint32_t last_time = millis();

	while (zStepActive) {
		uint32_t ms = millis();
		if((ms-last_time) > 100) { //run every 0.1s
			last_time = ms;

			uint16_t sg = driverZ.SG_RESULT();
			Serial.print("SG_RESULT: ");
			Serial.println(sg, DEC);
			if (sg < autoTouchThreshold) {
				Serial.println("Touched!");
				zZeroed = true;
				zStepActive = false;
				break;
			}
		}

		if ((digitalRead(BUTT_HANDLE_L) != LOW) || (digitalRead(BUTT_HANDLE_R) != LOW)) {
			zZeroed = false;
			zStepActive = false;
			break;
		}

		delay(1); // Allow time for ISR and checks
	}

	// TODO: do another auto touch at slower speed for more precision

	Timer1.detachInterrupt();
	delay(1); // Allow time for the last step to complete
	Serial.println("Compensate backslash...");
	stepperZ.move(autoTouchRetraction* ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	// restore defaults
	driverZ.rms_current(maxCurrent_RMS);
	driverZ.TCOOLTHRS(0);
	driverZ.TPWMTHRS(0x753);

	return zZeroed;
}

void acceptAutoTouchWorkspaceZ() {
	maxHeight = (zStepCount * 1.0f / ConvLead) - autoTouchRetraction;
	stepperZ.setCurrentPosition(0);

	stepperZ.setMaxSpeed(maxSpeedZ/2);
	stepperZ.moveTo(restHeight * ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	stepperZ.setMaxSpeed(maxSpeedZ);
	stepperZ.setAcceleration(maxAccelZ);
}

void workspaceZeroXY() {
	Position desPos;
	desPos.set(0.0f, 0.0f, 0.0f);
	cartesianToMotor(desPos);

	while (stepperR.distanceToGo() != 0 && stepperL.distanceToGo() != 0) {
		stepperR.run();
		stepperL.run();
	}

	stepperZ.moveTo(restHeight * ConvLead);
	while (stepperZ.distanceToGo() != 0) {
		stepperZ.run();
	}

	// Reset router pose
	pose = {0.0f};
}

void plungeZ(float zPos) {
	if (running == false) {
		if (stepperZ.distanceToGo() != 0) {
			stepperZ.setMaxSpeed(plungeRate_default * ConvLead);
		} else {
			// If the Z stepper is at the desired position, then start cutting
			stepperZ.setMaxSpeed(maxSpeedZ);
			running = true;
			cutState = CUTTING;
		}
	}
}

// Convert the position to motor coordinates, using the coreXY system
// Inputs:
// 	- pos - the position in the router's BFF coordinate system (mm)
void cartesianToMotor(Position pos) {
	// actuate coreXY system
	float a = pos.getX() + pos.getY();
	float b = pos.getX() - pos.getY();
	stepperR.moveTo(a * ConvBelt);
	stepperL.moveTo(b * ConvBelt);
}

// Convert the motor position to the router's BFF coordinate system
void motorToCartesian(float &x, float &y, float &z) {
	float a = stepperR.currentPosition() * 1.0f / ConvBelt;
	float b = stepperL.currentPosition() * 1.0f / ConvBelt;

	x = (a + b) / 2;
	y = (a - b) / 2;
	z = stepperZ.currentPosition() * 1.0f / ConvLead;
}

void zStepISR() {
	if (!zStepActive) return;
	if (zStepCount >= zStepTarget) {
		zStepActive = false;
		digitalWrite(MOT_STEP_Z, LOW);
		Timer1.detachInterrupt();
		return;
	}
	// Generate one step pulse
	digitalWrite(MOT_STEP_Z, HIGH);
	delayMicroseconds(stepPulseWidth);
	digitalWrite(MOT_STEP_Z, LOW);
	zStepCount++;
}