#include <Wire.h>
#include "smb.h"
//#include <SoftwareSerial.h>

////Serial Protocol Defines
#define TERMINATOR 0x00
#define TESTING 0x01
#define TEMPSENSE 0x02
#define VOLTSENSE 0x03
#define CURRENTSENSE 0x04
#define READYTODRIVE 5
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

//Pin Definitions
#define ohShit 9
#define currentSensor A0
#define startUp 4
#define watchDog 7
#define motherRx 11
#define motherTx 12
#define placeHolder 0
#define balancingInducator 1
#define fatalErrorInducator 13

int state = 0;
//State 0 = Standby
//State 1 = Running Mode
//State 2 = Charging Mode
bool balancingFlag = 0;
const float minModuleVoltage = 2.5;
const float maxModuleVoltage = 4.2;
float softMinModuleVoltage = 3.0;
float softMaxModuleVoltage = 4.1;
float softManditoryMinModuleVoltage = 2.6;
const float minPackVoltage = 55.0;
const float maxPackVoltage = 92.4;
float softMinPackVoltage = 66.0;
float softMaxPackVoltage = 87.0;
float lowestModuleVoltage = 100;
float balanceThreshold = 0.01;

//Temperatures are in celcius
const float minCellTemp = 0;
const float maxCellTemp = 75.0;
float softMinCellTemp = 0;
float softMaxCellTemp = 69.0;
float softManditoryMaxCellTemp = 74.0;
float currentPackVoltage = 0;
float lowestCellVoltage = 1000;
float highestCellVoltage = 0;

bool shutdown = 0;

unsigned long time;

const SMB smb1 = SMB(0x01,5);
const SMB smb2 = SMB(0x02,4);
const SMB smb4 = SMB(0x04,5);
const SMB smb5 = SMB(0x05,5);

void standby(); //Contains the standby loop functions and order
void running(); //Contains the running loop functions and order
void charging(); //Contains the charging loop functions and order
int estop(int stopSeverity); //Function that pulls the ohShit line
                              //low when someting bad is happening
                              //stopSeverity parameter sets how sevear the emergency is,
                              // 0 = pull ohShit super sevear
                              // 2 = Ask PLC to shutdown TSV
                              // 3 = Warn driver
                              //returns 1 when successfuly executed

void balance(); //
void toggleWatchDog(); //Toggles the watchDog pin
void preChargeShutdown();
void preChargeStart();
void checkCellVoltages();
void checkCellTemps();
int discountTripleCheck(SMB smb, int module);
void stopBalance();
void dataRecieve();
void parsePacket(char incomingByte[64]);

void setup()
{
  // put your setup code here, to run once:
  pinMode(ohShit, OUTPUT);
  pinMode(startUp, OUTPUT);
  pinMode(watchDog, OUTPUT);
  pinMode(motherRx, INPUT);
  pinMode(motherTx, OUTPUT);
  pinMode(fatalErrorInducator, OUTPUT);
  pinMode(balancingInducator, OUTPUT);
  pinMode(placeHolder, OUTPUT);

  digitalWrite(motherTx, LOW);
  digitalWrite(startUp, LOW);
  digitalWrite(ohShit, HIGH);
  digitalWrite(watchDog, HIGH);
  digitalWrite(fatalErrorInducator, LOW);
  digitalWrite(balancingInducator, LOW);
  digitalWrite(placeHolder, LOW);

  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(9600);
  Wire.begin();

}

void loop()
{
  if(state == 0)
    standby();
  else if(state == 1)
    running();
  else if(state == 2)
    charging();
  else
    estop(0);
  delay(100);
}

void standby()
{
  //Serial.println("_____I'M IN STANDBY!!!________");
  toggleWatchDog();
  if(digitalRead(motherRx) && !shutdown)
  {
    if((currentPackVoltage>softMinPackVoltage||currentPackVoltage<softMaxPackVoltage)&&(lowestCellVoltage>minModuleVoltage)&&(highestCellVoltage<maxModuleVoltage))
    {
      state = 1;
      digitalWrite(motherTx, HIGH);
    }
  }
  else
  {
    state = 0;
    digitalWrite(motherTx, LOW);
  }
  preChargeShutdown();
  checkCellVoltages();
  checkCellTemps();
  balance();
}

void running()
{
  //Serial.println("_____I'M RUNNING!!!________");
  toggleWatchDog();
  if(!digitalRead(motherRx))
  {
    state = 0;
    digitalWrite(motherTx, LOW);
  }
  stopBalance();
  preChargeStart();
  checkCellVoltages();
  checkCellTemps();
}

