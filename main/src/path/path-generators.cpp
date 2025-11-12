#include "path-generators.h"
// Path properties
const float sinAmp = 5.0f;
const float sinPeriod = 50.0f;
const float pathMax_y = 100.0f;
// const float circleDiameter = 800.0f;

void lineGenerator() {
	int num_points = 1000;
	// Generate line path to cut
	for (int i = 0; i < num_points; i++) {
		path.points[i] = Point{
			x: 0.0f,
			y: (pathMax_y) * (float)i / (num_points - 1),
			z: -matThickness,
			feature: NORMAL
		};
	}

	path.numPoints = num_points;
	path.points[num_points-1].z = restHeight;
}

void sinGenerator() {
	int num_points = 1000;
	// Generate sine path to cut
	for (int i = 0; i < num_points; ++i) {
		float y = (pathMax_y) * (float)i / (num_points - 1);
		float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
		path.points[i] = Point{
			x: x,
			y: y,
			z: -matThickness,
			feature: NORMAL
		};
	}

	path.numPoints = num_points;
	path.points[num_points-1].z = restHeight;
}

void zigZagGenerator() {
	int num_points = 1000;
	float zigSize = 40.0f;

	for (int i = 0; i < num_points; ++i) {
		float y = (pathMax_y) * (float)i / (num_points - 1);
		float x = fmod(y, zigSize);
		if (x > (zigSize / 2)) {
			x = zigSize - x;
		}
		
		path.points[i] = Point{
			x: x,
			y: y,
			z: -matThickness,
			feature: NORMAL
		};
	}
	
	path.numPoints = num_points;
	path.points[num_points-1].z = restHeight;
}

void doubleLineGenerator() {
	// One line going up at x = -20 and one line going down at x = 20
	int num_points = 1000;
	float length = 100.0f;

	for (int i = 0; i < num_points; i++) {
		float scale = (float)i / (num_points - 1);
		float zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
		path.points[i] = Point{
			x: -20.0f,
			y: length * scale,
			z: zVal,
			feature: NORMAL
		};
		path.points[i+num_points] = Point{
			x: 20.0f,
			y: length * (1.0f - scale),
			z: zVal,
			feature: NORMAL
		};
	}

	path.numPoints = 2 * num_points;
}

void circleGenerator() {
	int num_points = 4000;
	float r = 30.0f;
	Point center = Point{x: 0.0f, y: r, z: -matThickness};

	path.points[0] = Point{x: 0.0f, y: 0.0f, z: restHeight};

	for (int i = 1; i < num_points; i++) {
		float theta = (float)i/(num_points-1)*(2.0f*PI);
		float zVal = (i == num_points - 1 || i == 1) ? restHeight : -matThickness;
		
		path.points[i] = Point{
			x: center.x + r*cosf(theta - PI/2.0f),
			y: center.y + r*sinf(theta - PI/2.0f),
			z: zVal,
			feature: NORMAL
		};
	}

	path.numPoints = num_points;
}

void diamondGenerator() {
	int num_points = 1000;
	float angle = 60.0f;
	float angle_rad = angle * (M_PI / 180.0f);
	float segment_length = 100.0f;
	int dirs[2] = {1, -1};

	float y_increment = segment_length / (num_points - 1);
	float x_increment = y_increment / tanf(angle_rad);

	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < num_points; i++) {
			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
			int yIndex = p == 1 ? (num_points - 1 - i) : i;
			float zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
			path.points[i + p*num_points] = Point{
				x: dirs[p] * xIndex * x_increment,
				y: yIndex * y_increment,
				z: zVal
			};
		}
	}

	path.numPoints = 2 * num_points;

}

void squareGeneratorSine() {
	int num_points = 1000;
	float angle = 45.0f;
	float angle_rad = angle * (M_PI / 180.0f);
	float segment_length = 100.0f;
	float engrave_depth = matThickness / 4.0f;
	int dirs[3] = {1, -1, 1};

	// Generate design engraving
	for (int i = 0; i < num_points; ++i) {
		float y = (segment_length) * (float)i / (num_points - 1);
		float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
		float zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i] = Point{x, y, zVal};
	}

	// Calculate the x and y increments based on the angle
	float y_increment = segment_length / (num_points - 1);
	float x_increment = y_increment / tanf(angle_rad);

	// Generate diamond path to cut
	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < num_points; i++) {
			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
			int yIndex = p == 1 ? (num_points - 1 - i) : i;
			float zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
			if (p == 0) {
				path.points[i + (2)*num_points] = Point{
					x: dirs[p] * xIndex * x_increment,
					y: yIndex * y_increment,
					z: zVal
				};
			} else {
				path.points[i + (1)*num_points] = Point{
					x: dirs[p] * xIndex * x_increment,
					y: yIndex * y_increment,
					z: zVal
				};
			}
		}
	}

	path.numPoints = 3 * num_points;
}

