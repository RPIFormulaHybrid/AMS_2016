#include <Wire.h>

#define throttle 10
#define brake 9
#define interlock 6

void setup() {
  // put your setup code here, to run once:
  //`pinMode(A7, INPUT);
  pinMode(throttle, OUTPUT);
  pinMode(brake, OUTPUT);
  pinMode(interlock, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(interlock, HIGH);
  Serial.begin(9600);
  Wire.begin(1);
  Wire.onReceive(receiveEvent);
}

void loop() {
  
}

void receiveEvent(int numBytes)
{
  digitalWrite(13,LOW);
  delay(30);
  digitalWrite(13,HIGH);

  int throttleValue;
  int brakeValue;
  int interlockState;
  while (1 < Wire.available()) { // loop through all but the last
    throttleValue = Wire.read(); // receive byte as a character
    brakeValue = Wire.read();
    interlockState = Wire.read();
    Serial.println(throttleValue);         // print the character
    Serial.println(brakeValue);         // print the character
    Serial.println(interlockState);         // print the character
    Serial.println("-----------------");
  }
  float idealValue = ((.953 * ((throttleValue*5)/255) )+.015);
  float actualValue = -0.1038*pow(((throttleValue*5)/255),3)+.7993*pow(((throttleValue*5)/255),2)-.477*((throttleValue*5)/255)+.1178;
  float change = idealValue - actualValue;
  if(change <0 )
  {
    throttleValue -= floor((abs(change)*255)/5);
  }
  else
  {
    throttleValue += floor((abs(change)*255)/5);
  }
  
  analogWrite(throttle, throttleValue);
  analogWrite(brake, brakeValue);
  digitalWrite(interlock,interlockState);
}

