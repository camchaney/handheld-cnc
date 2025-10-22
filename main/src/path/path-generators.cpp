#include "path-generators.h"
// Path properties
const float sinAmp = 5.0;
const float sinPeriod = 50.0;
const float pathMax_y = 100.0;
// const float circleDiameter = 800.0;

Point lineGenerator(int i) {
	// Generate line path to cut
	Point p = Point{
		x: 0,
		y: (pathMax_y) * (float)i / (pathInfo.numPoints - 1),
		z: -matThickness,
		feature: NORMAL
	};

	if (i == pathInfo.numPoints - 1) { p.z = restHeight; }
	return p;
}

Point sinGenerator(int i) {
	// Generate sine path to cut
	float y = (pathMax_y) * (float)i / (pathInfo.numPoints - 1);
	float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
	Point p = Point{
		x: x,
		y: y,
		z: -matThickness,
		feature: NORMAL
	};

	if (i == pathInfo.numPoints - 1) { p.z = restHeight; }
	return p;
}

Point zigZagGenerator(int i) {
	float zigSize = 40;

	float y = (pathMax_y) * (float)i / (pathInfo.numPoints - 1);
	float x = fmod(y, zigSize);
	if (x > (zigSize / 2)) {
		x = zigSize - x;
	}
	
	Point p = Point{
		x: x,
		y: y,
		z: -matThickness,
		feature: NORMAL
	};
	
	if (i == pathInfo.numPoints - 1) { p.z = restHeight; }
	return p;
}

Point doubleLineGenerator(int i) {
	// One line going up at x = -20 and one line going down at x = 20
	int n = 1000;
	float length = 100.0;
	Point p;

	float scale = (float)i / (n - 1);
	float zVal = (i == n - 1 || i == 0) ? restHeight : -matThickness;
	if (i < n) {
		p = Point{
			x: -20.0,
			y: length * scale,
			z: zVal,
			feature: NORMAL
		};
	} else {
		p = Point{
			x: 20.0,
			y: length * (1 - scale),
			z: zVal,
			feature: NORMAL
		};
	}
	return p;
}

Point circleGenerator(int i) {
	float r = 30.0;
	Point center = Point{x: 0.0, y: r, z: -matThickness};

	float theta = (float)i/(pathInfo.numPoints-1)*(2*PI);
	float zVal = (i == pathInfo.numPoints - 1 || i == 1) ? restHeight : -matThickness;

	Point p = Point{
		x: center.x + r*cosf(theta - PI/2),
		y: center.y + r*sinf(theta - PI/2),
		z: zVal,
		feature: NORMAL
	};
	return p;
}

Point diamondGenerator(int i) {
	int n = pathInfo.numPoints / 2;
	float angle = 60;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	int dirs[2] = {1, -1};
	Point p;

	float y_increment = segment_length / (n - 1);
	float x_increment = y_increment / tan(angle_rad);

	int pass = (i < n) ? 0 : 1;
	int xIndex = (i >= n / 2) ? (n - 1 - i) : i;
	int yIndex = pass == 1 ? (n - 1 - i) : i;
	float zVal = (i == n - 1 || i == 0) ? restHeight : -matThickness;

	p = Point{
		x: dirs[pass] * xIndex * x_increment,
		y: yIndex * y_increment,
		z: zVal
	};
	return p;
}

Point squareGeneratorSine(int i) {
	int n = pathInfo.numPoints / 3;
	float angle = 45;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	float engrave_depth = matThickness / 4;
	int dirs[3] = {1, -1, 1};

	Point p;
	float x;
	float y;

	// Generate design engraving
	if (i < n) {
		y = (segment_length) * (float)i / (n - 1);
		x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
		float zVal = (i == n - 1 || i == 0) ? restHeight : -engrave_depth;
		p = Point{x, y, zVal};
	}

	// Calculate the x and y increments based on the angle
	float y_increment = segment_length / (n - 1);
	float x_increment = y_increment / tan(angle_rad);

	// Generate diamond path to cut
	int pass = (i < n) ? 0 : 1;
	int xIndex = (i >= n / 2) ? (n - 1 - i) : i;
	int yIndex = pass == 1 ? (n - 1 - i) : i;
	float zVal = (i == n - 1 || i == 0) ? restHeight : -matThickness;

	p = Point{
		x: dirs[pass] * xIndex * x_increment,
		y: yIndex * y_increment,
		z: zVal
	};
	return p;
}

