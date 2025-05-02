//Authors: Dylan Seibel, Adam Montgomery, Jackson Loughmiller, and Jackson Palmer
//Date: 05/06/2025

#include <dht.h>
#include <LiquidCrystal.h>
#include <Stepper.h>
#include <RTClib.h>

// UART Pointers and Values
#define RDA 0x80
#define TBE 0x20
volatile unsigned char *myUCSR0A = (unsigned char *) 0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *) 0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *) 0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *)  0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *) 0x00C6;

//ADC Pointers and Values
volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;        
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

//Error State LED and Idle LED Pointers (Pin 14 and 15)
volatile unsigned char* portDDRJ  = (unsigned char*) 0x104; 
volatile unsigned char* portJ = (unsigned char*) 0x105;

//State Register Pointers (Pin 2, 3, and 5)
volatile unsigned char* portE = (unsigned char*) 0x2E;
volatile unsigned char* portDDRE = (unsigned char*) 0x2D;
volatile unsigned char* pinE = (unsigned char*) 0x2C;

//Reset Register Pointers (Pin 10-13)
volatile unsigned char* portB = (unsigned char*) 0x25; 
volatile unsigned char* portDDRB  = (unsigned char*) 0x24; 
volatile unsigned char* pinB  = (unsigned char*) 0x23;

//DC Motor Register Pointers (Pin 6)
volatile unsigned char *portH =    (unsigned char *) 0x102;//pin6 for DC motor
volatile unsigned char *portDDRH = (unsigned char *) 0x101;//pin6 for DC motor

//Stepper Motor Button Pointer
volatile unsigned char* portL = (unsigned char*) 0x10B; 
volatile unsigned char* portDDRL  = (unsigned char*) 0x10A; 
volatile unsigned char* pinL = (unsigned char*) 0x109;

//State Initialization
volatile bool state = 0;

//Minute Check Global Variables
unsigned long previousMillis = 0;
#define MINUTE 60000

//Water Level Threshold and State
unsigned int threshold = 150;
bool errorState = 0;

//Stepper Motor Global Variables: currently set to rotate vent 180 degrees @ speed 15
const int stepsPerRevolution = 955;
bool ventOpen = true;
Stepper myStepper = Stepper(stepsPerRevolution, 42, 47, 43, 48);

//DHT Sensor Values
dht DHT;
#define DHT_PIN 4
#define TEMP_THRES 21
bool tempHigh = 0;

//LCD Initialization
const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int hum1 = 0;
int hum2 = 0;
int hum3 = 0;
int temp1 = 0;
int temp2 = 0;
int temp3 = 0;

//RTC Global Variables (with State Detection)
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
"Friday", "Saturday"};
unsigned int currentState = -1;
unsigned int prevState = -1;

void setup() {
  U0Init(9600);

  // Setup the ADC, LCD, and RTC
  adc_init();
  lcd.clear();
  lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,false);
  rtc.begin();
  currentState = 0;

  // Set all I/O
  *portDDRJ |= (0x01 << 1); //pin 14 (Red LED/ERROR)
  *portDDRJ |= 0x01; //pin 15 (Yellow LED/DISABLED)
  *portDDRE |= (0x01 << 5); //pin 3 (Green LED/IDLE)
  *portDDRE |= (0x01 << 3); //pin 5 (Blue LED/RUNNING)
  *portDDRB &= ~(0x01 << 7); //pin 13 (Reset Button)
  *portB &= ~(0x01 << 7); //pin 13 (Reset Button)
  *portDDRH |= 0x01 << 3; //pin 6 (DC Motor)
  *portH &= ~(0x01<< 3); //pin 6 (DC Motor OFF)
  *portDDRL &= ~(0x01); //pin 49 (Stepper Motor Button)

  // Configure Interrupt Input (on pin 2)
  *portDDRE |= ~(0x01 << 4); //input pullup (State Interrupt)
  *portE |= (0x01 << 4); //input pullup (State Interrupt)
  attachInterrupt(digitalPinToInterrupt(2), changeState, RISING);
}