void charging()
{

}

int estop(int stopSeverity)
{
  if(stopSeverity == 3)
  {
    //warn driver
    //Serial.println("______WARN THE DRIVER ABOUT THING_______ -ESTOP<3");
    return 1;
  }
  else if(stopSeverity == 2)
  {
    //Ask plc to shutdown
    state = 0;
    shutdown = 1;
    //Serial.println("_____TELL THE PLC TO SHUTDOWN________ -ESTOP<3");
    return 1;
  }
  else
  {
    state = 0;
    shutdown = 1;
    //Pull ohShit line and shutdown car
    //Serial.println("_________OH SHIT__________ -ESTOP<3");
    digitalWrite(fatalErrorInducator, HIGH);
    digitalWrite(ohShit, LOW);
  }
}

void toggleWatchDog()
{
  digitalWrite(watchDog, LOW);
  digitalWrite(watchDog, HIGH);
}

void preChargeStart()
{
  digitalWrite(startUp, HIGH);
}

void preChargeShutdown()
{
  digitalWrite(startUp, LOW);
}

void checkCellVoltages()
{
  int i;
  SMB selectedSMB;
  int numberOfModules = 0;
  float tempPackVoltage = 0;
  for(i=0; i<4; i++)
  {
    if(i == 0)
      selectedSMB = smb1;
    else if(i == 1)
      selectedSMB = smb2;
      //selectedSMB = smb1;
    else if(i == 2)
      selectedSMB = smb4;
      //selectedSMB = smb1;
    else if(i == 3)
      selectedSMB = smb5;
      //selectedSMB = smb1;
    int j;
    selectedSMB.pollSMB();
    selectedSMB.pollSMB();
    numberOfModules = selectedSMB.numModules();
    float* stringVoltages = selectedSMB.readVoltages();
    for(j = 0; j<numberOfModules; j++)
    {
      ////Serial.print(stringVoltages[j]);
      tempPackVoltage += stringVoltages[j];
      if(stringVoltages[j] > highestCellVoltage)
        highestCellVoltage = stringVoltages[j];
      if(stringVoltages[j] < lowestCellVoltage)
        lowestCellVoltage = stringVoltages[j];
      if (stringVoltages[j]<=softMinModuleVoltage || stringVoltages[j]>=softMaxModuleVoltage)
      {
        ////Serial.println("CELL IS BELOW OR ABOVE SOFT VOLTAGE LIMIT, BE CONCERNED!!!");
        estop(3);
      }
      if (stringVoltages[j]<=softManditoryMinModuleVoltage)
      {
        ////Serial.println("CELL IS AT/BELOW MANDITORY SOFT VOLTAGE LIMIT, PLEASE SHUT DOWN SIGMA BEFORE I SHUTDOWN!!!");
        int result = discountTripleCheck(selectedSMB, j);

        if(result != 1)
          estop(2);
      }
      if (stringVoltages[j]<=minModuleVoltage || stringVoltages[j]>=maxModuleVoltage)
      {
        ////Serial.println("CELL IS BELOW OR ABOVE HARD VOLTAGE LIMIT, HALP!!!");
        int result = discountTripleCheck(selectedSMB, j);
        if(result != 1)
          estop(0);
      }
    }
    if(!digitalRead(motherRx))
    {
      state = 0;
      digitalWrite(motherTx, LOW);
    }
    delay(100);
  }
  lowestModuleVoltage = lowestCellVoltage;
  //Serial.print("Lowest Cell Voltage: ");
  //Serial.println(lowestCellVoltage);
  //Serial.print("Highest Cell Voltage: ");
  //Serial.println(highestCellVoltage);
  //Serial.print("Total Pack Voltage: ");
  //Serial.println(tempPackVoltage);
  currentPackVoltage = tempPackVoltage;
}

