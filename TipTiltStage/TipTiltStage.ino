// Include the AccelStepper Library
#include <AccelStepper.h>

// Define motor interface type
#define motorInterfaceType 1

// Creates an the arrays for the steppers, the sensors, and the motor states
AccelStepper steppers[3];
int sensorPins[] = {A0, A1, A2};
boolean sensorValue[] = {false, false, false};
boolean motorMovement[] = {false, false, false};
int stepInit[3];
int state = 0; //States: 0 is nothing, 1 is leveling, 2 is eStopped

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

  Serial.begin(9600);

  //Setup the sensor pins
  pinMode(sensorPins[0], INPUT_PULLUP);
  pinMode(sensorPins[1], INPUT_PULLUP);
  pinMode(sensorPins[2], INPUT_PULLUP);

  pinMode(levelButton, INPUT_PULLUP);
  pinMode(lowerButton, INPUT_PULLUP);
  pinMode(eStopButton, INPUT_PULLUP);

}

void loop() {
buttonInput();

  //If we are in the leveling state...try to level
  if(state == 1) {
    isLevel();
    level();
  }
}

void buttonInput() {
  eStop();
  lower();
  if(digitalRead(levelButton) == LOW) {
    state = 1;
    for(int i = 0; i < 3; i++) {
      motorMovement[i] = true;
    }
  }
}

void level() {
  for(int i = 0; i < 3; i++) {
    if(!sensorValue[i]) {
      motorMovement[i] = false;
      steppers[i].moveTo(steppers[i].currentPosition() - 2);
      steppers[i].run();
    }
  }
  
  for(int i = 0; i < 3; i++) {
    if(motorMovement[i]) {
      steppers[i].moveTo(steppers[i].currentPosition() + 1);
      steppers[i].run();
    }
  }

  boolean noMotorMovement = true;
  for(int i = 0; i < 3; i++) {
    if(motorMovement[i]) noMotorMovement = false;
  }
   
  if(noMotorMovement) {
    for(int i = 0; i < 3; i++) {
      motorMovement[i] = true;
    }
  }
}

//Gets the reading from all three sensors and stores in "sensorValue" array
void getSensors() {
  for (int i = 0; i < 3; i++) {
    sensorValue[i] = digitalRead(sensorPins[i]);
  }
}

//Returns true if all sensors are touching and false if not
boolean isLevel() {
  getSensors();
  for(int i = 0; i < 3; i++) {
    if(sensorValue[i]) return false;
  }
  for(int i = 0; i < 3; i++) {
    motorMovement[i] = false;
  }
  state = 0;
  return true;
}

//Handles the emergency stop button. Causes motors to reverse to where they started. Sets state to 2 until it's home, then 0
void eStop() {
  if (digitalRead(eStopButton) == LOW) {
    motorMovement[0] = false;
    motorMovement[1] = false;
    motorMovement[2] = false;
    state = 2;
  }

  if(state == 2) {
     boolean isHome = true;
     for (int i = 0; i < 3; i++) {
      if(steppers[i].currentPosition() != stepInit[i]) {
        isHome = false;
        steppers[i].moveTo(stepInit[i]);
        steppers[i].run();
      }
    }
    if(isHome) {
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
