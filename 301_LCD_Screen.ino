// VSS Ground
// VDD Pos
// V0 to middle pin of potentiometer
// LCD RS pin to digital pin 7
// RW Ground
// LCD Enable pin to digital pin 8
// LCD D4 pin to digital pin 9
// LCD D5 pin to digital pin 10
// LCD D6 pin to digital pin 11
// LCD D7 pin to digital pin 12
// A to pos
// K to ground

#include <LiquidCrystal.h>

const int rs = 7, en = 8, d4 = 9, d5 = 10, d6 = 11, d7 = 12;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int hum1 = 0;
int hum2 = 0;
int hum3 = 0;
int temp1 = 0;
int temp2 = 0;
int temp3 = 0;
bool watLvlLow = false;

void setup() {
  // put your setup code here, to run once:
  lcd.clear();
  lcdDisplay(hum1,hum2,hum3,temp1,temp2,temp3,watLvlLow);
}

void loop() {
  // Turn off the display:
  //lcd.noDisplay();
  //delay(500);
  // Turn on the display:
  //lcd.display();
  //delay(500);
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

