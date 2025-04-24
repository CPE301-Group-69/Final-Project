#include <dht.h>
#include <LiquidCrystal.h>

// UART Pointers & Values
#define RDA 0x80
#define TBE 0x20
volatile unsigned char *myUCSR0A = (unsigned char *) 0x00C0;
volatile unsigned char *myUCSR0B = (unsigned char *) 0x00C1;
volatile unsigned char *myUCSR0C = (unsigned char *) 0x00C2;
volatile unsigned int  *myUBRR0  = (unsigned int *)  0x00C4;
volatile unsigned char *myUDR0   = (unsigned char *) 0x00C6;

volatile unsigned char* my_ADMUX = (unsigned char*) 0x7C;        
volatile unsigned char* my_ADCSRB = (unsigned char*) 0x7B;
volatile unsigned char* my_ADCSRA = (unsigned char*) 0x7A;
volatile unsigned int* my_ADC_DATA = (unsigned int*) 0x78;

//GPIO to set red LED when "water level low" error occurs
//using pin PJ1(14)
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

//initialzed at 0, previousMillis will grow bigger as time progresses (potentially getting very large hence "long")
unsigned long previousMillis = 0;
#define MINUTE 10000

//HERE WE WILL FIND OUR SPECIFIC THRESHOLD VALUE
unsigned int threshold = 60;
bool errorState = 0;

//DHT Sensor Values
dht DHT;
#define DHT_PIN 4
#define TEMP_THRES 19
bool tempHigh = 0;

volatile bool state = 0;

const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int hum1 = 0;
int hum2 = 0;
int hum3 = 0;
int temp1 = 0;
int temp2 = 0;
int temp3 = 0;

void setup() {
  U0Init(9600); //Serial initialization, same as Serial.begin(9600)

  // setup the ADC
  adc_init();
  //set portDDRJ to output
  *portDDRJ |= (0x01 << 1); //PJ1 or pin 14
  *portDDRJ |= 0x01; //PJ0 or pin 15

  *portDDRE |= (0x01 << 5); //Set PE5 or pin 3 to output (State)
  *portDDRE |= (0x01 << 3); //Set PE3 or pin 5 to output (DHT Sensor)
  *portDDRB &= ~(0x01 << 7); //Set PB6 or pin 13 to input (Reset)
  *portB &= ~(0x01 << 7); //Set PB6 or pin 13 to input (Reset)

  lcd.clear();
  lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,false);

  *portDDRE |= ~(0x01 << 4); //Set PE4 or pin 2 to 0 for input pullup (State Interrupt)
  *portE |= (0x01 << 4); //Set PE4 or pin 2 to pullup (State Interrupt)
  attachInterrupt(digitalPinToInterrupt(2), changeState, RISING);
}

void loop() {
  if(state){
    if(errorState == 0 && !tempHigh){
      *portE |= (0x01 << 5);          //Set green to high
    }
    *portJ &= ~(0x01);                //Set yellow to low

    if(*pinB & (0x01 << 7)){       //has reset been pressed?
      *portJ &= ~(0x01 << 1);      //set red LED to OFF
      *portE |= (0x01 << 5);
      errorState = 0;
      lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,false);
    }

    if(checkMinute()){

      if(adc_read(0) < threshold){     //is water level too low?
        Serial.println("Registering low");
        errorState = 1;
        lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,true);
      }

      if(errorState == 1){
        *portJ |= (0x01 << 1);       //set red LED to ON
        *portE &= ~(0x01 << 3);      //ensure blue LED is OFF
        *portE &= ~(0x01 << 5);      //ensure green LED is OFF
        putChar('l');                //output error (this will be to our LCD eventually, not serial monitor)
        putChar('o');
        putChar('w');
        putChar('\n');
        return;                      //avoid executing rest of loop, because we are in error state
      }

      if(temperatureIsHigh()){
        *portE |= (0x01 << 3); //Set PE3 or pin 5 to high
        *portE &= ~(0x01 << 5); //Set PE5 or pin 3 to low
        tempHigh = 1;
      }else{
        *portE &= ~(0x01 << 3); //Set PE3 or pin 5 to low
        *portE |= (0x01 << 5); //Set PE5 or pin 3 to high
        tempHigh = 0;
      }

      temp1 = (checkTemperature() / 100);
      temp2 = (checkTemperature() / 10) % 10;
      temp3 = checkTemperature() % 10;

      hum1 = (checkHumidity() / 100);
      hum2 = (checkHumidity() / 10) % 10;
      hum3 = checkHumidity() % 10;
      lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,false);
    }
  }  
  else{
    *portJ &= ~(0x01 << 1); //Set PJ1 or pin 14 to low
    errorState = 0;
    *portE &= ~(0x01 << 3); //Set PE3 or pin 5 to low
    *portE &= ~(0x01 << 5); //Set PE5 or pin 3 to low
    *portJ |= (0x01); //Set PJ0 or pin 15 to high
  }
}

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

void changeState() {
  state = !state;
}

bool checkMinute(){
  //sample total time elapsed with millis, update currentMillis to this
  //hardcode our minute (60000 millisecs) value which is treated as a threshold
  //update previousMillis if and when threshold is reached
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= MINUTE) {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}

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


int checkTemperature(){
  DHT.read11(DHT_PIN);
  return DHT.temperature;
}

int checkHumidity(){
  DHT.read11(DHT_PIN);
  return DHT.humidity;
}

bool temperatureIsHigh(){
  Serial.println(checkTemperature());
  if(checkTemperature() > TEMP_THRES){
    return true;
  }
  return false;
}

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