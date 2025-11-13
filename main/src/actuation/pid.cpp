#include "pid.h"

PIDController::PIDController(float p, float i, float d) : Kp(p), Ki(i), Kd(d) {}

float PIDController::compute(float error, float deltaTime) {
	integral += error * deltaTime;
	float derivative = (error - prevError) / deltaTime;
	prevError = error;
	return Kp * error + Ki * integral + Kd * derivative;
}

void PIDController::reset() {
	integral = 0.0f;
	prevError = 0.0f;
}