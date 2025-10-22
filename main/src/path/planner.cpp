#include "planner.h"

// Load gCode path (pre-processed into a vector of GCodePoints)
void TrajectoryGenerator::resetPath(Point& goal) {
	// TODO: make this a part of the Path class
	goal = Point{0.0, 0.0, 0.0};
	segmentTime = 0.0;
	currentTime = 0.0;
	current_point_idx = 0;
}
	
// Get desired position at current time (call in control loop)
void TrajectoryGenerator::update(long deltaTime, Point& goal) {
	// TODO: bring in buffers of the path instead of just one point
	// TODO: check if path is finished
	// if (path.empty()) return;

	// TODO: check if buffer has finished. If so, update buffer

	if (running) {
		currentTime += float(deltaTime) / float(1'000'000);		// convert to seconds

		// Find the segment we're in
		while (current_point_idx < path.numPoints - 1) {
			// TODO: careful of infinite loops. Is this a problem?
			// TODO: handle the case where feedrate would result in a greater-than-max motor speed
			//	- simpler case would be to just compare feedrate to max actuation speed (combined, not single motor speed)
			//	- can I just use distanceToGo() here? Only advance when distanceToGo() is 0?

			float f = path.points[current_point_idx].f * feedrateBoost;
	
			float segmentDistance = sqrt(
				pow(path.points[current_point_idx + 1].x - path.points[current_point_idx].x, 2) +
				pow(path.points[current_point_idx + 1].y - path.points[current_point_idx].y, 2) +
				pow(path.points[current_point_idx + 1].z - path.points[current_point_idx].z, 2)
			);
			if (segmentDistance == 0.0) {
				// Serial.println("Segment distance is zero");
				current_point_idx++;
				return;
			}
			// This is the time at which the segment will be completed
			segmentTime = segmentDistance / f;
			// Serial.printf("tCurr:%f, tSeg:%f, f:%f\n", currentTime, segmentTime, feedrate);
	
			if (currentTime <= segmentTime) {
				// Linear interpolation within the segment
				// TODO: handle arc movements
				float t = currentTime / segmentTime;
				goal.x = path.points[current_point_idx].x + t * (path.points[current_point_idx + 1].x - path.points[current_point_idx].x);
				goal.y = path.points[current_point_idx].y + t * (path.points[current_point_idx + 1].y - path.points[current_point_idx].y);
				goal.z = path.points[current_point_idx].z + t * (path.points[current_point_idx + 1].z - path.points[current_point_idx].z);
				// TODO: add feedrate and feature type to goal
				return;
			} else {
				currentTime = 0.0;			// reset time for next segment
				current_point_idx++;
				// Serial.printf("Current point index: %i\n", current_point_idx);
			}
		}
	} else {
		// Serial.println("Path execution stopped");
		return;
	}

	Serial.println("Path finished!");
	// TODO: end path action
	// desPos.setZ(restHeight * ConvLead);

	// // Make sure tool is raised after path is finished
	// while (stepperZ.distanceToGo() != 0) stepperZ.run();
	
	// Reset the path accordingly
	// TODO: make this more clean/robust and remove redudant resets
	resetPath(goal);

	// Log data and close SD
	closeSDFile();

	state = ZEROED;
	if (designType == SPEED_RUN) encoderEndScreen();
	encoderDesignType();
}