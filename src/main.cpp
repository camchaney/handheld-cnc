// Libraries to include
#include <PMW3360.h>
#include <AccelStepper.h>
#include <TMCStepper.h>
#include <SPI.h>
#include <SD.h>

/*
Sensor configuration (USING 3 SENSORS NOW!!):
0 --- 1              ^ y
|  x  |   (old x <-) |-> x
2 --- (3 gone)
    NOTE: old config didn't obey the right hand rule. We are now flipping x.

Angle signage: +CCW
*/

// Function definitions
// Setup functions
void sensorSetup();
void motorSetup();
void driverSetup();
void lineGenerator();
void sinGenerator();
void circleGenerator();
// Math functions
int convTwosComp(int b);
float myDist(float x1, float y1, float x2, float y2);
float signedDist(float xr, float yr, float xg, float yg, float th);
float desPosIntersect(float xc, float yc, float th, float x3, float y3, float x4, float y4);
float desiredPosition(float dX,float dY,float theta);
float mapF(long x, float in_min, float in_max, float out_min, float out_max);
// Sensing functions
void doSensing();
// Motor control functions
void enableStepperZ();
void disableStepperZ();
void stopStepperX();
void stopStepperZ();
void machineZeroX();
void machineZeroZ();
void workZeroXY();
void workZeroZ_man();
void raiseZ();
void lowerZ();
void sensorPlotting();
void debugging();
void parseNC(const char* filename, float* pathArrayX, float* pathArrayY);
void makePath();
void DesignModeToggle();

// Pin definitions -------------------------------------------------------------------------------
// Sensor pins
#define SS0   39   // Chip select pin. Connect this to SS on the module.
#define SS1   10
#define SS2   40
//#define SS3   32
#define LIMIT_MACH_X0   8
#define LIMIT_MACH_Z0   2
#define BUTT_MACH_X0    4
#define BUTT_MACH_Z0    3
#define BUTT_WORK_X0Y0  6
#define BUTT_WORK_Z0    5
#define BUTT_HANDLE     7
#define POT_THICK       26

// Motor Pins
#define MS1_X       17
#define MS2_X       16
#define MOT_EN_X    18
#define MOT_DIR_X   14
#define MOT_STEP_X  15
#define MS1_Z       37
#define MS2_Z       38
#define MOT_EN_Z    41
#define MOT_DIR_Z   36
#define MOT_STEP_Z  33

// Driver pins
#define SERIAL_PORT_X         Serial7     // HardwareSerial port
#define SERIAL_PORT_Z         Serial6

// SD pins
const int chipSelect = BUILTIN_SDCARD;

// Constants ------------------------------------------------------------------------
// Modes
int plotting = 0;             // plot values  (1 = yes; 0 = no)
int debugMode = 1;            // print values (1 = yes; 0 = no)
int generalMode = 1;          // use general mode (general_path = 1; line_drawing = 0)
int designMode = 0;           // choose the design - from hardcode (line = 0; sine_wave = 1; circle = 2; gCode = 3)
int cheatMode = 0;            // disregard orientation for sine drawing (1 = yes; 0 = no)

// Path properties
const int num_points = 1000;             // length of path array
float pathArrayX[num_points];
float pathArrayY[num_points];
// Path properties (sine wave)
const float sinAmp = 5.0;
const float sinPeriod = 50.0;
const float pathMax_y = 300.0;            // x-length of entire path (mm) (used for line too)
// Path properties (circle)
const float circleDiameter = 800.0;       // Diameter of the circle

// Material properties
float matThickness = 0.0;                   // thickness of material
float maxThickness = 10.0;                // upper bound of thickness knob (mm)
float restHeight = 4.0;                   // rest height of tool before cutting

// Button properties
long unsigned debounceDelay = 50;      // the debounce time; increase if the output flickers

// Sensor properties
const int ns = 3;                   // number of sensors
const int CPI = 2500;               // This value changes calibration coefficients
long unsigned dt = 500;       // microseconds (freq = 1,000,000/timestepPoll [Hz])
const float Cx[3] = {0.00997506234f,0.01003310926f,0.00996611521f};
const float Cy[3] = {0.01011531459f,0.01026588646f,0.01019056354f};
const float lx = 120.0f;                // x length of rectangular sensor configuration
const float ly = 140.0f;                // y length of rectangular sensor configuration
const float xOff[3] = {-lx/2,lx/2,-lx/2};
const float yOff[3] = {ly/2,ly/2,-ly/2};

// Motor properties
// Note: Constants are in units (steps/*) whereas variables are (mm/*). Kind of
// confusing, but whenedver the variables are used within an accelStepper function,
// multiply by Conv. This can be made simpler by using SpeedyStepper library.
int uSteps = 4;                       // microstep configuration
int Conv = 25*uSteps;                 // conversion factor (mm -> steps)
float stepPulseWidth = 20.0;          // min pulse width (from Mark Rober's code)
float maxCurrent_RMS = 640.0;         // motor RMS current rating (mOhm)
// float maxVel = 6400.0;                // max velocity motor can move at (step/s)
float maxVel = 80.0*Conv;                // max velocity motor can move at (step/s)
float maxAccel = 1600.0*Conv;             // max acceleration (step/s^2)
float retract = 5;                    // distance to retract (mm)
float speed_x0 = 20.0 * Conv;             // x zeroing speed (step/s)
float speed_x1 = 4.0 * Conv;              // x secondary zeroing speed (step/s)
float accel_x0 = 200.0 * Conv;            // x zeroing acceleration (step/s^2)
#define motorInterfaceType 1

