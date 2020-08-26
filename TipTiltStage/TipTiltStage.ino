// Include the AccelStepper Library
#include <AccelStepper.h>

// Define motor interface type
#define motorInterfaceType 1

// Our global variables
AccelStepper steppers[3];                        //Creates an array of stepper objects. These correspond to the sensor pins stored below
int sensorPins[] = {A0, A1, A2};                 //The pins the hall effect sensors are connected to. These correspond to the steppers stored above
boolean sensorValue[] = {false, false, false};   //Store the last sensor reading here
boolean motorMovement[] = {false, false, false}; //Store the current state of the motors here. Once a corresponding sensor hits, this changes to false
int stepInit[3];                                 //The starting position of each motor (used for the emergency stop which moves each motor home
int state = 0;                                   //States: 0 is nothing, 1 is leveling, 2 is eStopped

//The pins for each of the buttons
int levelButton = 11;
int lowerButton = 12;
int eStopButton = 13;

void setup() {
  //Create the stepper objects and set their properties
  steppers[0] = AccelStepper(motorInterfaceType, 3, 2);
  steppers[1] = AccelStepper(motorInterfaceType, 5, 4);
  steppers[2] = AccelStepper(motorInterfaceType, 7, 6);

  for (int i = 0; i < 3; i++) {
    steppers[i].setMaxSpeed(1000);
    steppers[i].setAcceleration(50);
    steppers[i].setSpeed(200);
    stepInit[i] = steppers[i].currentPosition();
  }


  //Setup the sensor pins
  pinMode(sensorPins[0], INPUT_PULLUP);
  pinMode(sensorPins[1], INPUT_PULLUP);
  pinMode(sensorPins[2], INPUT_PULLUP);

  //Setup the button pins
  pinMode(levelButton, INPUT_PULLUP);
  pinMode(lowerButton, INPUT_PULLUP);
  pinMode(eStopButton, INPUT_PULLUP);

}

void loop() {
  buttonInput(); //Handle button presses

  //If we are in the leveling state...try to level
  if (state == 1) {
    if (isLevel()) return true; //Gets the sensor reading and checks if they're all touching. If they are it returns so we leave the loop.
    level();                    //Tries to level the stage
  }
}

//Returns true if all sensors are touching and false if not
boolean isLevel() {
  //Get all three sensor readings
  for (int i = 0; i < 3; i++) {
    sensorValue[i] = digitalRead(sensorPins[i]);
  }

  //If any of them are HIGH (TRUE) return false because it's not touching
  for (int i = 0; i < 3; i++) {
    if (sensorValue[i]) return false;
  }
  //Anything after this point means we didn't return, so the stage is level! Congratulations, you did it.

  //Sets motorMovement to all false, the state to 0, and returns true
  for (int i = 0; i < 3; i++) {
    motorMovement[i] = false;
  }
  state = 0;
  return true;
}

//Tries to level the stage
void level() {
  //Start by checking the sensor values. If any of them are touching, sets that motorMovement to false and reverses 2 steps
  for (int i = 0; i < 3; i++) {
    if (!sensorValue[i]) {
      motorMovement[i] = false;
      steppers[i].moveTo(steppers[i].currentPosition() - 2);
      steppers[i].run();
    }
  }

  //Move each motor with a true motorMovement forward one step
  for (int i = 0; i < 3; i++) {
    if (motorMovement[i]) {
      steppers[i].moveTo(steppers[i].currentPosition() + 1);
      steppers[i].run();
    }
  }

  //Checks if all thre motorMovements are false and resets them to true.
  boolean noMotorMovement = true;
  for (int i = 0; i < 3; i++) {
    if (motorMovement[i]) noMotorMovement = false;
  }
  if (noMotorMovement) {
    for (int i = 0; i < 3; i++) {
      motorMovement[i] = true;
    }
  }
}

//Handle the various button imputs
void buttonInput() {
  eStop();  //Checks and handles emergency stop presses.
  lower();  //Check sand handles lowering the stage

  //Checks if the level button has been pressed. If so, sets the state to 1 and sets the motorMovement array to all true
  if (digitalRead(levelButton) == LOW) {
    state = 1;
    for (int i = 0; i < 3; i++) {
      motorMovement[i] = true;
    }
  }
}

//Handles the emergency stop button. Causes motors to reverse to where they started. Sets state to 2 until it's home, then 0
void eStop() {
  if (digitalRead(eStopButton) == LOW) {
    motorMovement[0] = false;
    motorMovement[1] = false;
    motorMovement[2] = false;
    state = 2;
  }

  if (state == 2) {
    boolean isHome = true;
    for (int i = 0; i < 3; i++) {
      if (steppers[i].currentPosition() != stepInit[i]) {
        isHome = false;
        steppers[i].moveTo(stepInit[i]);
        steppers[i].run();
      }
    }
    if (isHome) {
      state = 0;
    }
  }
}

//Lowers the motors while the "Lower Button" is pressed, sets state to 0.
void lower() {
  if (digitalRead(lowerButton) == LOW) {
    state = 0;
    motorMovement[0] = false;
    motorMovement[1] = false;
    motorMovement[2] = false;

    for (int i = 0; i < 3; i++) {
      steppers[i].moveTo(steppers[i].currentPosition() - 1);
      steppers[i].run();
    }
  }
}
