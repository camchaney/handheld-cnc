// Include the AccelStepper Library
#include <AccelStepper.h>

// Define pin connections
//const int PIN_STEP_ENABLE 13;
//const int dirPin = 27;
//const int stepPin = 33;
#define MS1  21
#define MS2  14
#define PIN_STEP_ENABLE 13
//#define RX2 0
//#define TX2 1
//#define DIAG_PIN 12
#define dirPin 27
#define stepPin 33
#define SERIAL_PORT        Serial1    // TMC2208/TMC2224 HardwareSerial port
#define DRIVER_ADDRESS     0b00       // TMC2209 Driver address according to MS1 and MS2
#define R_SENSE            0.11f 
int count = 0;
int Conv = 25*64;
int myArray[] = {5,-2,2,-5};
int vel[] = {20,10,30,40};
int size = 4;
// Define motor interface type
#define motorInterfaceType 1

// Creates an instance
AccelStepper myStepper(motorInterfaceType, stepPin, dirPin);

void setup() {
  // set the maximum speed, acceleration factor,
  // initial speed and the target position
  pinMode(MS1, OUTPUT);
  digitalWrite(MS1, LOW);
  pinMode(MS2, OUTPUT);
  digitalWrite(MS2, HIGH);
  myStepper.setMaxSpeed(12800*2);
  myStepper.setAcceleration(100*16*3*10);
 // myStepper.setSpeed(200);
  //myStepper.moveTo(200);
  myStepper.setCurrentPosition(0);
 
  

}
void iterate(int arr[], int arr1[], int size){
  delay(2000);
  for (int i = 0; i<size; i++){
    myStepper.setMaxSpeed(arr1[i]*Conv);
    //myStepper.setSpeed(arr1[i]*Conv);
    myStepper.moveTo(Conv*arr[i]);
    while (myStepper.distanceToGo() != 0){
      myStepper.runSpeed();
    }
  }
}

void loop() {
  // Change direction once the motor reaches target position
//  if (myStepper.distanceToGo() == 0) 
//    myStepper.moveTo(-myStepper.currentPosition());
//
//  // Move the motor one step
//  myStepper.run();
    iterate(myArray, vel, size);
//  if (count == 5){
//    exit(0);
//  }
//  myStepper.move(-1600*8*10); //One revolution at 64 microsteps is 1600*8
//  while (myStepper.distanceToGo() != 0){
//    myStepper.run();
//  }
//  myStepper.move(1600*8*10); //One revolution at 64 microsteps is 1600*8
//  while (myStepper.distanceToGo() != 0){
//    myStepper.run();
//  }
//  count = count + 1;
}