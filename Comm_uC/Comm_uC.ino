//RPI FH 2016 AMS communications processor
//KRP3.6.16

/*

*/


#include <SoftwareSerial.h>

#define watchdogTime 20

#define TERMINATOR 0x00
#define TESTING 0x01
#define TEMPSENSE 0x02
#define VOLTSENSE 0x03
#define CURRENTSENSE 0x04
#define READYTODRIVE 0x05
#define PLCPROGRAM 0x06 // TODO: not supported yet
#define AMSPROGRAM 0x07
#define PROGRAMCONFIRM 0x08
#define VOLSOFTSHUT 0x09
#define MANDSOFTSHUT 0x0A
#define STARTLOGAMS 0x0B
#define STARTLOGPLC 0x0C //TODO: not supported yet
#define ENDLOGAMS 0x0D
#define ENDLOGPLC 0x0E //TODO: not supported yet
#define PUMPSET 0x0F // TODO: not supported yet
#define MOTHERSTANDBY 0x10
#define MOTHERRUNNING 0x11
#define MOTHERCHARGING 0x12
#define MOTHER2CHARGER 0x13
#define CHARGER2MOTHER 0x14
#define IGNORE 0x15


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
      case 1: //STANDBY MODE, breathing blue
        //Case 1
        break;
      case 2: //BALANCE MODE, breathing yellow
        //Case 2
        break;
      case 3://RUN MODE, breathing orange
        //Case 3
        break;
      case 4: //DEBUG/TEST MODE, breathing green
        //Case 4
        break;
      case 5: //Standby warning, blinking blue
        //Case 5
        break;
      case 6: //Balance warning, blinking yellow
        //Case 6;
        break;
      case 7: //Run warning, blinking orange
        //Case 7
        break;
      case 8: //Emergency shutdown, blinking read
        //Case 8
        break;
        }
  }
}

void dataRecieve(){
  char incomingByte[64]; //Create local recieving array (no longer than serial buffer)
  memset(incomingByte,-1,sizeof(incomingByte));

  if(Serial.available() > 0)
    { //Check to see if data is in serial buffer
    for(int i=0; i<=Serial.available; i++)
      { //If so, load it into local array
      incomingByte[i] = Serial.read();
      }
    }
    parsePacket(incomingByte); //Send to the data parser
}

void parsePacket(char incomingByte[64])
{
  for(i = 0;i<64;i++)
  {
    if (incomingByte[i] == TERMINATOR) //If terminator, read next byte
    {
      i++;
      switch(incomingByte[i]){
        case TESTING:
        // Left intentionally blank
        break;
        case TEMPSENSE:
        tempSense();
        break;
        case VOLTSENSE:
        tempSense();
        break;
        case CURRENTSENSE:
        i++; //Length byte
        i++; //Current byte
        Xbee.print("Instantaneous current: ")
        Xbee.println(incomingByte[i]); //Print current information
        break;
        case READYTODRIVE:
        // Should never be recieved by COMM
        break;
        case PLCPROGRAM:
        // TODO: Not currently supported
        break;
        case AMSPROGRAM:
        // TODO : Lower priority
        break;
        case PROGRAMCONFIRM:
        // TODO: Lower priority
        break;
        case VOLSOFTSHUT:
        PLC.print(0x09,HEX);
        break;
        case MANDSOFTSHUT:
        PLC.print(0x0A,HEX)
        break;
        case STARTLOGAMS:
        startLogAMS();
        break;
        case STARTLOGPLC:
        // Not currently supported
        break;
        case ENDLOGAMS:
        // Not currently supported
        break;
        case ENDLOGPLC:
        //
        break;
        case PUMPSET:
        //
        break;
        //
        case MOTHERSTANDBY:
        //
        break;
        case MOTHERRUNNING:
        //
        break;
        case MOTHERCHARGING:
        //
        break;
        case MOTHER2CHARGER;
        //
        break;
        case CHARGER2MOTHER;
        //
        break;
        case IGNORE:
        //
        break;
      }
    }
  }
}
