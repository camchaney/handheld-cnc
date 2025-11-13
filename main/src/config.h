#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Pin definitions
#define SS0   				30
#define SS1					10
#define SS2					31
#define SS3					32
#define LIMIT_MACH_X0       5
#define LIMIT_MACH_Y0		6
#define LIMIT_MACH_Z0       7
#define BUTT_HANDLE_L       2
#define BUTT_HANDLE_R       3
#define ENCODER_PIN_A       22
#define ENCODER_PIN_B       21
#define ENCODER_BUTT	  	4
#define MOT_EN_L    		23
#define MOT_DIR_L   		18
#define MOT_STEP_L  		19
#define MOT_EN_R    		15
#define MOT_DIR_R   		38
#define MOT_STEP_R  		39
#define MOT_EN_Z    		41
#define MOT_DIR_Z   		36
#define MOT_STEP_Z  		33
#define TFT_CS      		29
#define TFT_DC      		28
#define TFT_RST     		25
#define TFT_BL				24

// Driver settings
#define DRIVER_ADDRESS_L	0b00
#define DRIVER_ADDRESS_R  	0b01
#define DRIVER_ADDRESS_Z	0b10
#define R_SENSE            	0.11f
#define SERIAL_PORT     	Serial4
#define motorInterfaceType 	1

// Constants
// #define MAX_PATHS  			10
#define MAX_POINTS 			8000
#define MAX_FILES			100
#define GC9A01A_WEBWORK_GREEN 0x8FF1

// Sensor properties
const int ns = 4;								// number of sensors
const int CPI = 2500;							// counts per inch. This value changes calibration coefficients
const float lx = 120.0f;						// x length of rectangular sensor configuration (mm)
const float ly = 140.0f;						// y length of rectangular sensor configuration (mm)
const float xSensorOffset = -2.32f;				
const float ySensorOffset = -3.2f;				// offset to try to counteract weird rotation behavior (mm) (UNUSED)
const float driftRate = 0.001f;					// drift rate of positioning (unitless, drift per distance traveled - measured)
const float maxDrift = 1.0f;					// maximum acceptable drift (mm)

// Motor properties
const int stepsPerRev = 200;					// steps per revolution
const int uSteps = 4;							// microstep configuration
// A and B Motors (belt)
const float beltPitch = 2.0f;					// belt pitch (mm)
const int pulleyTeeth = 16;						// pulley teeth on drive pulley
const float ConvBelt = (stepsPerRev*uSteps)/(beltPitch*pulleyTeeth);	// conversion factor (mm -> steps)
// Z Motor (lead screw)
const float lead = 8.0f;							// lead screw lead (mm)
const float ConvLead = (stepsPerRev*uSteps)/lead;	// conversion factor (mm -> steps)
// Other motor properties
const float stepPulseWidth = 20.0f;				// min pulse width (from Mark Rober's code)
const float maxCurrent_RMS = 1273.0f;			// motor RMS current rating (mA)
const float maxSpeedAB = 140.0f*ConvBelt;		// max velocity A and B motors can move at (step/s)
const float maxAccelAB = 2000.0f*ConvBelt;		// A/B motor max acceleration (step/s^2)
const float maxSpeedZ = 180.0f*ConvLead;		// max velocity Z motor can move at (step/s)
const float maxAccelZ = 3000.0f*ConvLead;		// z max acceleration (step/s^2)
const float retract = 2.0f;						// distance to retract (mm)
const float zeroSpeed_0 = 20.0f;				// zeroing speed (mm/s)
const float zeroSpeed_1 = 4.0f;					// secondary zeroing speed (mm/s)
const float zeroAccel = 200.0f;					// zeroing acceleration (mm/s^2)
const float feedrate_default = 5.0f;			// default feedrate (mm/s)
const float plungeRate_default = 5.0f;			// default plunge rate (mm/s)

// Material properties
const float maxThickness = 15.0f;				// upper bound of thickness knob (mm)
const float restHeight = 2.0f;					// rest height of tool before cutting (mm)
const float holeTolerance = 0.25f;				// tolerance for hole positioning (mm)

// Gantry geometry
const float xRange = 20.0f;						// usable range of x-axis (mm)
const float xRangeHard = 24.0f;					// hard limit of x-axis (mm)
const float yRange = 20.0f;						// usable range of y-axis (mm)
const float yRangeHard = 24.0f;					// hard limit of y-axis (mm)
const float zRange = 34.0f;						// usable length of z-axis (mm)
const float zRangeHard = 34.0f;					// hard limit of z-axis (mm)
const float xLimitOffset = 1.32f;				// distance from wall of stepper when zeroed (mm)
const float yLimitOffset = 1.48f;				// distance from wall of stepper when zeroed (mm)
const float zLimitOffset = 2.29f;				// distance from wall when zeroed (mm)
const float wallBuffer = 3.0f;					// safety buffer between tool body and walls (mm)

// Timing constants
const long unsigned debounceDelay = 50;			// the debounce time; increase if the output flickers
const long unsigned dtDebug = 500;				// (ms)
const long unsigned dtStopwatch = 500;			// time interval for loop timing (ms)
const long unsigned dtPlot = 50;				// (ms)
const long unsigned dtOutput = 20;				// (ms)
const long unsigned dtOutputSD = 10;			// time interval for auxiliary SD logging (ms)
const long unsigned dt = 900;					// length of sensor timestep (us)(freq = 1,000,000/timestepPoll [Hz])
const long unsigned dtControl = 500;			// length of control timestep (us)(freq = 1,000,000/timestepControl [Hz])

// Auto Touch constants
const float maxAutoTouchCurrent_RMS = 300.0f;		// motor RMS current rating (mA)
const long unsigned autoTouchStepInterval = 600;	// step interval for auto touch (us)
const short unsigned autoTouchThreshold = 50;		// auto touch threshold (SG value)
const float autoTouchRetraction = 1.4f;				// auto touch retraction (to compensate flex coupler play)

#endif