// Driver properties
#define DRIVER_ADDRESS      0b00    // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE             0.11f   // Match to your driver
int TPWMTHRS = 0x753;                  // threshold velocity for spreadCycle (at 1 mm/s right now)

// Gantry geometry
float gantryLength = 106.0;         // usable length of x-gantry (mm)
float xLimitOffset = 2.54;          // distance from wall of stepper when zeroed (mm)
float xBuffer = 3.0;                // safety buffer between tool body and walls (mm)
float zLength = 34.0;               // usable length of z-gantry (mm)
float zLimitOffset = 2.13;          // distance from wall when zeroed (mm)
float maxHeight = zLength;          // max height that z can actuate without collision
                                    // (really depends on zeroed position)

// Variables ------------------------------------------------------------------------
// Run states
int limitHitX = 0;
int limitHitZ = 0;
int cutStarted = 0;
int toolRaised = 1;

// Button states
int readyOrNot = 0;                 // (0 = workpiece is unzeroed; 1 = z is zeroed; 2 = xy is zeroed)

// Zeroing variables
int x0_count = 2;           // x zeroing count variable (start as "false")
int z0_count = 2;           // z zeroing count variable (start as "false")

// Timing variables
int firstPoint = 1;
long unsigned dtDebug = 500;                   // (ms)
long unsigned timeLastPoll = 0;
long unsigned timeLastDebug = 0;
long unsigned timeLastDebounce = 0;   // (ms)

// Measured quantities
float measVelX[3] = {0.0f,0.0f,0.0f};     // BFF x velocity of each sensor (mm/us)
float measVelY[3] = {0.0f,0.0f,0.0f};     // BFF y velocity of each sensor (mm/us)

// Estimated quantities
float estVelX[3] = {0.0f,0.0f,0.0f};      // world frame x velocity of router (mm/us)
float estVelY[3] = {0.0f,0.0f,0.0f};      // world frame y velocity of router (mm/us)
float estVelX1 = 0.0f;                    // averaged x velocity calculation (mm/us)
float estVelY1 = 0.0f;                    // averaged y velocity calculation (mm/us)
float estPosX = 0.0f;                         // router center position - x
float estPosY = 0.0f;                         // router center position - y
float estPosToolX = 0.0f;                     // tool center position - x
float estPosToolY = 0.0f;                     // tool center position - y
float estAngVel[4] = {0.0f,0.0f,0.0f,0.0f};   // angular velocity of router (rad/s)
float estAngVel1 = 0.0f;                      // averaged velocity calculation (rad/s)
float estYaw = 0.0f;                          // orientation of router (rad)
float estTraj = 0.0f;                     // (..?) trajectory angle wrt. world frame
float estVelAbs = 0.0f;                   // (unsure why used..?) absolute velocity

// Motor quantities
float motorPosX = 0.0f;                       // motor position (in mm!!)
float motorPosZ = 0.0f;                       // motor position (in mm!!)

// Control Constants ----------------------------------------------------------------
const int num_queries = 2;        // number of points to query for min distance calculation

// Control Variables ----------------------------------------------------------------
// Path following
int prev_pnt_ind = 0;
int goal_pnt_ind = 0;             // index of current goal point
float goalX = 0.0f;               // goal point x coordinate (mm)
float goalY = 0.0f;               // goal point y coordinate (mm)
float lastX = 0.0f;               // (unsure if needed) last goal point
float lastY = 0.0f;               // (unsure if needed) last goal point

// Motor control variables
float desPos = 0.0f;              // desired position of tool (mm - 0 is center of gantry)
float desVel = 0.0f;              // desired velocity of tool (mm/s)

// Object Initialization ------------------------------------------------------------
// Sensor object creation
PMW3360 sensor0;
PMW3360 sensor1;
PMW3360 sensor2;
//PMW3360 sensor3;

// Motor object creation
AccelStepper stepperX(motorInterfaceType, MOT_STEP_X, MOT_DIR_X);
AccelStepper stepperZ(motorInterfaceType, MOT_STEP_Z, MOT_DIR_Z);

// Driver objecet creation
TMC2209Stepper driverX(&SERIAL_PORT_X, R_SENSE, DRIVER_ADDRESS);
TMC2209Stepper driverZ(&SERIAL_PORT_Z, R_SENSE, DRIVER_ADDRESS);