// Point squareGeneratorWave(int i) {
// 	// Currently identical to squareGeneratorSine
// 	// TODO: Implement wave pattern
// 	squareGeneratorSine();
// }

// Point squareGeneratorMake(int i) {
// 	int num_points = 1000;
// 	float x = 0.0f;
// 	float y = 0.0f;
// 	float zVal = 0.0f;
// 	float start_y = 0.0f;
// 	float angle = 45;
// 	float angle_rad = angle * (M_PI / 180.0);
// 	float segment_length = 100.0;
// 	float engrave_depth = matThickness / 4;
// 	float make_ratio = 0.3;
// 	float colon_ratio = 0.6;
// 	float make_length = make_ratio * segment_length;
// 	float colon_length = colon_ratio * make_length;
// 	int dirs[7] = {1, -1, 1, -1, 1, -1, 1};

// 	// Generate left M vertical
// 	start_y = (segment_length/2) - (make_length/2);
// 	for (int i = 0; i < num_points; ++i) {
// 		y = start_y + (make_length) * (float)i / (num_points - 1);
// 		x = -make_length/2;
// 		zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
// 		path.points[i] = Point{x, y, zVal};
// 	}

// 	// Generate left M slanted
// 	start_y = (segment_length/2) + (make_length/2);
// 	for (int i = 0; i < num_points; ++i) {
// 		y = start_y - (make_length) * (float)i / (num_points - 1);
// 		x = -make_length/2 + (make_length * 3/8) * (float)i / (num_points - 1);
// 		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
// 		path.points[i + num_points] = Point{x, y, zVal};
// 	}

// 	// Generate right M slanted
// 	start_y = (segment_length/2) - (make_length/2);
// 	for (int i = 0; i < num_points; ++i) {
// 		y = start_y + (make_length) * (float)i / (num_points - 1);
// 		x = -make_length/2 + (make_length * 3/8) + (make_length * 3/8) * (float)i / (num_points - 1);
// 		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
// 		path.points[i + 2*num_points] = Point{x, y, zVal};
// 	}

// 	// Generate right M vertical
// 	start_y = (segment_length/2) + (make_length/2);
// 	for (int i = 0; i < num_points; ++i) {
// 		y = start_y - (make_length) * (float)i / (num_points - 1);
// 		x = make_length*1/4;
// 		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
// 		path.points[i + 3*num_points] = Point{x, y, zVal};
// 	}

// 	// Generate colon
// 	start_y = (segment_length/2) - (colon_length/2);
// 	float dot_ratio = 0.2;
// 	float dot_length = dot_ratio * colon_length;
// 	for (int i = 0; i < num_points; ++i) {
// 		y = start_y + (colon_length) * (float)i / (num_points - 1);
// 		x = make_length/2;
// 		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
// 		if (y - start_y <= dot_length || y - start_y > colon_length - dot_length) {
// 			path.points[i + 4*num_points] = Point{x, y, zVal};
// 		} else {
// 			path.points[i + 4*num_points] = Point{x, y, restHeight};
// 		}
// 	}

// 	// Calculate the x and y increments based on the angle
// 	float y_increment = segment_length / (num_points - 1);
// 	float x_increment = y_increment / tan(angle_rad);

// 	// Generate diamond path to cut
// 	for (int p = 0; p < 2; p++) {
// 		for (int i = 0; i < num_points; i++) {
// 			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
// 			int yIndex = p == 1 ? (num_points - 1 - i) : i;
// 			zVal = (i == num_points - 1 || i == 0) ? restHeight : -matThickness;
// 			if (p == 0) {
// 				path.points[i + 6*num_points] = Point{
// 					x: dirs[p] * xIndex * x_increment,
// 					y: yIndex * y_increment,
// 					z: zVal
// 				};
// 			} else {
// 				path.points[i + 5*num_points] = Point{
// 					x: dirs[p] * xIndex * x_increment,
// 					y: yIndex * y_increment,
// 					z: zVal
// 				};
// 			}
// 		}
// 	}

