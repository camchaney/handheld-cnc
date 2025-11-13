#include "sensors.h"

/*
Sensor configuration:
0 --- 1    ^ y
|  x  |    |-> x
2 --- 3

Angle signage: +CCW
*/

// Timing variables
static long unsigned timeLastPlot = 0;

// Additional tracking variables
static int surfaceQuality[4] = {0,0,0,0};
static bool onSurface[4] = {1,1,1,1};		// 1 if sensor is on surface, 0 otherwise
static float estVel[2][4] = {{0.0f,0.0f,0.0f,0.0f},
							{0.0f,0.0f,0.0f,0.0f}};
static float estAngVel[8] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};

void sensorSetup() {
	int sensorPins[4] = {SS0, SS1, SS2, SS3};

	screen->fillScreen(BLACK);
	int size = 2;
	int16_t centerX = screen->width() / 2;
	int16_t centerY = screen->height() / 2;
	int16_t totalHeight = 4 * size * 10;			// 4 lines, size 2, 10px per line
	int16_t yStart = centerY - totalHeight / 2;
	int16_t x1, y1;
	uint16_t w, h;
	char text[10];

	sprintf(text, "v%s", FIRMWARE_VERSION);
	screen->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
	int16_t xStart = centerX - w / 2;
	screen->setCursor(xStart, yStart - size *10);
	screen->println(text);

	for (int i = 0; i < 4; i++) {
		// display each sensors' initialization status on the screen, showing each sensor on a new line
		// and setting text color green or red depending on initialization status
		Serial.print("Sensor");
		Serial.print(i);

		if (sensors[i].begin(sensorPins[i], CPI)) {
			screen->setTextColor(GREEN);

			Serial.print(" initialization succeeded, with CPI = ");
			Serial.println(sensors[i].getCPI());
		} else {
			screen->setTextColor(RED);

			Serial.println(" initialization failed");
		}

		sprintf(text, "Sensor %i", i);
		screen->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
		xStart = centerX - w / 2;
		screen->setCursor(xStart, yStart + i * size * 10);
		screen->println(text);
	}
}

int16_t convTwosComp(int16_t value) {
	if (value & 0x8000) {
		return -((~value & 0xFFFF) + 1);
	} else {
		return value;
	}
}

void setupSensorTimer() {
	sensorTimer.begin(sensorTimerISR, dt);
	sensorTimer.priority(128);
}

void sensorTimerISR() {
	if (sensorDataReady) {
		return;
	}
	
	sensorStartTime = micros();
	readAllSensors();
	sensorDataReady = true;
}

void readAllSensors() {
	for (int i = 0; i < ns; i++) {
		motionData[i] = sensors[i].readBurst();
	}
}