// -------------------------------------------------------------------------------------------------
// Setup and Main Loop -----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);  
  while(!Serial);

  // Limit switch initialization
  pinMode(LIMIT_MACH_X0, INPUT);
  pinMode(LIMIT_MACH_Z0, INPUT);

  // Button initialization
  pinMode(BUTT_MACH_X0, INPUT);
  pinMode(BUTT_MACH_Z0, INPUT);
  pinMode(BUTT_WORK_X0Y0, INPUT);
  pinMode(BUTT_HANDLE, INPUT);
  //handleButton.setDebounceTime(50);

  // Setup systems
  sensorSetup();

  motorSetup(); 

  driverSetup();
  
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    return;
  }
  Serial.println("Initialization done.");

  // Make path
  makePath();

//  for (int i = 1; i < num_points; i++) {
//    Serial.printf("x(%i) = 
//  }

  Serial.println("Handheld CNC Router set up!");
  
  delay(500);
}

void loop() {
  // Serial Interface -----------------------------------------------------------------------------
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'd') {
      DesignModeToggle();
    }
  }
  
  // System Initialization ------------------------------------------------------------------------
  // If zeroing operations are needed, they are excecuted here.
  // Machine X zeroing
  if (digitalRead(BUTT_MACH_X0) == LOW) {
    x0_count = 0;
  }
  machineZeroX();

  // Machine Z zeroing
  if (digitalRead(BUTT_MACH_Z0) == LOW) {
    z0_count = 0;
    enableStepperZ();
    //Serial.println("z homing");
  }
  machineZeroZ();

  // Workpiece zeroing
  // X and Y
  if (digitalRead(BUTT_WORK_X0Y0) == LOW) {
    workZeroXY();
  }
  // Z
  if (digitalRead(BUTT_WORK_Z0) == LOW) {
    z0_count = 0;
    workZeroZ_man();
    readyOrNot = 1;
  }

  // Sensing and Control ---------------------------------------------------------------------------
  if (readyOrNot > 1) {       // keep in mind this may occur premptively b/c of X0Y0 reading
    if(micros() - timeLastPoll >= dt) {
      doSensing();
    }

    // Motor position
    motorPosX = stepperX.currentPosition() / (float)Conv;
    // Tool position
    estPosToolX = estPosX + motorPosX*cosf(estYaw);
    estPosToolY = estPosY + motorPosX*cosf(estYaw);
  
    // Control ---------------------------------------------------------------------------------
    if (digitalRead(BUTT_HANDLE) == LOW  && goal_pnt_ind + 1 < num_points &&
        abs(motorPosX) < (0.5*gantryLength - xBuffer) && !limitHitX) {
      // If statement makes sure:
      //    - user has both hands on device (BUTT_HANDLE)
      //    - the path hasn't finished (num_points)
      //    - the tool is not colliding with the wall
      //    - the tool hasn't hit a limit switch (limitHitX)

      // Lower tool
      if (toolRaised) {
        lowerZ();
      }

      // Update states
      cutStarted = 1;
      timeLastDebounce = millis();
      if (isnan(float(timeLastDebug))) {
        timeLastDebug = millis();
      }

      // Determine goal point
      if (signedDist(estPosX,estPosY,goalX,goalY,estYaw) > 0) {
        // If the goal point has been passed
        goal_pnt_ind++;
        goalX = pathArrayX[goal_pnt_ind];      // x coordinate of closest point
        goalY = pathArrayY[goal_pnt_ind];      // y coordinate of closest point
      }

      float deltaX = goalX - estPosX;               // x distance of goal from current router position
      float deltaY = goalY - estPosY;               // y distance of goal from current router position
      
      // Determine desired actuation
      if (generalMode) {
        if (!cheatMode) {
          // General path drawing
          desPos = desiredPosition(deltaX,deltaY,estYaw);
          // desPos = (deltaX - tanf(estYaw)*deltaY)*cosf(estYaw);
        } else {
          // Cheat mode sine drawing
          desPos = sinAmp * sinf((TWO_PI/sinPeriod)*estPosY);     // cheat mode
        }
        // Velocity control
        if (!isnan(estTraj)) {
          desVel = estVelAbs * cosf(estTraj - estYaw) * 1000000;    // unused right now
        }
      } else {
        // Simple line drawing
        desPos = -estPosX*cosf(estYaw);
        // Velocity control
        if (!isnan(estTraj)) {
          desVel = estVelAbs * cosf(estTraj - estYaw) * 1000000;    // unused right now
        }
      }

      // Motor actuation ---------------------------------------------------------------------------
      stepperX.moveTo(Conv*desPos);           // actuate tool to desired position
      //  if (Conv*desVel <= maxVel) {
      //    myStepper.setMaxSpeed(Conv*desVel);
      //  }
      //myStepper.setMaxSpeed(maxVel);
      if (stepperX.distanceToGo() != 0) {
        //delay(100);
        stepperX.run();
      }

      // // Save last point (not being used)
      // if (start_point != closest_point_index) {
      //   lastX = goalX;
      //   lastY = goalY;
      // }
    }
    // React to non-operational state ---------------------------------------------------------------
    else if (cutStarted  && (millis() - timeLastDebounce) > debounceDelay) {
      // USER IS NOT IN CONTROL OF DEVICE -> cancel everything
      // TO-DO: this is where we would initiate a reset after someone is done demoing

      stopStepperX();
      //readyOrNot = 0;
      //Serial.println("User not in control!");

      // Z-up
      raiseZ();

      // Reset
      cutStarted = 0;
      readyOrNot = 1;         // retains z workpiece homing, but not xy

      Serial.println("User is no longer in control");
      timeLastDebug = nanl;            // stop debug printing
    }

    if (x0_count == 2 && digitalRead(LIMIT_MACH_X0) == LOW) {
      // If X carriage runs into X limit switch
      stopStepperX();
      raiseZ();
      limitHitX = 1;

      // Reset
      cutStarted = 0;
      readyOrNot = 1;         // retains z workpiece homing, but not xy
      x0_count = 0;           // removes x machine homing
      
      Serial.println("X limit reached");
      timeLastDebug = nanl;            // stop debug printing
    }

    if (z0_count == 2 && digitalRead(LIMIT_MACH_Z0) == LOW) {
      // If Z carriage runs into Z limit switch
      stopStepperZ();
      limitHitZ = 1;

      // Reset
      cutStarted = 0;
      readyOrNot = 1;         // retains z workpiece homing, but not xy
      z0_count = 0;           // removes z workpiece/machine homing
      
      Serial.println("Z limit reached");
      timeLastDebug = nanl;            // stop debug printing
    }
    
    // Debugging -----------------------------------------------------------------------------------
    if(millis() - timeLastDebug >= dtDebug) {
      timeLastDebug = millis();
      if (debugMode) {
        debugging();
      }
    }
    //delay(10);
  }
}

