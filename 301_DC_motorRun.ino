
volatile unsigned char *portDDRH = (unsigned char *) 0x102;//pin6 for DC motor
volatile unsigned char *portH =    (unsigned char *) 0x101;//pin6 for DC motor
//For the fan to spin forwards the following must be set:
//The pin connected to IN1 -> LOW (since this should always be low, we will not connect it to anything)
//The pin connected to IN2 -> HIGH
void setup() {
  Serial.begin(9600);
  *portDDRH |= 0x01 << 3;//sets pin6 (IN2) to OUTPUT for DC motor
  
}

void loop() {
  *portH |= 0x01<< 3; //set pin6 high to turn on DC motor
}
