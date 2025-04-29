#include <Stepper.h>

 
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;
/*
connect pins as follows:
  IN1 - pin 20
  IN2 - pin 21
  IN3 - pin 22
  IN4 - pin 23
*/

//steps per rotation, currently set to rotate vent 180 degrees @ speed 15
const int stepsPerRevolution = 955;
bool ventOpen = true;
//pins in order IN1, IN3, IN2, IN4
Stepper myStepper = Stepper(stepsPerRevolution, 20, 22, 21, 23); //change to whatever desired pins are

void setup() {
  Serial.begin(9600);
  adc_init();
}

void loop() {
  unsigned int stepperPotentiometer = adc_read(5);
  
  if(stepperPotentiometer > 450 && !ventOpen){
    openVent();
    ventOpen = !ventOpen;
  }
  else if (stepperPotentiometer <= 450 && ventOpen){
    closeVent();
    ventOpen = !ventOpen;
  }
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

void adc_init() //write your code after each commented line and follow the instruction 
{
  // setup the A register
 // set bit   7 to 1 to enable the ADC

 // clear bit 6 to 0 to disable the ADC trigger mode

 // clear bit 5 to 0 to disable the ADC interrupt

 // clear bit 0-2 to 0 to set prescaler selection to slow reading

  *my_ADCSRA |= 0b10000000;
  *my_ADCSRA &= 0b11011111;
  *my_ADCSRA &= 0b11110111;
  *my_ADCSRA &= 0b11111000;
  // setup the B register
// clear bit 3 to 0 to reset the channel and gain bits

 // clear bit 2-0 to 0 to set free running mode

  *my_ADCSRB &= 0b11110000;
  *my_ADCSRB &= 0b11111000;
  // setup the MUX Register
 // clear bit 7 to 0 for AVCC analog reference

// set bit 6 to 1 for AVCC analog reference

  // clear bit 5 to 0 for right adjust result

 // clear bit 4-0 to 0 to reset the channel and gain bits

  *my_ADMUX &= 0b01111111;
  *my_ADMUX |= 0b01000000;
  *my_ADMUX &= 0b11011111;
  *my_ADMUX &= 0b11100000;
}

unsigned int adc_read(unsigned char adc_channel_num) //work with channel 0
{
  // clear the channel selection bits (MUX 4:0)
 
  *my_ADMUX &= 0b11110000;
  // clear the channel selection bits (MUX 5) hint: it's not in the ADMUX register
  *my_ADCSRB &= 0b11011111;
 
  // set the channel selection bits for channel 0
  *my_ADMUX |= (adc_channel_num & 0x1F);

  // set bit 6 of ADCSRA to 1 to start a conversion
  *my_ADCSRA |= 0b01000000;
  // wait for the conversion to complete
  while((*my_ADCSRA & 0x40) != 0);
  // return the result in the ADC data register and format the data based on right justification (check the lecture slide)
  
  unsigned int val = (*my_ADC_DATA & 0x03FF);
  return val;
}