int discountTripleCheck(SMB smb, int module)
{
  int i;
  int j;
  float cellAverage = 0;
  for(i=0; i<3; i++)
  {
    smb.pollSMB();
    smb.pollSMB();
    float* stringVoltages = smb.readVoltages();
    cellAverage = (cellAverage + stringVoltages[module])/(i+1);

  }
  if (cellAverage<=softMinModuleVoltage || cellAverage>=softMaxModuleVoltage)
  {
    ////Serial.println("CELL IS BELOW OR ABOVE SOFT VOLTAGE LIMIT, BE CONCERNED!!!");
    return estop(3);
  }
  if (cellAverage<=softManditoryMinModuleVoltage)
  {
    ////Serial.println("CELL IS AT/BELOW MANDITORY SOFT VOLTAGE LIMIT, PLEASE SHUT DOWN SIGMA BEFORE I SHUTDOWN!!!");
    return estop(2);
  }
  if (cellAverage<=minModuleVoltage || cellAverage>=maxModuleVoltage)
  {
    ////Serial.println("CELL IS BELOW OR ABOVE HARD VOLTAGE LIMIT HALP!!!");
    digitalWrite(fatalErrorInducator, HIGH);
    return estop(0);
  }

  return -1; //indicates function error
}

void checkCellTemps()
{

  int i;
  SMB selectedSMB;
  int lowestCellTemp = 1000;
  int highestCellTemp = 0;
  int numberOfModules = 0;
  for(i=0; i<4; i++)
  {
    if(i == 0)
      selectedSMB = smb1;
    else if(i == 1)
      selectedSMB = smb2;
      //selectedSMB = smb1;
    else if(i == 2)
      selectedSMB = smb4;
      //selectedSMB = smb1;
    else if(i == 3)
      selectedSMB = smb5;
      //selectedSMB = smb1;
    selectedSMB.pollSMB();
    selectedSMB.pollSMB();
    int j;
    int numberOfSensors = selectedSMB.numSensors();
    float* stringTemps = selectedSMB.readTemps();
    for(j = 0; j<numberOfSensors; j++)
    {
      if(stringTemps[j] > highestCellTemp)
        highestCellTemp = stringTemps[j];
      if(stringTemps[j] < lowestCellTemp)
        lowestCellTemp = stringTemps[j];
      if(stringTemps[j]<=softMinCellTemp || stringTemps[j]>=softMaxCellTemp)
      {
        //Serial.println("CELL IS BELOW OR ABOVE SOFT TEMP LIMIT, BE CONCERNED!!!");
      }
      if(stringTemps[j]>=softManditoryMaxCellTemp)
      {
        //Serial.println("CELL IS ABOVE MANDITORY SOFT TEMP LIMIT, PLEASE STOP SIGMA BEFORE I SHUTDOWN!!!");
      }
      if(stringTemps[j]<=minCellTemp || stringTemps[j]>=maxCellTemp)
      {
        //Serial.println("CELL IS BELOW OR ABOVE HARD TEMP LIMIT, HALP!!!");
      }
    }
    if(!digitalRead(motherRx))
    {
      state = 0;
      digitalWrite(motherTx, LOW);
    }
  }
  //Serial.print("Lowest Cell Temperature: ");
  //Serial.println(lowestCellTemp);
  //Serial.print("Highest Cell Temperature: ");
  //Serial.println(highestCellTemp);
  if(lowestCellTemp<softMinCellTemp || highestCellTemp > softMaxCellTemp)
  {
    estop(0);
  }
}

void balance()
{
  int i;
  int j;
  SMB selectedSMB;
  boolean needBalancing = 0;
  for(i=0; i<4; i++)
  {
    if(i == 0)
      selectedSMB = smb1;
    else if(i == 1)
      selectedSMB = smb2;
      //selectedSMB = smb1;
    else if(i == 2)
      selectedSMB = smb4;
      //selectedSMB = smb1;
    else if(i == 3)
      selectedSMB = smb5;
      //selectedSMB = smb1;
    int numberOfModules = selectedSMB.numModules();
    float* stringVoltages = selectedSMB.readVoltages();
    char cellMask = B0;
    for(j=0;j<numberOfModules;j++)
    {
      if((stringVoltages[j] - lowestModuleVoltage) > balanceThreshold)
        cellMask = cellMask | (1<<j);
    }
    selectedSMB.balance(cellMask);
    if(cellMask>0)
      needBalancing = 1;
  }
  digitalWrite(balancingInducator, needBalancing);
}

void stopBalance()
{
  int i;
  SMB selectedSMB;
  for(i=0; i<4; i++)
  {
    if(i == 0)
      selectedSMB = smb1;
    else if(i == 1)
      selectedSMB = smb2;
      //selectedSMB = smb1;
    else if(i == 2)
      selectedSMB = smb4;
      //selectedSMB = smb1;
    else if(i == 3)
      selectedSMB = smb5;
      //selectedSMB = smb1;
    selectedSMB.balance(0);
    selectedSMB.stopBalance();
  }
}
