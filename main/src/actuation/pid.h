#ifndef PID_H
#define PID_H

class PIDController {
public:
	// Constructor with PID gains
	PIDController(float Kp, float Ki, float Kd);

	// Compute PID output
	float compute(float error, float deltaTime);

	// Reset integral and derivative terms
	void reset();

private:
	float Kp, Ki, Kd;
	float integral = 0.0f;
	float prevError = 0.0f;
};

#endif