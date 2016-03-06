//RPI FH 2016 AMS communications processor
//KRP3.6.16

/*

*/


#include <SoftwareSerial.h>

SoftwareSerial Xbee = SoftwareSerial(XbeeRx, XbeeTx);
SoftwareSerial PLC = SoftwareSerial(PLCRx, PLCTx);


boolean shutdown = 0;
boolean startup = 0;



void setup(
  Serial.begin(115200);
  Xbee.begin(115200);
  PLC.begin(115200);

  pinMode(XbeeRx, INPUT);
  pinMode(XbeeTx, OUTPUT);
  pinMode(PLCRx, INPUT);
  pinMode(PLCTx, OUTPUT);

)

void loop(){
  serviceWatchdog(); //Service watchdog timer
  shutdownCheck(); //Check for emergency shutdown signal
  startupCheck(); //Check for startup signal

  dataRecieve(); //Read data from buffer
  dataTransmit(); //Send out data to different devices

  updateLED();
}

void serviceWatchdog(){
  digitalWrite(watchdog, HIGH);
  delay(watchdogTime);
  digitalWrite(watchdogPin, LOW)
}

void shutdownCheck(){
  shutdown = digitalRead(shutdownPin);
  if (shutdown){
    //TODO add appropriate shutdown functionality
  }
}

void startupCheck(){
  startup = digitalRead(startupPin);
  if(startup){
    //TODO add appropriate startup functionality
  }
}

void updateLED(){
  if(shutdown){
    color(255,0,0);
  }else{
    switch(ledStatus){
      case 1:
        //Case 1
        break;
      case 2:
        //Case 2
        break;
      case 3;
        //Case 3
        break
    }
  }
}

void dataRecieve(){
  if(Serial.available() > 0){
    for(int i=1; i<=Serial.available; i++){
      incomingByte = Serial.read();
    }
  }
}
