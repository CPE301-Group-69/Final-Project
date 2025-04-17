
volatile unsigned char *portDDRE = (unsigned char *) 0x2d;
volatile unsigned char *portE =    (unsigned char *) 0x2e;
//For the fan to spin forwards the following must be set:
//The pin connected to IN1 -> LOW
//The pin connected to IN2 -> HIGH
void setup() {
  Serial.begin(9600);
  *portDDRE |= 0x01 << 5;//sets pin2 (IN1) to OUTPUT
  *portDDRE |= 0x01 << 4;//sets pin3 (IN2) to OUTPUT
  
}

void loop() {
  *portE &= ~(0x01 << 5); //set pin 2 low 
  *portE |= 0x01<< 4; //set pin 3 high
}
