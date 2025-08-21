#include "path-generators.h"
// Path properties
const float sinAmp = 5.0;
const float sinPeriod = 50.0;
const float pathMax_y = 100.0;

void lineGenerator() {
    int num_points = 1000;

    // rotation in degrees: 0 -> up (0, +length), 90 -> right (+length, 0)
    float rad = rotation * (M_PI / 180.0f);
    float dx = sinf(rad);
    float dy = cosf(rad);

    for (int i = 0; i < num_points; i++) {
        float t = (float)i / (num_points - 1);
        path.points[i] = Point{
            x: xOffset + dx * length * t,
            y: yOffset + dy * length * t,
            z: -deepth,
            feature: NORMAL
        };
    }

    path.numPoints = num_points;
    path.points[num_points - 1].z = restHeight;
}

void sinGenerator() {
	int num_points = 1000;
	// Generate sine path to cut
	for (int i = 0; i < num_points; ++i) {
		float y = (pathMax_y) * (float)i / (num_points - 1);
		float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
		path.points[i] = Point{
			x: x + xOffset,
			y: y + yOffset,
			z: -deepth,
			feature: NORMAL
		};
	}

	path.numPoints = num_points;
	path.points[num_points-1].z = restHeight;
}

void zigZagGenerator() {
	int num_points = 1000;
	float zigSize = 40;

	for (int i = 0; i < num_points; ++i) {
		float y = (pathMax_y) * (float)i / (num_points - 1);
		float x = fmod(y, zigSize);
		if (x > (zigSize / 2)) {
			x = zigSize - x;
		}
		
		path.points[i] = Point{
			x: x + xOffset,
			y: y + yOffset,
			z: -deepth,
			feature: NORMAL
		};
	}
	
	path.numPoints = num_points;
	path.points[num_points-1].z = restHeight;
}

void doubleLineGenerator() {
	// One line going up at x = -20 and one line going down at x = 20
	int num_points = 1000;
	float length = 100.0;

	for (int i = 0; i < num_points; i++) {
		float scale = (float)i / (num_points - 1);
		float zVal = (i == num_points - 1 || i == 0) ? restHeight : -deepth;
		path.points[i] = Point{
			x: -20.0f + xOffset,
			y: length * scale + yOffset,
			z: zVal,
			feature: NORMAL
		};
		path.points[i+num_points] = Point{
			x: 20.0f+ xOffset,
			y: length * (1 - scale) + yOffset,
			z: zVal,
			feature: NORMAL
		};
	}

	path.numPoints = 2 * num_points;
}

void circleGenerator() {
	int num_points = 4000;
	Point center = Point{x: xOffset, y: yOffset, z: -deepth};

	path.points[0] = Point{x: xOffset, y: yOffset - radius, z: restHeight};

	for (int i = 1; i < num_points; i++) {
		float theta = (float)i / (num_points - 1) * (2 * PI);
		float zVal = (i == num_points - 1 || i == 1) ? restHeight : -deepth;
		
		path.points[i] = Point{
			x: center.x + radius * cosf(theta - PI/2),
			y: center.y + radius * sinf(theta - PI/2),
			z: zVal,
			feature: NORMAL
		};
	}

	path.numPoints = num_points;
}