void doSensing() {
	if (!sensorDataReady) return;
	
	sensorDataReady = false;
	sensingTime = micros() - sensorStartTime;
	unsigned long logTime = filemicros;
	
	SensorData logData[ns];
	
	for (int i = 0; i < ns; i++) {
		float dx = -convTwosComp(motionData[i].dx);
		float dy = convTwosComp(motionData[i].dy);
		surfaceQuality[i] = motionData[i].SQUAL;
		onSurface[i] = motionData[i].isOnSurface;
		
		if (onSurface[i]) {
			float sinfr = sinf(cal[i].r);
			float cosfr = cosf(cal[i].r);
			measVel[0][i] = cal[i].x * (dx*cosfr - dy*sinfr) / sensingTime;
			measVel[1][i] = cal[i].y * (dx*sinfr + dy*cosfr) / sensingTime;
		} else {
			measVel[0][i] = NAN;
			measVel[1][i] = NAN;
		}
		
		logData[i].dx = dx;
		logData[i].dy = dy;
		logData[i].onSurface = motionData[i].isOnSurface;
		logData[i].sq = motionData[i].SQUAL;
		logData[i].rawDataSum = motionData[i].rawDataSum;
	}
	
	estAngVel[0] = (measVel[0][2] - measVel[0][0])/ly;
	estAngVel[1] = (measVel[0][2] - measVel[0][1])/ly;
	estAngVel[2] = (measVel[0][3] - measVel[0][0])/ly;
	estAngVel[3] = (measVel[0][3] - measVel[0][1])/ly;
	estAngVel[4] = (measVel[1][1] - measVel[1][0])/lx;
	estAngVel[5] = (measVel[1][1] - measVel[1][2])/lx;
	estAngVel[6] = (measVel[1][3] - measVel[1][0])/lx;
	estAngVel[7] = (measVel[1][3] - measVel[1][2])/lx;
	
	float sumAngVel = 0.0f;
	float estAngVel1 = 0.0f;
	int num_good_calcs = 0;
	for (int i = 0; i < 8; i++) {
		if (!isnan(estAngVel[i])) {
			sumAngVel = sumAngVel + estAngVel[i];
			num_good_calcs++;
		}
	}
	
	if (num_good_calcs == 0) {
		valid_sensors = false;
		return;
	} else {
		valid_sensors = true;
		estAngVel1 = sumAngVel / num_good_calcs;
	}
	
	float sinfy = sinf(pose.yaw);
	float cosfy = cosf(pose.yaw);
	estVel[0][0] = measVel[0][0]*cosfy-measVel[1][0]*sinfy + 0.5*estAngVel1*(lx*cosfy-(ly)*sinfy);
	estVel[0][1] = measVel[0][1]*cosfy-measVel[1][1]*sinfy + 0.5*estAngVel1*(lx*cosfy+(ly)*sinfy);
	estVel[0][2] = measVel[0][2]*cosfy-measVel[1][2]*sinfy + 0.5*estAngVel1*(-lx*cosfy-(ly)*sinfy);
	estVel[0][3] = measVel[0][3]*cosfy-measVel[1][3]*sinfy + 0.5*estAngVel1*(-lx*cosfy+(ly)*sinfy);
	estVel[1][0] = measVel[0][0]*sinfy+measVel[1][0]*cosfy + 0.5*estAngVel1*((ly)*cosfy+lx*sinfy);
	estVel[1][1] = measVel[0][1]*sinfy+measVel[1][1]*cosfy + 0.5*estAngVel1*(-(ly)*cosfy+lx*sinfy);
	estVel[1][2] = measVel[0][2]*sinfy+measVel[1][2]*cosfy + 0.5*estAngVel1*((ly)*cosfy-lx*sinfy);
	estVel[1][3] = measVel[0][3]*sinfy+measVel[1][3]*cosfy + 0.5*estAngVel1*(-(ly)*cosfy-lx*sinfy);
	
	float sumVelX = 0.0f;
	float sumVelY = 0.0f;
	float estVel1[2] = {0.0f, 0.0f};
	num_good_calcs = 0;
	for (int i = 0; i < ns; i++) {
		if (!isnan(estVel[0][i]) && !isnan(estVel[1][i])) {
			sumVelX = sumVelX + estVel[0][i];
			sumVelY = sumVelY + estVel[1][i];
			num_good_calcs++;
		}
	}
	
	if (num_good_calcs == 0) {
		valid_sensors = false;
		return;
	} else {
		valid_sensors = true;
		estVel1[0] = sumVelX / num_good_calcs;
		estVel1[1] = sumVelY / num_good_calcs;
	}
	
	pose.yaw = pose.yaw + estAngVel1*sensingTime;
	pose.x = pose.x + estVel1[0]*sensingTime;
	pose.y = pose.y + estVel1[1]*sensingTime;
	
	float distRatio = 0.6f;
	dXY = distRatio*dXY + (1-distRatio)*sqrtf(estVel1[0]*estVel1[0] + estVel1[1]*estVel1[1]) * sensingTime;
	distanceTraveled += dXY;
	
	if (outputSDOn && logFile) {
		writeSensorData(logTime, logData, sensingTime);
	}
}