// ------------------------------------------------------------------------------------------------
// Setup subfunctions -----------------------------------------------------------------------------
void sensorSetup() {
  // Sensor initialization
  if(sensor0.begin(SS0, CPI)) {  // 10 is the pin connected to SS of the module.
    Serial.print("Sensor0 initialization successed, with CPI = ");
    Serial.println(sensor0.getCPI());
  }else{
    Serial.println("Sensor0 initialization failed");
  }
  if(sensor1.begin(SS1, CPI)) {  // 10 is the pin connected to SS of the module.
    Serial.print("Sensor1 initialization successed, with CPI = ");
    Serial.println(sensor1.getCPI());
  }else{
    Serial.println("Sensor1 initialization failed");
  }
  if(sensor2.begin(SS2, CPI)) {  // 10 is the pin connected to SS of the module.
    Serial.print("Sensor2 initialization successed, with CPI = ");
    Serial.println(sensor2.getCPI());
  }else{
    Serial.println("Sensor2 initialization failed");
  }
}

void motorSetup() {
  // Set up motors
  // Initialize pins
  // pinMode(MS1_X, OUTPUT);
  // pinMode(MS2_X, OUTPUT);
  pinMode(MS1_Z, OUTPUT);
  pinMode(MS2_Z, OUTPUT);
  pinMode(MOT_EN_X, OUTPUT);
  pinMode(MOT_EN_Z, OUTPUT);

  // Initialize microstep
  // digitalWrite(MS1_X, LOW);
  // digitalWrite(MS2_X, LOW);
  digitalWrite(MS1_Z, LOW);
  digitalWrite(MS2_Z, LOW);

  // Enable motors (Mark Rober disables steppers initially..?)
  digitalWrite(MOT_EN_X, LOW);
  digitalWrite(MOT_EN_Z, LOW);

  // Disable motors
  delay(100);
  disableStepperZ();        // so that tool can be moved down manually to work surface
  //digitalWrite(MOT_EN_X, HIGH);
  //digitalWrite(MOT_EN_Z, HIGH);

  // Set motor properties
  stepperX.setMinPulseWidth(stepPulseWidth);
  stepperX.setMaxSpeed(speed_x0);
  stepperX.setAcceleration(maxAccel);
  stepperX.setCurrentPosition(0);
  stepperZ.setMinPulseWidth(stepPulseWidth);
  stepperZ.setMaxSpeed(speed_x0);
  stepperZ.setAcceleration(maxAccel);
  stepperZ.setCurrentPosition(0);
}

void driverSetup() {
  // Set up TMC2209 drivers
  SERIAL_PORT_X.begin(115200);      // INITIALIZE UART TMC2209
  SERIAL_PORT_Z.begin(115200);
  delay(100);

  driverX.begin();                // Initialize driver                   
  driverX.toff(5);                // Enables driver in software
  driverZ.begin();
  driverZ.toff(5);

  driverX.rms_current(maxCurrent_RMS);       // Set motor RMS current
  driverZ.rms_current(maxCurrent_RMS);       // Set motor RMS current
  if (uSteps > 1) {
    driverX.microsteps(uSteps);               // Set microsteps
    driverZ.microsteps(uSteps);               // Set microsteps
  } else {
    driverX.microsteps(0);
    driverZ.microsteps(0);
  }

  driverX.pwm_autoscale(true);    // Needed for stealthChop
  driverX.en_spreadCycle(false);   // false = StealthChop / true = SpreadCycle
  driverX.TPWMTHRS(TPWMTHRS);     // enable hybrid mode with velocity threshold
  driverZ.pwm_autoscale(true);    // Needed for stealthChop
  driverZ.en_spreadCycle(false);   // false = StealthChop / true = SpreadCycle
}

