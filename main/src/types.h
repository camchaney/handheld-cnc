#ifndef TYPES_H
#define TYPES_H

#include "config.h"

extern float maxHeight;	// max height of the workspace (mm)

// State values
typedef enum State {
	// TODO: reorganize these states and make strict rules for updating them
	POWER_ON,
	MACHINE_XY_ZERO,
	WORKSPACE_Z_ZERO,
	ZEROED,
	THICKNESS_SET,
	DOC_SELECTED,
	CALIBRATION,
	CALIBRATION_ADVANCE,
	TYPE_SELECTED,
	SELECTING_DESIGN,
	DESIGN_SELECTED,
	WORKSPACE_XY_ZERO,
	READY,
	STANDBY
} State;

typedef enum CutState {
	NOT_CUT_READY,
	NOT_USER_READY,
	CUT_READY,
	CUTTING,
	PLUNGING,
	RETRACTING
} CutState;

// Type of design
typedef enum DesignType {
	PRESET,
	FROM_FILE,
	SPEED_RUN
 } DesignType;

// Feature type
typedef enum Feature {
	// TODO: add types for travel, contour, pocket, etc.
	NORMAL,
	DRILL,
} Feature;

// Router pose
typedef struct RouterPose {
	float x;			// x position
	float y;			// y position
	float yaw;			// orientation
	// Future additions like velocity, acceleration, etc.
} RouterPose;

class Position {
	// This is a class to represent a 3D position for the tool to move to (in the router's BFF coordinate system)
	// The position is clamped to the workspace limits

	public:
		Position(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {
			clamp();
		}
	
		bool set(float newX, float newY, float newZ) {
			x = newX;
			y = newY;
			z = newZ;
			return clamp();
		}
	
		bool setX(float newX) {
			x = newX;
			return clamp();
		}
	
		bool setY(float newY) {
			y = newY;
			return clamp();
		}
	
		bool setZ(float newZ) {
			z = newZ;
			return clamp();
		}
	
		float getX() const { return x; }
		float getY() const { return y; }
		float getZ() const { return z; }
	
	private:
		float x, y, z;
	
		bool clamp() {
			bool validMotionX = false;
			bool validMotionY = false;
			bool validMotionZ = false;
			x = constrainValue(x, -xRange / 2, xRange / 2, validMotionX);
			y = constrainValue(y, -yRange / 2, yRange / 2, validMotionY);
			z = constrainValue(z, maxHeight - zRange + zLimitOffset, maxHeight, validMotionZ);
			if (validMotionX && validMotionY && validMotionZ) return true;
			return false;
		}
	
		float constrainValue(float value, float min, float max, bool& b) {
			if (value < min) return min;
			if (value > max) return max;
			b = true;
			return value;
		}
};

// Calibration parameters
typedef struct CalParams {
	float x = 25.4 / CPI;		// x scalar (default based on sensor CPI)
	float y = 25.4 / CPI;		// y scalar (default based on sensor CPI)
	float r = 0.0f;				// sensor rotation
} CalParams;

// Coordinate
typedef struct Point {
	float x;				// target position (mm)
	float y;				// target position (mm)
	float z;				// target position (mm)
	float f = feedrate_default;			// feedrate (mm/s)
	Feature feature = NORMAL;	// type of feature
} Point;

// Path
typedef struct Path {
	// TODO: add other properties such as angle, etc. (more info in notebook)
	Point points[MAX_POINTS] = {NAN};
	int numPoints = MAX_POINTS;
} Path;

#endif
