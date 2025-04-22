#include <Stepper.h>
//connect IN1 - IN4 on driver to desired pins

//steps per rotation
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