void lineGenerator() {
  // Generate line path to cut
  for (int i = 0; i < num_points; i++) {
    pathArrayX[i] = 0;
    pathArrayY[i] = (pathMax_y) * (float)i / (num_points - 1);
  }
}

void sinGenerator() {
  // Generate sine path to cut
  for (int i = 0; i < num_points; ++i) {
    float y = (pathMax_y) * (float)i / (num_points - 1);
    float x = sinAmp * sinf((TWO_PI/sinPeriod)*y);
    pathArrayX[i] = x;
    pathArrayY[i] = y;
  }
}

void circleGenerator() {
  // Generate a circle path to cut
  float radius = circleDiameter / 2.0;
  float angle_step = 2.0 * PI / num_points;

  for (int i = 0; i < num_points; i++) {
    float angle = i * angle_step;
    pathArrayX[i] = -radius + (radius * cosf(angle));
    pathArrayY[i] = radius * sinf(angle);
  }
}

void zigZagGenerator() {
  // Generate a zig zaxg to cut
  int numPLine = 50;
  int pCount = 0;
  // int leftRight = 1;
  float zigSize = 40;

  for (int i = 0; i < num_points; ++i) {
    if (pCount < numPLine) {
      float y = (pathMax_y) * (float)i / (num_points - 1);
      float x = std::fmod(y, zigSize);
      if (x > (zigSize / 2)) {
        x = zigSize - x;
      }
      pathArrayX[i] = x;
      pathArrayY[i] = y;
    }
  }
}

// Loop subfunctions -----------------------------------------------------------------------------
// Math functions
int convTwosComp(int b){
  // Convert from 2's complement
  // This is needed to convert the binary values from the sensor library to decimal

  if(b & 0x80){                     // 0x80 (hex) = 10000000 (bin) = 128 (dec)
    b = -1 * ((b ^ 0xff) + 1);      // 0xff (hex) = 11111111 (bin) = 255 (dec)
  }
  return b;
}

float myDist(float x1, float y1, float x2, float y2) {
  // Calculate the distance between two points

  return sqrt(pow(x1 - x2,2) + pow(y1 - y2,2));
}

float signedDist(float xr, float yr, float xg, float yg, float th) {
  // Calculate the signed distance between goal point and line of gantry
  // Note: if the distance is:
  //    < 0 - the point is in front of the gantry (it is yet to be passed)
  //    > 0 - the point is behind the gantry (it has been passed)

  float m = tan(th);
  float b = yr - m*xr;
  float A = m;
  float B = -1;
  float C = b;

  return (A*xg + B*yg + C)/sqrt(pow(A,2) + pow(B,2));
}

float desPosIntersect(float xc, float yc, float th, float x3, float y3, float x4, float y4) {
  // Determine where two lines intersect (one line will always be the gantry, made by xc, yc, and th)

  float x1 = xc - (cosf(th)*gantryLength/2);
  float y1 = yc - (sinf(th)*gantryLength/2);
  float x2 = xc + (cosf(th)*gantryLength/2);
  float y2 = yc + (sinf(th)*gantryLength/2);
  float den = (x1 - x2)*(y3 - y4) - (y1 - y2)*(x3 - x4);

  // Check for parallel lines (denominator is zero)
  if (den == 0) {
      Serial.println("Lines are parallel, no intersection point.");
      return NAN;
  }

  float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
  float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / den;
  // Check if the intersection point is on the line segments
  if (t < 0 || t > 1 || u < 0 || u > 1) {
      Serial.println("Intersection point is not on the line segments.");
      return NAN;     // TO-DO: do other stuff like stopping motor
  }
  
  float x = x1 + t * (x2 - x1);
  float y = y1 + t * (y2 - y1);
//  float pointDes[2];
//  pointDes[0] = (((x1*y2 - y1*x2)*(x3 - x4)) - ((x1 - x2)*(x3*y4 - y3*x4)))/den;
//  pointDes[1] = (((x1*y2 - y1*x2)*(y3 - y4)) - ((y1 - y2)*(x3*y4 - y3*x4)))/den;

  return -myDist(xc,yc,x,y)*cosf(th);
}

float desiredPosition(float dX,float dY,float theta) {
  // Calculate the desired position for the tool

  desPos = (dX - tanf(theta)*dY)*cosf(theta);       // Sanzhar equation

  // Alternative methods (preivous attempts):
//desPos = myDist(estPosX,estPosY,goalX,goalY)*sinf(nextTrajC);
//        if (closest_point_index == 0 && goalX == 0 && goalY == 0) {
//          desPos = 0;
//        } else {
//          desPos = desPosIntersect(estPosX,estPosY,estYaw,lastX,lastY,goalX,goalY);
//        }
// desPos = -distance(estPosX,estPosY,goalX,goalY);
// desPos = -(goalX - estPosX);
// desPos = sinAmp * sinf((TWO_PI/sinPeriod)*estPosY);     // cheat mode
  return desPos;
}