void doSensingLinear() {
    timeLastPoll = micros();

    // Sensing ---------------------------------------------------------------------
    // Collect sensor data (raw)
 
    PMW3360_DATA data[4];
	// TODO: SensorData logData[ns];
    for (int i = 0; i < ns; i++) {
      data[i] = sensors[i].readBurst();
      surfaceQuality[i] = data[i].SQUAL;
    }

    for (int i = 0; i < ns; i++) {
      // Sensor velocity sensing
      // measVel[0][i] = -convTwosComp(data[i].dx);     // '-' convention is used to flip sensor's z axis
      measVel[0][i] = -convTwosComp(data[i].dx);
      measVel[1][i] = convTwosComp(data[i].dy);

      // Integrate linear velocities to get position
      calPos[0][i] = calPos[0][i] + measVel[0][i];
      calPos[1][i] = calPos[1][i] + measVel[1][i];
    }

    // Sensor plotting
	for (int i = 0; i < ns; i++) {
		float angle = 0.0f;
		if (abs(calPos[0][i]) > abs(calPos[1][i])) {
			angle = atan2f(-calPos[1][i],calPos[0][i]);
		} else {
			angle = atan2f(calPos[0][i],calPos[1][i]);
		}
		Serial.printf("%i: x:%.0f,\ty:%.0f,\tb:%.6f,\tsq:%i",i,calPos[0][i],calPos[1][i],angle,surfaceQuality[i]);
		Serial.println();
	}
	Serial.println();

	// TODO: add in calibration logging
}

void sensorPlotting() {
	if(millis() - timeLastPlot >= dtPlot) {
		timeLastPlot = millis();

		for (int i = 0; i < ns; i++) {
			// NOTE: indiviudal sensor plotting is now done in calibration
			// Serial.printf("x_%i:%f,y_%i:%f",i,estPosSen[0][i],i,estPosSen[1][i]);
			// Serial.println();
			Serial.printf("sq_%i:%d",i,surfaceQuality[i]);
			Serial.println();
		}
	}
}

void readEepromCalibration() {
	int addr = 0;
	float tempVal = 0.0f;
	for (int i = 0; i < ns; i++) {
		EEPROM.get(addr, tempVal);
		cal[i].x = isnan(tempVal) ? cal[i].x : tempVal;			// check if calibration has been performed
		addr += sizeof(float);

		EEPROM.get(addr, tempVal);
		cal[i].y = isnan(tempVal) ? cal[i].y : tempVal;
		addr += sizeof(float);

		EEPROM.get(addr, tempVal);
		cal[i].r = isnan(tempVal) ? cal[i].r : tempVal;
		addr += sizeof(float);
	}
}

void writeEepromCalibration() {
	//  - Calibration values will be stored using (4)x(4)x(3) = 48 bytes
	//  - This comes from (4 bytes per float)x(4 sensors)x(2 vals per sensor)
	//	- The values will be stored in the order (cal[0].x),(cal[0].y),(cal[0].r),
	//		(cal[1].x),...,(cal[i].r), so sensor 0 calibration value for x,
	//		sensor 0 calibration value for y, sensor 0 calibration for rotation,
	//		sensor 1 calibration value for x, and so on

	Serial.println("writing calibration values to EEPROM...");

	int addr = 0;         // starting address is always 0
	for (int i = 0; i < ns; i++) {    // sensor number
		EEPROM.put(addr, cal[i].x);
		addr += sizeof(float);
		EEPROM.put(addr, cal[i].y);
		addr += sizeof(float);
		EEPROM.put(addr, cal[i].r);
		addr += sizeof(float);
  	}
}

