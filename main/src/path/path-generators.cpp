#include "path-generators.h"
#include "../config.h"
#include "../globals.h"
#include "../io/logging.h"
#include <Arduino.h>

// Path properties
const float sinAmp = 5.0;
const float sinPeriod = 50.0;
const float pathMax_y = 100.0;
const float circleDiameter = 800.0;

void lineGenerator() {
	// Generate line path to cut
	for (int i = 0; i < MAX_POINTS; i++) {
		paths[0][i] = Point{
			x: 0,
			y: (pathMax_y) * (float)i / (MAX_POINTS - 1),
			z: -matThickness
		};
	}

	num_paths = 1;
	num_points = MAX_POINTS;
}

void sinGenerator() {
	// Generate sine path to cut
	for (int i = 0; i < MAX_POINTS; ++i) {
		float y = (pathMax_y) * (float)i / (MAX_POINTS - 1);
		float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
		paths[0][i] = Point{
			x: x,
			y: y,
			z: -matThickness
		};
	}

	num_paths = 1;
	num_points = MAX_POINTS;
}

void zigZagGenerator() {
	float zigSize = 40;

	for (int i = 0; i < MAX_POINTS; ++i) {
		float y = (pathMax_y) * (float)i / (MAX_POINTS - 1);
		float x = fmod(y, zigSize);
		if (x > (zigSize / 2)) {
			x = zigSize - x;
		}
		
		paths[0][i] = Point{
			x: x,
			y: y,
			z: -matThickness
		};
	}
	
	num_paths = 1;
	num_points = MAX_POINTS;
}

void doubleLineGenerator() {
	// One line going up at x = -20 and one line going down at x = 20
	float length = 100.0;

	for (int i = 0; i < MAX_POINTS; i++) {
		float scale = (float)i / (MAX_POINTS - 1);
		paths[0][i] = Point{
			x: -20.0,
			y: length * scale,
			z: -matThickness
		};
		paths[1][i] = Point{
			x: 20.0,
			y: length * (1 - scale),
			z: -matThickness
		};
	}

	pathDir[0] = 1;
	pathDir[1] = -1;

	num_paths = 2;
	num_points = MAX_POINTS;
}

void circleGenerator() {
	float r = 30.0;
	Point center = Point{x: 0.0, y: 50.0, z: -matThickness};
	float theta;
	
	// Path alternates forward and backward
	pathDir[0] = 1;
	pathDir[1] = -1;
	pathDir[2] = 1;
	pathDir[3] = -1;

	for (int i = 0; i < MAX_POINTS; i++) {
		theta = (float)i/MAX_POINTS*PI/2;
		
		// Q4: 270->360
		paths[0][i] = Point{
			x: center.x + r*cosf(3*PI/2 + theta),
			y: center.y + r*sinf(3*PI/2 + theta),
			z: -matThickness
		};

		// Q2: 90->180
		paths[1][i] = Point{
			x: center.x + r*cosf(PI/2 + theta),
			y: center.y + r*sinf(PI/2 + theta),
			z: -matThickness
		};

		// Q3: 270->180
		paths[2][i] = Point{
			x: center.x + r*cosf(3*PI/2 - theta),
			y: center.y + r*sinf(3*PI/2 - theta),
			z: -matThickness
		};

		// Q1: 90->0 
		paths[3][i] = Point{
			x: center.x + r*cosf(PI/2 - theta),
			y: center.y + r*sinf(PI/2 - theta),
			z: -matThickness
		};
	}

	num_paths = 4;
	num_points = MAX_POINTS;
}

void squareGenerator() {
	float angle = 60;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	pathDir[0] = 1;
	pathDir[1] = -1;

	float y_increment = segment_length / (MAX_POINTS - 1);
	float x_increment = y_increment / tan(angle_rad);

	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < MAX_POINTS; i++) {
			int xIndex = (i >= MAX_POINTS / 2) ? (MAX_POINTS - 1 - i) : i;
			int yIndex = p == 1 ? (MAX_POINTS - 1 - i) : i;
			paths[p][i] = Point{
				x: pathDir[p] * xIndex * x_increment,
				y: yIndex * y_increment,
				z: -matThickness
			};
		}
	}

	num_paths = 2;
	num_points = MAX_POINTS;
}

