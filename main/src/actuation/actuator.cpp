#include "actuator.h"

ActuationController::ActuationController(Position &dp)
    : desPos(&dp), pidX(1.0f, 0.1f, 0.01f), pidY(1.0f, 0.1f, 0.01f), pidZ(1.0f, 0.1f, 0.01f) {
}

// Compute actuator adjustments (call in control loop)
void ActuationController::update(
	long deltaTime,
	Point goal,
	RouterPose currentPose
) {
	// Global position error
	float errX_global = goal.x - currentPose.x;
	float errY_global = goal.y - currentPose.y;
	float errZ_global = goal.z;								// TODO: does this need to be implemented differently?

	// Coordinate transform (global error -> local actuator frame)
	float errX_local = errX_global * cosf(currentPose.yaw) + errY_global * sinf(currentPose.yaw);
	float errY_local = -errX_global * sinf(currentPose.yaw) + errY_global * cosf(currentPose.yaw);

	// PID control
	// desPos.setX(pidX.compute(errX_local, deltaTime));
	// desPos.setY(pidY.compute(errY_local, deltaTime));
	// desPos.setZ(pidZ.compute(errZ_global, deltaTime));		// TODO: does this need to be implemented differently?

	// Normal control (without PID)
	validMotion = desPos->set(errX_local, errY_local, errZ_global);
}