// 	int num_paths = 7;
// 	path.numPoints = num_paths * num_points;
// }

// Point drillSquareGenerator(int i) {
// 	// Generate a drill cycle that starts at (0,0) and does a square pattern
// 	// TODO: modify this to work with new version
// 	float l = 50.0f;

// 	path.points[0] = Point{x: 0.0f, y: 0.0f, z: -matThickness, feature: DRILL};
// 	path.points[1] = Point{x: l, y: l, z: -matThickness, feature: DRILL};
// 	path.points[2] = Point{x: -l, y: l, z: -matThickness, feature: DRILL};
// 	path.points[3] = Point{x: -l, y: -l, z: -matThickness, feature: DRILL};
// 	path.points[4] = Point{x: l, y: -l, z: -matThickness, feature: DRILL};
// 	path.points[5] = Point{x: 0.0f, y: 0.0f, z: -matThickness, feature: DRILL};

// 	path.numPoints = 6;
// }

void makePresetPath() {
	// Reset path buffer
	pathBuffer = PathBuffer();
	
	switch (designPreset) {
		case 0:
			pathInfo.numPoints = 1000;
			Serial.println("Line path generated!");
			break;
		case 1:
			pathInfo.numPoints = 1000;
			Serial.println("Sine wave path generated!");
			break;
		case 2:
			pathInfo.numPoints = 1000;
			Serial.println("Zig-zag path generated!");
			break;
		case 3:
			pathInfo.numPoints = 2000;
			Serial.println("Double line path generated!");
			break;
		case 4:
			pathInfo.numPoints = 2000;
			Serial.println("Sine square path generated!");
			break;
		case 5:
			pathInfo.numPoints = 3000;
			Serial.println("Circle path generated!");
			break;
		// case 6:
		// 	pathInfo.numPoints = 7000;
		// 	squareGeneratorMake();
		// 	Serial.println("Wave square path generated!");
		// 	break;
		case 6:
			pathInfo.numPoints = 4000;
			Serial.println("Circle path generated!");
			break;
		// case 8:
		// 	pathInfo.numPoints = 6;
		// 	drillSquareGenerator();
		// 	Serial.println("Square drill path generated!");
		// 	break;
	}

	updatePresetPathBuffer();

	// Log the generated path
	// logPath();
}

void updatePresetPathBuffer() {
	Serial.printf("Updating, current_point_idx: %d, first_point_idx: %d\n", current_point_idx, pathBuffer.first_point_idx);

	if (current_point_idx == pathBuffer.first_point_idx && !isnan(pathBuffer.points[0].x)) {
		// Path has not progressed, no need to update buffer
		Serial.println("Path buffer is up to date.");
		return;
	}
	pathBuffer.first_point_idx = current_point_idx;

	int buffer_idx = 0;			// relative point index within buffer (curr_point_idx - first_point_idx)
	Point activePoint = {NAN};

	// Clear pathBuffer
	// TODO: is there a better way to do this without emptying and filling every point?
	for (int j = 0; j < PATH_BUFFER_SIZE; j++) {
		pathBuffer.points[j] = {NAN};
	}

	while (buffer_idx < PATH_BUFFER_SIZE) {
		int point_idx = current_point_idx + buffer_idx;
		switch (designPreset) {
			case 0:
				activePoint = lineGenerator(point_idx);
				break;
			case 1:
				activePoint = sinGenerator(point_idx);
				break;
			case 2:
				activePoint = zigZagGenerator(point_idx);
				break;
			case 3:
				activePoint = doubleLineGenerator(point_idx);
				break;
			case 4:
				activePoint = diamondGenerator(point_idx);
				break;
			case 5:
				activePoint = squareGeneratorSine(point_idx);
				break;
			// case 6:
			// 	activePoint = squareGeneratorMake(point_idx);
			// 	break;
			case 6:
				activePoint = circleGenerator(point_idx);
				break;
			// case 8:
			// 	activePoint = drillSquareGenerator(point_idx);
			// 	break;
		}
		pathBuffer.points[buffer_idx] = activePoint;
		buffer_idx++;
	}
}