void squareGeneratorSine() {
	float angle = 45;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	float engrave_depth = matThickness / 4;
	pathDir[0] = 1;
	pathDir[1] = -1;
	pathDir[2] = 1;

	// Generate design engraving
	for (int i = 0; i < MAX_POINTS; ++i) {
		float y = (segment_length) * (float)i / (MAX_POINTS - 1);
		float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
		paths[0][i] = Point{x, y, -engrave_depth};
	}

	// Calculate the x and y increments based on the angle
	float y_increment = segment_length / (MAX_POINTS - 1);
	float x_increment = y_increment / tan(angle_rad);

	// Generate diamond path to cut
	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < MAX_POINTS; i++) {
			int xIndex = (i >= MAX_POINTS / 2) ? (MAX_POINTS - 1 - i) : i;
			int yIndex = p == 1 ? (MAX_POINTS - 1 - i) : i;
			if (p == 0) {
				paths[2][i] = Point{
					x: pathDir[p] * xIndex * x_increment,
					y: yIndex * y_increment,
					z: -matThickness
				};
			} else {
				paths[1][i] = Point{
					x: pathDir[p] * xIndex * x_increment,
					y: yIndex * y_increment,
					z: -matThickness
				};
			}
		}
	}

	num_paths = 3;
	num_points = MAX_POINTS;
}

void squareGeneratorWave() {
	// Currently identical to squareGeneratorSine
	// TODO: Implement wave pattern
	squareGeneratorSine();
}

void parseNC(const char* filename) {
	FsFile myFile = sd.open(filename);
	if (!myFile) {
		Serial.println("Failed to open file for reading");
		return;
	}

	int idx = 0;
	while (myFile.available()) {
		String line = myFile.readStringUntil('\n');
		int xPos = line.indexOf('X');
		int yPos = line.indexOf('Y');
		int zPos = line.indexOf('Z');
		int spacePos = 0;
		float x = 0;
		float y = 0;
		float z = 0;

		if (xPos != -1 || yPos != -1 || zPos != -1) {
			if (xPos == -1) {
				x = paths[0][idx-1].x;
			} else {
				spacePos = line.indexOf(' ', xPos);
				x = line.substring(xPos+1, spacePos).toFloat();
			}
			
			if (yPos == -1) {
				y = paths[0][idx-1].y;
			} else {
				spacePos = line.indexOf(' ', yPos);
				if (spacePos == -1) {
					spacePos = line.length();
				}
				y = line.substring(yPos+1, spacePos).toFloat();
			}

			if (zPos == -1) {
				z = paths[0][idx-1].z;
			} else {
				spacePos = line.indexOf(' ', zPos);
				if (spacePos == -1) {
					spacePos = line.length();
				}
				z = line.substring(zPos+1, spacePos).toFloat();
			}

			paths[0][idx] = Point{x,y,z};
			idx++;
		}
	}

	myFile.close();
}

void makePath() {
	switch (designMode) {
		case 0:
			lineGenerator();
			Serial.println("Line path generated!");
			break;
		case 1:
			sinGenerator();
			Serial.println("Sine wave path generated!");
			break;
		case 2:
			zigZagGenerator();
			Serial.println("Zig-zag path generated!");
			break;
		case 3:
			doubleLineGenerator();
			Serial.println("Double line path generated!");
			break;
		case 4:
			squareGeneratorSine();
			Serial.println("Sine square path generated!");
			break;
		case 5:
			circleGenerator();
			Serial.println("Circle path generated!");
			break;
		case 6:
			squareGeneratorWave();
			Serial.println("Wave square path generated!");
			break;
		case 7:
			squareGeneratorWave();
			Serial.println("____ square path generated!");
			break;
		case 8:
			squareGeneratorWave();
			Serial.println("Hexagon square path generated!");
			break;
	}

	// Log the generated path
	logPath();
}