void loop() {
  //Check for state change and output to serial monitor
  if(currentState != prevState){
    DateTime now = rtc.now();
    putChar('\n');
    putChar('S');
    putChar('t');
    putChar('a');
    putChar('t');
    putChar('e');
    putChar(' ');
    putChar('c');
    putChar('h');
    putChar('a');
    putChar('n');
    putChar('g');
    putChar('e');
    putChar('d');
    putChar(' ');
    putChar('t');
    putChar('o');
    putChar(':');
    putChar(' ');
    
    switch(currentState){
      case 0:
        putChar('D');
        putChar('I');
        putChar('S');
        putChar('A');
        putChar('B');
        putChar('L');
        putChar('E');
        putChar('D');
        break;

      case 1:
        putChar('I');
        putChar('D');
        putChar('L');
        putChar('E');
        break;
      
      case 2:
        putChar('R');
        putChar('U');
        putChar('N');
        putChar('N');
        putChar('I');
        putChar('N');
        putChar('G');
        break;

      case 3:
        putChar('E');
        putChar('R');
        putChar('R');
        putChar('O');
        putChar('R');
        break;

      default:
        putChar('U');
        putChar('N');
        putChar('K');
        putChar('N');
        putChar('O');
        putChar('W');
        putChar('N');
        break;
    }

    putChar('\n');
    putChar('a');
    putChar('t');
    putChar(' ');
    putChar((now.hour() / 10) + 48);
    putChar((now.hour() % 10) + 48);
    putChar(':');
    putChar((now.minute() / 10) + 48);
    putChar((now.minute() % 10) + 48);
    putChar(':');
    putChar((now.second() / 10) + 48);
    putChar((now.second() % 10) + 48);
    putChar(' ');
    putChar('o');
    putChar('n');
    putChar(' ');
    putChar((now.month() / 10) + 48);
    putChar((now.month() % 10) + 48);
    putChar('/');
    putChar((now.day() / 10) + 48);
    putChar((now.day() % 10) + 48);
    putChar('/');
    putChar((now.year() / 1000) + 48);
    putChar(((now.year() / 100) % 10) + 48); 
    putChar(((now.year() / 10) % 100) + 48); 
    putChar(((now.year() % 100) % 10) + 48);
  
    prevState = currentState;
  }

  //If state is true run all required checks, else turn off everything but yellow light
  if(state){
    //Check if vent button has been pressed 
    if(*pinL & (0x01)){
      if (!ventOpen){
        openVent();
      }
      else{
        closeVent();
      }
      ventOpen = !ventOpen;
    }

    //Check if reset button has been pressed
    if(*pinB & (0x01 << 7)){
      errorState = 0;
      currentState = 1;
    }

    //Check water level threshold and trigger error state.
    if(adc_read(0) < threshold){
      errorState = 1;
      currentState = 3;
    }

    //Do this if in error state
    if(errorState == 1){
      *portH &= ~(0x01<< 3);
    }

    //Reset properly from error state
    if(errorState == 0 && !tempHigh){
      currentState = 1;
    }

    //Check tempature if not in error state and trigger running state
    if(temperatureIsHigh() && !errorState){
      *portH |= 0x01<< 3;
      tempHigh = 1;
      currentState = 2; 
    }else if(!temperatureIsHigh()){
      *portH &= ~(0x01<< 3);
      tempHigh = 0;
    }
    
    //Trigger Light Changes
    switch(currentState){
      case 0:
        *portJ |= (0x01);
        *portE &= ~(0x01 << 3);
        *portE &= ~(0x01 << 5);
        *portJ &= ~(0x01 << 1);
        break;
      
      case 1:
        *portJ &= ~(0x01);
        *portE &= ~(0x01 << 3);
        *portE |= 0x01 << 5;
        *portJ &= ~(0x01 << 1);
        break;

      case 2:
        *portJ &= ~(0x01);
        *portE |= 0x01 << 3;
        *portE &= ~(0x01 << 5);
        *portJ &= ~(0x01 << 1);
        break;

      case 3:
        *portJ &= ~(0x01);
        *portE &= ~(0x01 << 3);
        *portE &= ~(0x01 << 5);
        *portJ |= 0x01 << 1;
        break;
    }

    //Check if a minute has passed for LCD updates
    if(checkMinute()){
      if(errorState == 1){
        lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,true);
      }
      else{
        temp1 = (checkTemperature() / 100);
        temp2 = (checkTemperature() / 10) % 10;
        temp3 = checkTemperature() % 10;
        hum1 = (checkHumidity() / 100);
        hum2 = (checkHumidity() / 10) % 10;
        hum3 = checkHumidity() % 10;
        lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,false);
      }
    }
  }  
  else{
    *portJ &= ~(0x01 << 1);
    errorState = 0;
    *portE &= ~(0x01 << 3);
    *portE &= ~(0x01 << 5);
    *portJ |= (0x01);
    *portH &= ~(0x01<< 3);
    currentState = 0;
  }
}

