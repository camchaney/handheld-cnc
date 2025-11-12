#ifndef PLANNER_H
#define PLANNER_H

#include "../config.h"
#include "../globals.h"
#include "../types.h"
#include "../io/logging.h"
#include "../ui/encoder.h"

class TrajectoryGenerator {
	public:
		// Load a pre-processed gCode path
		void resetPath(Point& goal);
	
		// Update desired position based on elapsed time
		void update(long deltaTime, Point& goal);
	
	private:
		// std::vector<GCodePoint> path;
		float segmentTime = 0.0f;
		float currentTime = 0.0f;
		size_t currentIndex = 0;
	};

#endif