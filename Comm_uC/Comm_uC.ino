//RPI FH 2016 AMS communications processor
//KRP3.6.16
//SKM4.28.16
#include <SoftwareSerial.h>

//Serial Protocol Defines
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

#define watchdogTime 20

//LED STATUS DEFINE
#define STANDBY 1
#define BALANCE 2
#define RUN 3
#define DEBUG 4
#define STANDBYERROR 5
#define BALANCEERROR 6
#define RUNERROR 7
#define EMERGENCY 8

//pin defines
#define motherRx 11
#define motherTx 12
#define xbeeRx 10
#define xbeeTx 8
#define ledRed 5
#define ledGreen 6
#define ledBlue 9
#define watchdog 7
#define shutdownPin 13
#define startupPin A0

boolean shutdown = 0;
boolean startup = 0;
char ledStatus = 0;
unsigned long ledTiming = 0;
unsigned long currentTime = 0;

void setup()
{
  Serial.begin(9600); //PLC Serial Comms

  pinMode(ledRed, OUTPUT);
  pinMode(ledBlue, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(watchdog, OUTPUT);
  pinMode(startupPin, INPUT);
  pinMode(motherRx, INPUT);
  pinMode(motherTx, OUTPUT);

  digitalWrite(watchdog, HIGH);
  digitalWrite(motherTx, LOW);

  for(int i =0;i<3;i++)
  {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledBlue, LOW);
    delay(100);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledBlue, LOW);
    delay(100);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, LOW);
    digitalWrite(ledBlue, HIGH);
  }
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledBlue, HIGH);
  delay(60);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledBlue, LOW);
  delay(50);
  digitalWrite(ledRed, HIGH);
  digitalWrite(ledGreen, HIGH);
  digitalWrite(ledBlue, HIGH);
  delay(60);
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(ledBlue, LOW);

  ledStatus = STANDBY;
}

void loop()
{
  serviceWatchdog(); //Service watchdog timer
  currentTime = millis();
  shutdownCheck(); //Check for emergency shutdown signal
  startupCheck(); //Check for startup signal

  if(currentTime - ledTiming >=25)
  {
    ledTiming = currentTime;
    updateLED();
  }
}

void serviceWatchdog(){
  digitalWrite(watchdog, LOW);
  delay(watchdogTime);
  digitalWrite(watchdog, HIGH);
}

void shutdownCheck(){
  shutdown = digitalRead(shutdownPin);
  if (shutdown){
    //TODO add appropriate shutdown functionality
  }
}

void startupCheck(){
  startup = digitalRead(startupPin);
  if(startup)
  {
    digitalWrite(motherTx, HIGH);
    if(digitalRead(motherRx))
      ledStatus = RUN;
    else
      ledStatus = STANDBY;
  }
  else
  {
    digitalWrite(motherTx, LOW);
    if(digitalRead(motherRx))
      ledStatus = RUN;
    else
      ledStatus = STANDBY;
  }
}

void updateLED()
{
  static int lastStatus = 0;
  static int timeStep = 0;
  static int scale = 10;
  if(ledStatus != lastStatus)
  {
    timeStep = 0;
    lastStatus = ledStatus;
  }
  else
    timeStep += scale;
  if(timeStep >= 240)
    scale = -10;
  else if(timeStep<=10)
    scale = 10;
  if(shutdown)
  {
    //color(255,0,0);
  }
  else
  {
    switch(ledStatus)
    {
      case 1: //STANDBY MODE, breathing blue
        //Case 1
        digitalWrite(ledRed, LOW);
        digitalWrite(ledGreen, LOW);
        analogWrite(ledBlue, timeStep);
        break;
      case 2: //BALANCE MODE, breathing yellow
        //Case 2
        analogWrite(ledRed, timeStep);
        analogWrite(ledGreen, timeStep);
        digitalWrite(ledBlue, LOW);
        break;
      case 3://RUN MODE, breathing orange
        analogWrite(ledRed, timeStep);
        analogWrite(ledGreen, floor(timeStep/2));
        digitalWrite(ledBlue, LOW);
        break;
      case 4: //DEBUG/TEST MODE, breathing green
        digitalWrite(ledRed, LOW);
        digitalWrite(ledGreen, timeStep);
        analogWrite(ledBlue, LOW);
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
      case 8: //Emergency shutdown, blinking red
        //Case 8
        break;
        }
  }
}

void parsePacket(char incomingByte[64])
{
  int i;
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
        //tempSense();
        break;
        case VOLTSENSE:
        //tempSense();
        break;
        case CURRENTSENSE:
        i++; //Length byte
        i++; //Current byte
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
        Serial.print(0x09,HEX);
        break;
        case MANDSOFTSHUT:
        Serial.print(0x0A,HEX);
        break;
        case STARTLOGAMS:
        //startLogAMS();
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
          ledStatus = STANDBY;
        break;
        case MOTHERRUNNING:
          ledStatus = RUN;
        break;
        case MOTHERCHARGING:
        //
        break;
        case MOTHER2CHARGER:
        //
        break;
        case CHARGER2MOTHER:
        //
        break;
        case IGNORE:
        //
        break;
      }
    }
  }
}