//UART Methods
void U0Init(int U0baud){
  unsigned long FCPU = 16000000;
  unsigned int tbaud;
  tbaud = (FCPU / 16 / U0baud - 1);
  *myUCSR0A = 0x20;
  *myUCSR0B = 0x18;
  *myUCSR0C = 0x06;
  *myUBRR0  = tbaud;
}

unsigned char kbhit() {
  if(*myUCSR0A & RDA) {
    return 1;
  }
  return 0;
}

unsigned char getChar() {
  unsigned char character = *myUDR0;
  return character;
}

void putChar(unsigned char U0pdata) {
  while(!(*myUCSR0A & TBE));
  *myUDR0 = U0pdata;
}

//Interrrupt method
void changeState() {
  state = !state;
}

//Check if a minute has passed and return a bool
bool checkMinute(){
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MINUTE) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

//ADC methods
void adc_init(){
  *my_ADCSRA |= 0b10000000;
  *my_ADCSRA &= 0b10111111;
  *my_ADCSRA &= 0b11011111;
  *my_ADCSRA &= 0b11111000;
  *my_ADCSRB &= 0b11110111;
  *my_ADCSRB &= 0b11111000;
  *my_ADMUX &= 0b01111111;
  *my_ADMUX |= 0b01000000;
  *my_ADMUX &= 0b11011111;
  *my_ADMUX &= 0b11100000;
}

unsigned int adc_read(unsigned char adc_channel_num){
  *my_ADMUX &= 0b11110000;
  *my_ADCSRB &= 0b11110111;
  *my_ADMUX += adc_channel_num;
  *my_ADCSRA |= 0b01000000;
  while((*my_ADCSRA & 0x40) != 0);
  unsigned int val = *my_ADC_DATA & 0x03FF;
  return val;
}

//Check tempature and return an int
int checkTemperature(){
  DHT.read11(DHT_PIN);
  return DHT.temperature;
}

//Check humidity and return an int
int checkHumidity(){
  DHT.read11(DHT_PIN);
  return DHT.humidity;
}

//Check if tempature is high and return a bool
bool temperatureIsHigh(){
  if(checkTemperature() > TEMP_THRES){
    return true;
  }
  return false;
}

//Trigger stepper motor
void openVent(){
  myStepper.setSpeed(15);
  myStepper.step(stepsPerRevolution);
  delay(1000);
  myStepper.step(0);
}

//Reverse trigger stepper motor
void closeVent(){
  myStepper.setSpeed(15);
  myStepper.step(-stepsPerRevolution);
  delay(1000);
  myStepper.step(0);
}

//Update LCD with passed in values
void lcdDisplay(int h1, int h2, int h3, int t1, int t2, int t3,bool wLvl){
  if(wLvl == false){
    lcd.begin(16,2);
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Humid Lvl: ");
    lcd.print(h1);
    lcd.print(h2);
    lcd.print(h3);
    lcd.print("%");
    lcd.setCursor(0,1);
    lcd.print("Temp Lvl:  ");
    lcd.print(t1);
    lcd.print(t2);
    lcd.print(t3);
  }
  else{
    lcd.clear();
    lcd.print("Water Level Low!");
  }
}