void rectangleGenerator() {
    // Points per edge
    const int edge_points = 1000;

    // Orientation: 0° -> up (0,+length), 90° -> right (+length,0)
    float rad = rotation * (M_PI / 180.0f);
    float lx = sinf(rad);   // length direction x
    float ly = cosf(rad);   // length direction y

    // Perpendicular (width direction): right-handed relative to length direction
    float wx = ly;
    float wy = -lx;

    // Half extents
    float halfL = length * 0.5f;
    float halfW = width  * 0.5f;

    // Center at offsets
    float cx = xOffset;
    float cy = yOffset;

    // Rectangle corners (counter-clockwise)
    float x0 = cx - lx*halfL - wx*halfW;
    float y0 = cy - ly*halfL - wy*halfW;

    float x1 = cx + lx*halfL - wx*halfW;
    float y1 = cy + ly*halfL - wy*halfW;

    float x2 = cx + lx*halfL + wx*halfW;
    float y2 = cy + ly*halfL + wy*halfW;

    float x3 = cx - lx*halfL + wx*halfW;
    float y3 = cy - ly*halfL + wy*halfW;

    const int total = 4 * edge_points;
    int idx = 0;

    auto emitEdge = [&](float sx, float sy, float ex, float ey) {
        for (int i = 0; i < edge_points; ++i, ++idx) {
            float t = (float)i / (edge_points - 1);
            float px = sx + (ex - sx) * t;
            float py = sy + (ey - sy) * t;
            float pz = (idx == 0 || idx == total - 1) ? restHeight : -deepth;

            path.points[idx] = Point{
                x: px,
                y: py,
                z: pz,
                feature: NORMAL
            };
        }
    };

    // Emit perimeter as one continuous loop
    emitEdge(x0, y0, x1, y1);
    emitEdge(x1, y1, x2, y2);
    emitEdge(x2, y2, x3, y3);
    emitEdge(x3, y3, x0, y0);

    path.numPoints = total;
}

void diamondGenerator() {
	int num_points = 1000;
	float angle = 60;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	int dirs[2] = {1, -1};

	float y_increment = segment_length / (num_points - 1);
	float x_increment = y_increment / tan(angle_rad);

	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < num_points; i++) {
			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
			int yIndex = p == 1 ? (num_points - 1 - i) : i;
			float zVal = (i == num_points - 1 || i == 0) ? restHeight : -deepth;
			path.points[i + p*num_points] = Point{
				x: dirs[p] * xIndex * x_increment + xOffset,
				y: yIndex * y_increment + yOffset,
				z: zVal
			};
		}
	}

	path.numPoints = 2 * num_points;

}