void calibrate() {
	encoder.setEncoderHandler(nullHandler);
	encoder.setClickHandler(onClickCalibrationAdvance);

	float calDistance = 300.0;
	int currentRun = 0;
	int numRuns = 5;
	float tempCalScalar[2][ns] = {0.0f};
	float tempCalRot[2][ns] = {0.0f};

	for (int axis = 0; axis < 2; axis++) {
		while (currentRun < numRuns) {
			if (axis == 0) {
				drawCenteredText("Align X axis\nClick when at 0", 2);
			} else {
				drawCenteredText("Align Y axis\nClick when at 0", 2);
			}
			// draw circular progress bar based on run number
			int progressRadius = (screen->width()/2) - 10;
			int centerX = screen->width() / 2;
			int centerY = screen->height() / 2;
			float progressAngle = (currentRun + 1) * (TWO_PI / numRuns);

			screen->drawCircle(centerX, centerY, progressRadius, WHITE);
			for (float rad = 0; rad < progressAngle; rad+=0.01) {
				for (int i = 0; i < 3; i++) {
					int x = centerX + (progressRadius-1+i) * sinf(rad);
					int y = centerY - (progressRadius-1+i) * cosf(rad);
					screen->drawPixel(x, y, GREEN);
				}
			}

			while (state != CALIBRATION_ADVANCE) encoder.update();
			state = CALIBRATION;
			
			// Flush sensor readings then reset
			for (int i = 0; i < ns; i++) {
				sensors[i].readBurst();
				calPos[0][i] = 0.0f;
				calPos[1][i] = 0.0f;
			}

			// prompt x calibration
			// 	- do x run
			//		- "Reached 300?"
			// 		- "Retry?"
			if (axis == 0) {
				drawCenteredText("Move in +X\nClick when at 300mm", 2);
			} else {
				drawCenteredText("Move in +Y\nClick when at 300mm", 2);
			}

			while (state != CALIBRATION_ADVANCE) {
				if(micros() - timeLastPoll >= dt) {
					doSensingLinear();
				}

				encoder.update();
			}
			
			for (int i = 0; i < ns; i++) {
				float sensorDist = calDistance / (myDist(calPos[0][i],calPos[1][i],0,0));
				float sensorRot = axis ? atan2f(calPos[0][i],calPos[1][i]) : atan2f(-calPos[1][i],calPos[0][i]);	// Cry : Crx
				
				Serial.printf("Cx/y:%f, Cr:%f\n",sensorDist,sensorRot);

				tempCalScalar[axis][i] = tempCalScalar[axis][i] + (sensorDist / numRuns);
				tempCalRot[axis][i] =  tempCalRot[axis][i] + (sensorRot / numRuns);
			}

			state = CALIBRATION;
			currentRun++;
		}
		currentRun = 0;
	}

	Serial.println("Calibration results:");
	for (int i = 0; i < ns; i++) {
		float avgRot = (tempCalRot[0][i]+tempCalRot[1][i]) / 2;
		Serial.printf("\t%i: Cx:%f,Cy:%f, Crx:%f,Cry:%f, Cr:%f\n",
			i,tempCalScalar[0][i],tempCalScalar[1][i],tempCalRot[0][i],tempCalRot[1][i],avgRot);
	}

	const char* options[] = {"Exit", "Save!"};
	drawMenu(options, 2, acceptCal);
	encoder.setEncoderHandler(onEncoderAcceptCalibration);
	encoder.setClickHandler(onClickAcceptCalibration);
	while (state != CALIBRATION_ADVANCE) encoder.update();

	if (acceptCal) {
		for (int i=0; i<ns; i++) {
			cal[i].x = tempCalScalar[0][i];
			cal[i].y = tempCalScalar[1][i];
			cal[i].r = (tempCalRot[0][i]+tempCalRot[1][i]) / 2;
		}
		writeEepromCalibration();

		Serial.println("Values written to EEPROM. Reading them back just to double check:");
		readEepromCalibration();
	
		for (int i = 0; i < ns; i++) {
			Serial.printf("Sensor %i:\tCx:%.4f, Cy:%.4f, Cr:%.4f\n", i, cal[i].x, cal[i].y, cal[i].r);
		}

		encoderDesignType();
	} else {
		encoderDesignOrCalibrate();
	}
}