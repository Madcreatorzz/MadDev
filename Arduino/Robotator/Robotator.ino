/* 
Demonstrates use of rotary encoder for motor
direction and distance.
*/

#define CHA 2
#define CHB 3
#define CW_LED 8
#define CCW_LED 7

volatile int master_count = 0; // universal count
volatile byte INTFLAG1 = 0; // interrupt status flag

void setup() { 
  pinMode(CHA, INPUT);
  pinMode(CHB, INPUT);
  pinMode(CW_LED, OUTPUT); // LED connected to pin to ground
  pinMode(CCW_LED, OUTPUT); // LED connected to pin to ground
  
  Serial.begin(9600); 
  Serial.println(master_count);
  
  attachInterrupt(0, flag, RISING);  
  // interrupt 0 digital pin 2 positive edge trigger
}

void loop() {

  if (INTFLAG1)   {
       Serial.println(master_count);
       delay(500);
     INTFLAG1 = 0; // clear flag
} // end if


} // end loop


void flag() {
  INTFLAG1 = 1;
  // add 1 to count for CW
  if (digitalRead(CHA) && !digitalRead(CHB)) {
    master_count++ ;
    digitalWrite(CW_LED, HIGH);
    digitalWrite(CCW_LED, LOW);
  }
  // subtract 1 from count for CCW
  if (digitalRead(CHA) && digitalRead(CHB)) {
    master_count-- ;
    digitalWrite(CW_LED, LOW);
    digitalWrite(CCW_LED, HIGH);
  } 
}
