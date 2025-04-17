
volatile unsigned char *portDDRE = (unsigned char *) 0x2d;
volatile unsigned char *portE =    (unsigned char *) 0x2e;
// int dir1=4;
// int dir2=3;
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  // pinMode(dir1,OUTPUT);
  // pinMode(dir2,OUTPUT);
  *portDDRE |= 0x01 << 5;//sets pinB3(dir2) to OUTPUT
  *portDDRE |= 0x01 << 4;//sets pinB4(dir1) to OUTPUT
  
}

void loop() {
  // put your main code here, to run repeatedly:
  // digitalWrite(3,LOW);
  // digitalWrite(4,HIGH);
  *portE &= ~(0x01 << 5); // dir
  *portE |= 0x01<< 4;
}