void squareGeneratorWave() {
	// Currently identical to squareGeneratorSine
	// TODO: Implement wave pattern
	squareGeneratorSine();
}

void squareGeneratorMake() {
	int num_points = 1000;
	float x = 0.0f;
	float y = 0.0f;
	float zVal = 0.0f;
	float start_y = 0.0f;
	float angle = 45.0f;
	float angle_rad = angle * (M_PI / 180.0f);
	float segment_length = 100.0f;
	float engrave_depth = matThickness / 4.0f;
	float make_ratio = 0.3f;
	float colon_ratio = 0.6f;
	float make_length = make_ratio * segment_length;
	float colon_length = colon_ratio * make_length;
	int dirs[7] = {1, -1, 1, -1, 1, -1, 1};

	// Generate left M vertical
	start_y = (segment_length/2.0f) - (make_length/2.0f);
	for (int i = 0; i < num_points; ++i) {
		y = start_y + (make_length) * (float)i / (num_points - 1);
		x = -make_length/2.0f;
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
		path.points[i] = Point{x, y, zVal};
	}

	// Generate left M slanted
	start_y = (segment_length/2.0f) + (make_length/2.0f);
	for (int i = 0; i < num_points; ++i) {
		y = start_y - (make_length) * (float)i / (num_points - 1);
		x = -make_length/2.0f + (make_length * 3.0f/8.0f) * (float)i / (num_points - 1);
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i + num_points] = Point{x, y, zVal};
	}

	// Generate right M slanted
	start_y = (segment_length/2.0f) - (make_length/2.0f);
	for (int i = 0; i < num_points; ++i) {
		y = start_y + (make_length) * (float)i / (num_points - 1);
		x = -make_length/2.0f + (make_length * 3.0f/8.0f) + (make_length * 3.0f/8.0f) * (float)i / (num_points - 1);
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i + 2*num_points] = Point{x, y, zVal};
	}

	// Generate right M vertical
	start_y = (segment_length/2.0f) + (make_length/2.0f);
	for (int i = 0; i < num_points; ++i) {
		y = start_y - (make_length) * (float)i / (num_points - 1);
		x = make_length*1.0f/4.0f;
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i + 3*num_points] = Point{x, y, zVal};
	}

	// Generate colon
	start_y = (segment_length/2.0f) - (colon_length/2.0f);
	float dot_ratio = 0.2f;
	float dot_length = dot_ratio * colon_length;
	for (int i = 0; i < num_points; ++i) {
		y = start_y + (colon_length) * (float)i / (num_points - 1);
		x = make_length/2.0f;
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		if (y - start_y <= dot_length || y - start_y > colon_length - dot_length) {
			path.points[i + 4*num_points] = Point{x, y, zVal};
		} else {
			path.points[i + 4*num_points] = Point{x, y, restHeight};
		}
	}

	// Calculate the x and y increments based on the angle
	float y_increment = segment_length / (num_points - 1);
	float x_increment = y_increment / tanf(angle_rad);

	// Generate diamond path to cut
	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < num_points; i++) {
			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
			int yIndex = p == 1 ? (num_points - 1 - i) : i;
			zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
			if (p == 0) {
				path.points[i + 6*num_points] = Point{
					x: dirs[p] * xIndex * x_increment,
					y: yIndex * y_increment,
					z: zVal
				};
			} else {
				path.points[i + 5*num_points] = Point{
					x: dirs[p] * xIndex * x_increment,
					y: yIndex * y_increment,
					z: zVal
				};
			}
		}
	}

	int num_paths = 7;
	path.numPoints = num_paths * num_points;
}

void drillSquareGenerator() {
	// Generate a drill cycle that starts at (0,0) and does a square pattern
	// TODO: modify this to work with new version
	float l = 50.0f;

	path.points[0] = Point{x: 0.0f, y: 0.0f, z: -matThickness, feature: DRILL};
	path.points[1] = Point{x: l, y: l, z: -matThickness, feature: DRILL};
	path.points[2] = Point{x: -l, y: l, z: -matThickness, feature: DRILL};
	path.points[3] = Point{x: -l, y: -l, z: -matThickness, feature: DRILL};
	path.points[4] = Point{x: l, y: -l, z: -matThickness, feature: DRILL};
	path.points[5] = Point{x: 0.0f, y: 0.0f, z: -matThickness, feature: DRILL};

	path.numPoints = 6;
}

void makePresetPath() {
	switch (designPreset) {
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
			diamondGenerator();
			Serial.println("Sine square path generated!");
			break;
		case 5:
			squareGeneratorSine();
			Serial.println("Circle path generated!");
			break;
		case 6:
			squareGeneratorMake();
			Serial.println("Wave square path generated!");
			break;

		case 7:
			circleGenerator();
			Serial.println("Circle path generated!");
			break;
		case 8:
			drillSquareGenerator();
			Serial.println("Square drill path generated!");
			break;
	}

	// Log the generated path
	logPath();
}