float mapF(long x, float in_min, float in_max, float out_min, float out_max) {
  // Maps a float value

  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Sensing functions
void doSensing() {
  //Serial.println(micros() - timeLastPoll);
  timeLastPoll = micros();

  // Sensing ---------------------------------------------------------------------
  // Collect sensor data (raw)
  PMW3360_DATA data0 = sensor0.readBurst_simple();
  PMW3360_DATA data1 = sensor1.readBurst_simple();
  PMW3360_DATA data2 = sensor2.readBurst_simple();
  //PMW3360_DATA data3 = sensor3.readBurst_simple();

  int yup = 1;        // filling in for good motion detection boolean
  if(yup) {
  //if(data1.isOnSurface && data1.isMotion) {   // If movement...
    // **Only checking sensor 1 rn (TODO: check all of them and account for misreads)**

    // Sensor velocity sensing
    measVelX[0] = -convTwosComp(data0.dx)*Cx[0] / dt;
    measVelX[1] = -convTwosComp(data1.dx)*Cx[1] / dt;
    measVelX[2] = -convTwosComp(data2.dx)*Cx[2] / dt;
    //measVelX[3] = -convTwosComp(data3.dx)*Cx[3] / dt;
    measVelY[0] = convTwosComp(data0.dy)*Cx[0] / dt;
    measVelY[1] = convTwosComp(data1.dy)*Cx[1] / dt;
    measVelY[2] = convTwosComp(data2.dy)*Cx[2] / dt;
    //measVelY[3] = convTwosComp(data3.dy)*Cx[3] / dt;

    // Body angle estimation
    estAngVel[0] = (measVelX[2] - measVelX[0])/ly;
    estAngVel[1] = (measVelX[2] - measVelX[1])/ly;
    estAngVel[2] = (measVelY[1] - measVelY[0])/lx;
    estAngVel[3] = (measVelY[1] - measVelY[2])/lx;
    // Simple average of angular velocities
    float sumAngVel = 0.0f;
    for (int i = 0; i<4; i++) {
      sumAngVel = sumAngVel + estAngVel[i];
    }
    estAngVel1 = sumAngVel / 4.0f;
    // Integrate angular velocity to get angle
    estYaw = estYaw + estAngVel1*dt;

    // Body position estimation
    estVelX[0] = measVelX[0]*cosf(estYaw)-measVelY[0]*sinf(estYaw) + 0.5*estAngVel1*(lx*cosf(estYaw)-ly*sinf(estYaw));
    estVelX[1] = measVelX[1]*cosf(estYaw)-measVelY[1]*sinf(estYaw) + 0.5*estAngVel1*(lx*cosf(estYaw)+ly*sinf(estYaw));
    estVelX[2] = measVelX[2]*cosf(estYaw)-measVelY[2]*sinf(estYaw) + 0.5*estAngVel1*(-lx*cosf(estYaw)-ly*sinf(estYaw));
    estVelY[0] = measVelX[0]*sinf(estYaw)+measVelY[0]*cosf(estYaw) + 0.5*estAngVel1*(ly*cosf(estYaw)+lx*sinf(estYaw));
    estVelY[1] = measVelX[1]*sinf(estYaw)+measVelY[1]*cosf(estYaw) + 0.5*estAngVel1*(-ly*cosf(estYaw)+lx*sinf(estYaw));
    estVelY[2] = measVelX[2]*sinf(estYaw)+measVelY[2]*cosf(estYaw) + 0.5*estAngVel1*(ly*cosf(estYaw)-lx*sinf(estYaw));
    // Simple average of linear velocities
    float sumVelX = 0.0f;
    float sumVelY = 0.0f;
    for (int i = 0; i<ns; i++) {
      sumVelX = sumVelX + estVelX[i];
      sumVelY = sumVelY + estVelY[i];
    }
    estVelX1 = sumVelX / ns;
    estVelY1 = sumVelY / ns;
    // Integrate linear velocities to get position
    estPosX = estPosX + estVelX1*dt;
    estPosY = estPosY + estVelY1*dt;

    // Additional values
    estTraj = atanf(estVelY1/estVelX1);    // trajectory angle w.r.t inertial frame
    estVelAbs = sqrt(pow(estVelX1,2) + pow(estVelY1,2));

    // Sensor plotting
    if (plotting) {
      sensorPlotting();
    }
  }
}

// Motor control functions
void enableStepperZ() {
  digitalWrite(MOT_EN_Z, LOW);
}

void disableStepperZ() {
  // stepperZ.disableOutputs();
  digitalWrite(MOT_EN_Z, HIGH);
}

void stopStepperX() {
  // Stop stepper motor (not using acceleration. If acceleration is desired, user library's stop() function)
  stepperX.setSpeed(0);
  stepperX.runSpeed();
}

void stopStepperZ() {
  // Stop stepper motor (not using acceleration. If acceleration is desired, user library's stop() function)
  stepperZ.setSpeed(0);
  stepperZ.runSpeed();
}

void machineZeroX() {
  if (x0_count == 0) { 
    // First x calibration run
    stepperX.setMaxSpeed(speed_x0);
    stepperX.setAcceleration(accel_x0);
    stepperX.move(Conv*gantryLength);
    stepperX.run();
    if (digitalRead(LIMIT_MACH_X0) == LOW) {
      // myStepper.setSpeed(0);              // stop motor
      stopStepperX();
      stepperX.setCurrentPosition(0);
      x0_count += 1;
      Serial.println("X limit reached");
    }
  } else if (x0_count == 1) {
    // Second x calibration run
    //myStepper.setMaxSpeed(speed_x0);
    // Retract
    stepperX.move(-Conv*retract);
    while (stepperX.distanceToGo() != 0) {
      stepperX.run();
    }
    stopStepperX();
    delay(100);

    // Move in for second calibration
    stepperX.setSpeed(speed_x1);
    while (digitalRead(LIMIT_MACH_X0) == HIGH) {
      // Run until you hit the limit switch
      stepperX.runSpeed();
    }
    stopStepperX();
    stepperX.setCurrentPosition(0);
    // Serial.println(stepperZ.currentPosition());

    // Move to middle
    stepperX.move(-Conv*((gantryLength/2) - xLimitOffset));
    while (stepperX.distanceToGo() != 0) {
      stepperX.run();
    }
    stepperX.setCurrentPosition(0);

    // Go back to standard settings
    stepperX.setMaxSpeed(maxVel);
    stepperX.setAcceleration(maxAccel);
    x0_count += 1;      // Stop limit switch function (should go back to 0 for main code)
  }

}

void machineZeroZ() {
  // Z machine zeroing operation (not really necessary when using manual workpiece zeroing)

  if (z0_count == 0) { 
    // First calibration
    //stepperZ.moveTo(Conv*retract);
    stepperZ.move(Conv*zLength);
    stepperZ.run();
    if (digitalRead(LIMIT_MACH_Z0) == LOW) {
      stopStepperZ();
      stepperZ.setCurrentPosition(0);
      z0_count += 1;
      Serial.println("Z limit reached");
    }
  } else if (z0_count == 1) {
    //stepperZ.setMaxSpeed(speed_x0);
    // Retract
    stepperZ.move(-Conv*retract);
    while (stepperZ.distanceToGo() != 0) {
      stepperZ.run();
    }
    stopStepperZ();
    delay(100);
    
    // Move in for second calibration
    stepperZ.setSpeed(speed_x1);
    while (digitalRead(LIMIT_MACH_Z0) == HIGH) {
      // Run until you hit the limit switch
      stepperZ.runSpeed();
    }
    stopStepperZ();
    stepperZ.setCurrentPosition(0);
    // Serial.println(stepperZ.currentPosition());
    
    z0_count += 1;      // Stop limit switch function (should go back to 0 for main code)
    disableStepperZ();  // disable so it can be zeroed to workpiece
  }
}

void workZeroXY() {
  if (readyOrNot == 1) {
    readyOrNot++;

    // Make sure tool is centered
    stepperX.moveTo(0);
    while (stepperX.distanceToGo() != 0) {
      stepperX.run();
    }

    // Make sure tool is down
    stepperZ.moveTo(Conv*restHeight);           // brings tool down to rest height
    while (abs(stepperZ.distanceToGo()) > (Conv*2)) {
      stepperZ.run();
    }
    stepperZ.setMaxSpeed(round(speed_x1/2));
    while (stepperZ.distanceToGo() !=0) {
      stepperZ.run();
    }
    stepperZ.setMaxSpeed(speed_x0);

    // Set all working position and orientation data to 0
    estPosX = 0;
    estPosY = 0;
    estYaw = 0;
    estPosToolX = 0;
    estPosToolY = 0;
    goal_pnt_ind = 0;           // reset path to initial point
    goalX = pathArrayX[0];
    goalY = pathArrayY[0];
  }
}

void workZeroZ_man() {
  // Manually set workpiece Z0
  // This is initiated only after the user has manually moved the tool to the top of the surface.

  enableStepperZ();
  stepperZ.setCurrentPosition(0);

  while (z0_count < 2) {
    stepperZ.move(Conv*zLength);
    if (z0_count == 0) { 
      // First calibration
      //stepperZ.moveTo(Conv*retract);
      stepperZ.move(Conv*zLength);
      stepperZ.run();
      if (digitalRead(LIMIT_MACH_Z0) == LOW) {
        stopStepperZ();
        z0_count += 1;
        Serial.println("Limit reached");
      }
    }
    if (z0_count == 1) {
      //stepperZ.setMaxSpeed(speed_x0);
      // Retract
      stepperZ.move(-Conv*retract);
      while (stepperZ.distanceToGo() != 0) {
        stepperZ.run();
      }
      stopStepperZ();
      delay(100);
      
      // Move in for second calibration
      stepperZ.setSpeed(speed_x1);
      while (digitalRead(LIMIT_MACH_Z0) == HIGH) {
        // Run until you hit the limit switch
        stepperZ.runSpeed();
      }
      stopStepperZ();
      maxHeight = stepperZ.currentPosition() / Conv;
      Serial.printf("Max height = %f", maxHeight);
      Serial.println();

      // Go back down to manually set height
      stepperZ.moveTo(Conv*restHeight);                              // go to restheight
      while (abs(stepperZ.distanceToGo()) > (Conv*2)) {
        stepperZ.run();
      }
      stepperZ.setMaxSpeed(round(speed_x1/2));
      while (stepperZ.distanceToGo() !=0) {
        stepperZ.run();
      }

      // toolRaised = 0;     // [still 1 because it is at restHeight, not -matThickness]

      stepperZ.setMaxSpeed(speed_x0);
      //disableStepperZ();
      z0_count += 1;      // Stop limit switch function (should go back to 0 for main code)
    }
  }
}

void raiseZ() {
  // Raise tool to top of gantry

  stepperZ.moveTo(Conv*(maxHeight - 1));
  while (stepperZ.distanceToGo() != 0) {
    stepperZ.run();
  }
  toolRaised = 1;
}

void lowerZ() {
  // Lower tool

  int sensorVal = analogRead(POT_THICK);
  // matThickness = mapF(sensorVal, 0, 1024, 0, maxThickness);
  matThickness = 0.0;       // for now, we'll ignore the potentiometer

  stepperZ.moveTo(-Conv*matThickness);
  while (abs(stepperZ.distanceToGo()) > (Conv*1)) {
    stepperZ.run();
  }
  stepperZ.setMaxSpeed(speed_x1);             // initial speed faster now
  while (stepperZ.distanceToGo() !=0) {
    stepperZ.run();
  }
  stepperZ.setMaxSpeed(speed_x0);
  
  toolRaised = 0;
}

void sensorPlotting() {
  // Plot sensor data

  //Serial.printf("dx:%f,dy:%f",measVelX,measVelY);
  //Serial.printf("dx:%i,dy:%i",data.dx,data.dy);
  Serial.printf("x:%f,y:%f,theta:%f",estPosX,estPosY,estYaw);
//      Serial.printf("w1:%f,w2:%f,w3:%f,w4:%f,w5:%f,w6:%f,w7:%f,w8:%f",estAngVel[0],estAngVel[1],
//        estAngVel[2],estAngVel[3],estAngVel[4],estAngVel[5],estAngVel[6],estAngVel[7]);
  //Serial.printf("x:%f,y:%f",xmm[1],ymm[1]);
  Serial.println();
}

void debugging() {
  // Print debug data
  // Put all Serial print lines here to view
  
  // Serial.printf("x:%f,y:%f,theta:%f,dist:%f",estPosX,estPosY,estYaw,signedDist(estPosX,estPosY,0,10,estYaw));
  Serial.printf("x:%f,y:%f,theta:%f,xg:%f,yg:%f,desPos:%f",estPosX,estPosY,estYaw,goalX,goalY,desPos);
  Serial.println();
  Serial.printf("thickness:%f, analog: %i",Conv*analogRead(POT_THICK), analogRead(POT_THICK));
  // Serial.printf("x:%f,y:%f,goalX:%f,goalY:%f,desPos:%i",estPosToolX,estPosToolY,goalX,goalY,desPos);
  // Serial.print(motorPosX);

  Serial.println();
}

void parseNC(const char* filename, float* pathArrayX, float* pathArrayY) {
  // NC gCode parsing function
  File myFile = SD.open(filename);
  if (!myFile) {
    Serial.println("Failed to open file for reading");
    return;
  }

  int idx = 0;
  while (myFile.available()) {
    String line = myFile.readStringUntil('\n');
    int xPos = line.indexOf('X');
    int yPos = line.indexOf('Y');
    if (xPos != -1 && yPos != -1) {
      int spacePos = line.indexOf(' ', xPos);
      // if (spacePos == -1) {
      //   spacePos = line.length();
      // }
      pathArrayX[idx] = line.substring(xPos+1, spacePos).toFloat();

      spacePos = line.indexOf(' ', yPos);
      if (spacePos == -1) {
        spacePos = line.length();
      }
      pathArrayY[idx] = line.substring(yPos+1, spacePos).toFloat();

      idx++;        // only go to update index if there is a valid XY coordinate
    }
  }

  myFile.close();
}

void makePath() {
  // Make path
  switch (designMode) {
    case 0:
      lineGenerator();
      Serial.println("Line path generated!");
      break;
    case 1:
      sinGenerator();
      Serial.println("Sine wave path generated!");
      break;
    case 2:
      circleGenerator();
      Serial.println("Circle path generated!");
      break;
    case 3:
      parseNC("generic_test02.nc", pathArrayX, pathArrayY);
      break;
    case 4:
      parseNC("stupid_wave02.nc", pathArrayX, pathArrayY);
      break;
    case 5:
      zigZagGenerator();
      break;
  }
}

void DesignModeToggle() {
  Serial.println("Start Design Mode Toggle");
  
  // Clear the serial buffer
  while (Serial.available()) {
    Serial.read();
  }
  
  while (!Serial.available()) {
    // Wait until there is data available in the serial buffer
  }
  
  int receivedNum = Serial.parseInt();
  
  Serial.print("Received num: ");
  Serial.println(receivedNum);
  
  designMode = receivedNum;
  
  makePath();
  
  Serial.println("End of Design Mode Toggle!");
}