void squareGeneratorSine() {
	int num_points = 1000;
	float angle = 45;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	float engrave_depth = deepth / 4;
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
	float x_increment = y_increment / tan(angle_rad);

	// Generate diamond path to cut
	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < num_points; i++) {
			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
			int yIndex = p == 1 ? (num_points - 1 - i) : i;
			float zVal = (i == num_points - 1 || i == 0) ? restHeight : -deepth;
			if (p == 0) {
				path.points[i + (2)*num_points] = Point{
					x: dirs[p] * xIndex * x_increment + xOffset,
					y: yIndex * y_increment + yOffset,
					z: zVal
				};
			} else {
				path.points[i + (1)*num_points] = Point{
					x: dirs[p] * xIndex * x_increment + xOffset,
					y: yIndex * y_increment + yOffset,
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
	float angle = 45;
	float angle_rad = angle * (M_PI / 180.0);
	float segment_length = 100.0;
	float engrave_depth = deepth / 4;
	float make_ratio = 0.3;
	float colon_ratio = 0.6;
	float make_length = make_ratio * segment_length;
	float colon_length = colon_ratio * make_length;
	int dirs[7] = {1, -1, 1, -1, 1, -1, 1};

	// Generate left M vertical
	start_y = (segment_length/2) - (make_length/2);
	for (int i = 0; i < num_points; ++i) {
		y = start_y + (make_length) * (float)i / (num_points - 1);
		x = -make_length/2;
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -deepth;
		path.points[i] = Point{x, y, zVal};
	}

	// Generate left M slanted
	start_y = (segment_length/2) + (make_length/2);
	for (int i = 0; i < num_points; ++i) {
		y = start_y - (make_length) * (float)i / (num_points - 1);
		x = -make_length/2 + (make_length * 3/8) * (float)i / (num_points - 1);
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i + num_points] = Point{x, y, zVal};
	}

	// Generate right M slanted
	start_y = (segment_length/2) - (make_length/2);
	for (int i = 0; i < num_points; ++i) {
		y = start_y + (make_length) * (float)i / (num_points - 1);
		x = -make_length/2 + (make_length * 3/8) + (make_length * 3/8) * (float)i / (num_points - 1);
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i + 2*num_points] = Point{x, y, zVal};
	}

	// Generate right M vertical
	start_y = (segment_length/2) + (make_length/2);
	for (int i = 0; i < num_points; ++i) {
		y = start_y - (make_length) * (float)i / (num_points - 1);
		x = make_length*1/4;
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		path.points[i + 3*num_points] = Point{x, y, zVal};
	}

	// Generate colon
	start_y = (segment_length/2) - (colon_length/2);
	float dot_ratio = 0.2;
	float dot_length = dot_ratio * colon_length;
	for (int i = 0; i < num_points; ++i) {
		y = start_y + (colon_length) * (float)i / (num_points - 1);
		x = make_length/2;
		zVal = (i == num_points - 1 || i == 0) ? restHeight : -engrave_depth;
		if (y - start_y <= dot_length || y - start_y > colon_length - dot_length) {
			path.points[i + 4*num_points] = Point{x, y, zVal};
		} else {
			path.points[i + 4*num_points] = Point{x, y, restHeight};
		}
	}

	// Calculate the x and y increments based on the angle
	float y_increment = segment_length / (num_points - 1);
	float x_increment = y_increment / tan(angle_rad);

	// Generate diamond path to cut
	for (int p = 0; p < 2; p++) {
		for (int i = 0; i < num_points; i++) {
			int xIndex = (i >= num_points / 2) ? (num_points - 1 - i) : i;
			int yIndex = p == 1 ? (num_points - 1 - i) : i;
			zVal = (i == num_points - 1 || i == 0) ? restHeight : -deepth;
			if (p == 0) {
				path.points[i + 6*num_points] = Point{
					x: dirs[p] * xIndex * x_increment + xOffset,
					y: yIndex * y_increment + yOffset,
					z: zVal
				};
			} else {
				path.points[i + 5*num_points] = Point{
					x: dirs[p] * xIndex * x_increment + xOffset,
					y: yIndex * y_increment + yOffset,
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

	path.points[0] = Point{x: 0.0f + xOffset, y: 0.0f + yOffset, z: -deepth, feature: DRILL};
	path.points[1] = Point{x: l + xOffset, y: l + yOffset, z: -deepth, feature: DRILL};
	path.points[2] = Point{x: -l + xOffset, y: l + yOffset, z: -deepth, feature: DRILL};
	path.points[3] = Point{x: -l + xOffset, y: -l + yOffset, z: -deepth, feature: DRILL};
	path.points[4] = Point{x: l + xOffset, y: -l + yOffset, z: -deepth, feature: DRILL};
	path.points[5] = Point{x: 0.0f + xOffset, y: 0.0f + yOffset, z: -deepth, feature: DRILL};

	path.numPoints = 6;
}

void makePresetPath(char c) {
	switch (c) {
		case '0':
			lineGenerator();
			Serial.println("Line path generated!");
			break;
		case '1':
			sinGenerator();
			Serial.println("Sine wave path generated!");
			break;
		case '2':
			zigZagGenerator();
			Serial.println("Zig-zag path generated!");
			break;
		case '3':
			doubleLineGenerator();
			Serial.println("Double line path generated!");
			break;
		case '4':
			//diamondGenerator();
			rectangleGenerator();
			Serial.println("Rectangle path generated!");
			break;
		case '5':
			squareGeneratorSine();
			Serial.println("Circle path generated!");
			break;
		case '6':
			squareGeneratorMake();
			Serial.println("Wave square path generated!");
			break;

		case '7':
			circleGenerator();
			Serial.println("Circle path generated!");
			break;
		case '8':
			drillSquareGenerator();
			Serial.println("Square drill path generated!");
			break;
	}

	// Log the generated path
	logPath();
}
