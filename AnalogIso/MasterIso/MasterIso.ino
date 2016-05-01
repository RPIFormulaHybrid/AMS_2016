#include <Wire.h>

#define throttle A3
#define brake A2
#define interlock 9

void setup() {
  // put your setup code here, to run once:
  pinMode(interlock, INPUT);
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  int throttleValue = analogRead(throttle);
  int brakeValue = analogRead(brake);
  int interlockState = digitalRead(interlock);
  Wire.beginTransmission(1);
  Wire.write(map(throttleValue,0,1024,0,255));
  Wire.write(map(brakeValue,0,1024,0,255));
  Wire.write(interlockState);
  int transmissionStatus = Wire.endTransmission();
  Serial.print(transmissionStatus);
}
