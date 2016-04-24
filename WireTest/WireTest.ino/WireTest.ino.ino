#include <Wire.h>
#include "smb.h"

SMB testsmb1 = SMB(0x01,6);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  testsmb1.pollSMB();
  delay(20);
  
}
