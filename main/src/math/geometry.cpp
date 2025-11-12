#include "geometry.h"

float myDist(float x1, float y1, float x2, float y2) {
	// Calculate the Euclidean distance between two points
	return sqrtf(powf(x1 - x2, 2.0f) + powf(y1 - y2, 2.0f));
}

float clamp(float val, float min, float max) {
	// Clamp a value between min and max
	if (val < min) {
		return min;
	}
	if (val > max) {
		return max;
	}
	return val;
}

float principalAngleRad(float x) {
	// Returns x mapped between 0 and PI
	// This assumes that the line is infinite, so angle may be 180 degrees
	// different from the original value x.
	// Ex: 3 PI/2 -> PI/2
	while (x > PI) {
		x -= PI;
	}
	while (x < 0) {
		x += PI;
	}
	return x;
}

float mapF(float x, float in_min, float in_max, float out_min, float out_max) {
	// Maps a float value from one range to another
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float signedDist(RouterPose rPose, Point p) {
	// Calculate the signed distance between point and line of gantry
	// Note: if the distance is:
	//    < 0 - the point is in front of the gantry (it is yet to be passed)
	//    > 0 - the point is behind the gantry (it has been passed)
	float m = tanf(rPose.yaw);
	float b = rPose.y - m * rPose.x;
	float A = m;
	float B = -1.0f;
	float C = b;

	return (A * p.x + B * p.y + C) / sqrtf(powf(A, 2.0f) + powf(B, 2.0f));
}

float angleFrom(Point a, Point b) {
	// Returns the angle (rads) between the gantry and the 
	// line connecting points a and b.
	float th1 = principalAngleRad(atan2f(b.y - a.y, b.x - a.x));
	// float dx = b.x - a.x;
	// float dy = b.y - a.y;
	// // float th1 = atan2f(dx, dy);			// angle from +y axis
	// float th1 = atan2f(dy,dx);

	// Using yaw here is a bit odd, since yaw is 0 when the router is in the
	// original orientation, which would actually be 90 degrees from +x. However,
	// we actually care about the angle of the gantry, which will be 0 degrees from +x
	// when the yaw is 0, so this works ok.
	float th2 = principalAngleRad(pose.yaw);
	// float th2 = pose.yaw;

	return fabsf(th1 - th2);
}

int direction(Point g, Point n) {
	// Returns the direction of the path at the given orientation
	// 1 - forward
	// -1 - backward
	// 0 - no direction

	// float angle = angleFrom(g, n);
	float angle = atan2f(n.y - g.y, n.x - g.x) - pose.yaw;
	
	if (sinf(angle) > 0) {
		return 1;
	} else if (sinf(angle) < 0) {
		return -1;
	} else {
		return 0;
	}
}
