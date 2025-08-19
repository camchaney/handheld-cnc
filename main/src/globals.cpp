#include "globals.h"

// Initialize hardware objects
AccelStepper stepperR(motorInterfaceType, MOT_STEP_R, MOT_DIR_R);
AccelStepper stepperL(motorInterfaceType, MOT_STEP_L, MOT_DIR_L);
AccelStepper stepperZ(motorInterfaceType, MOT_STEP_Z, MOT_DIR_Z);
TMC2209Stepper driverR(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS_R);
TMC2209Stepper driverL(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS_L);
TMC2209Stepper driverZ(&SERIAL_PORT, R_SENSE, DRIVER_ADDRESS_Z);
PMW3360 sensors[4];
EncoderButton encoder(ENCODER_PIN_A, ENCODER_PIN_B, ENCODER_BUTT);
Arduino_DataBus *bus = new Arduino_HWSPI(TFT_DC, TFT_CS, &SPI1);
Arduino_GFX *screen = new Arduino_GC9A01(bus, TFT_RST, 0, true);
SdFat sd;
CompassUI ui(screen, []() { encoder.update(); });

// State variables
State state = POWER_ON;
CutState cutState = NOT_CUT_READY;
bool running = false;
bool valid_sensors = true;
DesignType designType = PRESET;
bool plungeReady = false;

// Path data
Path path;
int current_point_idx = 0;
char selectedDesignPreset = ' ';

// SD Stuff
FsFile logFile;
FsFile root;
FsFile currentDir;
String currentPath = "/";
int current_file_idx = 0;
int totalFiles = 0;
String fileList[MAX_FILES];

// Position tracking
RouterPose pose = {0.0f};
float measVel[2][4] = {{0.0f,0.0f,0.0f,0.0f},
						{0.0f,0.0f,0.0f,0.0f}};

// Calibration data
float calPos[2][4] = {{0.0f,0.0f,0.0f,0.0f},
						{0.0f,0.0f,0.0f,0.0f}};		// used for calibration
CalParams cal[4];

// Kinematics
float feedrate = feedrate_default;						// speed of tracking (mm/s)			TODO: make this modifiable (and change units to mm)
float feedrateBoost = 1;								// feedrate boost factor (i.e. 1.0 = no boost, 2.0 = double speed, etc.)

// Mode select
bool plottingOn = false;			// plot values
bool debuggingOn = false;			// print debug statements
bool stopwatchOn = false;
bool outputSerialOn = false;				// output data to serial
bool outputSDOn = true;				// output data to SD card
int designOrCalibrate = 0;			// choose design or calibrate (0 or 1)
int acceptCal = 0;					// accept calibration or not
int pauseSelection = 0;					// pause menu selection
bool autoZeroXY = true;

// gCode parameters
bool drawGCode = false;			// draw gCode, do not penetrate paper

// Preset parameters
float radius = 30.0;
float length = 30.0f;
float width = 30.0f;
float deepth = 0.0f;
float xOffset = 0.0f;
float yOffset = 0.0f;
float rotation = 0.0f;

// Timing variables
long unsigned totalLoopTime = 0;
long unsigned sensingTime_debug = 0;
long unsigned stepperTime = 0;
long unsigned serialTime = 0;
long unsigned safetyTime = 0;
long unsigned cuttingTime = 0;
long unsigned SDLogTime = 0;
long unsigned timeLoopStart = 0;
long unsigned timeLastDebounce = 0;
long unsigned lastDraw = 0;
long unsigned timeLastPoll = 0;
long unsigned sensingTime = 0;
elapsedMicros runTimer;
elapsedMicros filemicros;
elapsedMillis speedRunTimer;
uint8_t iter = 0;