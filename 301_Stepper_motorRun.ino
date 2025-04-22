#include <Stepper.h>
/*
connect pins as follows:
  IN1 - pin 20
  IN2 - pin 21
  IN3 - pin 22
  IN4 - pin 23
*/

//steps per rotation, currently set to rotate vent 180 degrees @ speed 15
const int stepsPerRevolution = 955;
//pins in order IN1, IN3, IN2, IN4
Stepper myStepper = Stepper(stepsPerRevolution, 20, 22, 21, 23); //change to whatever desired pins are

void setup() {
  
}

void loop() {
  openVent();
  closeVent();
}

void openVent(){
  myStepper.setSpeed(15);
  myStepper.step(stepsPerRevolution);
  delay(1000);
  myStepper.step(0);
}

void closeVent(){
  myStepper.setSpeed(15);
  myStepper.step(-stepsPerRevolution);
  delay(1000);
  myStepper.step(0);
}
