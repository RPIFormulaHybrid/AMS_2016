#include <Wire.h>
#include "smb.h"
#include <SoftwareSerial.h>

//Serial Protocol Defines
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

int state = 0;
//State 0 = Standby
//State 1 = Running Mode
//State 2 = Charging Mode
bool balancingFlag = 0;
const int minModuleVoltage = 1667; //2.5 * 666.66 = 1666.65 = 1667 (round up)
const int maxModuleVoltage = 2800; //4.2 * 666.66 = 2799.972 = 2800 (round up)
int softMinModuleVoltage = 1867;   //2.8 * 666.66 = 1866.648 = 1867 (round up)
int softMaxModuleVoltage = 2733;  //4.1 * 666.66 = 2733.306 = 2733 (round down)
int softManditoryMinModuleVoltage = 1733; //2.6 * 666.66 = 1733.316 = 1733 (round down)
const int minPackVoltage = 36666;  //55.0 * 666.66 = 36666.3 = 36666 (round down)
const int maxPackVoltage = 61599; //92.4 * 666.66 = 61599.384 = 61599 (round down)
int softMinPackVoltage = 44000;  //66.0 * 666.66 = 43999.56 = 44000 (round up)
int softMaxPackVoltage = 57999;  //87.0 * 666.66 = 57999.42 = 57999 (round down)
int lowestModuleVoltage = 66666;  //100 * 666.66 = 66666
int balanceThreshold = 7;  //0.01 * 666.66 = 6.6666 = 7 (round up)

//Temperatures are in celcius
const float minCellTemp = 0;
const float maxCellTemp = 75.0;
float softMinCellTemp = 0;
float softMaxCellTemp = 69.0;
float softManditoryMaxCellTemp = 74.0;
int currentPackVoltage = 0;
int lowestCellVoltage = 10000;
int highestCellVoltage = 0;

bool shutdown = 0;

unsigned long time;

const SMB smb1 = SMB(0x01,6);
const SMB smb2 = SMB(0x02,5);
const SMB smb4 = SMB(0x04,6);
const SMB smb5 = SMB(0x05,6);

int i =0;
SMB selectedSMB;

char SMB::cellsInNeedOfBalancing = B0;

unsigned long SMB::currentTime = 0;
unsigned long SMB::previousTime = 0;
char SMB::balancingMask = B0; //BalancingMask, 6 bits set which cell is balancing
char SMB::cellsCurrentlyBalancing = B0; //Boolean array that stores which cells on the SMB are currently balancing
int SMB::cellVoltages[6] = {0}; //Stores the last read cell voltages
float SMB::cellTemps[12] = {0}; //Stores the last read cell temperatures
//static int* stringVoltages;

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

  digitalWrite(motherTx, LOW);
  digitalWrite(startUp, LOW);
  digitalWrite(ohShit, HIGH);
  digitalWrite(watchDog, HIGH);

  // put your setup code here, to run once:
  delay(1000);
  Serial.begin(9600);
  Wire.begin();
}

void loop()
{
    if(i == 0)
      selectedSMB = smb1;
    else if(i == 1)
      //selectedSMB = smb2;
      selectedSMB = smb1;
    else if(i == 2)
      //selectedSMB = smb4;
      selectedSMB = smb1;
    else if(i == 3)
    {
      //selectedSMB = smb5;
      i=-1;
    }
      //selectedSMB = smb1;
    selectedSMB.pollSMB();
    selectedSMB.pollSMB();

  if(state == 0)
    standby();
  else if(state == 1)
    running();
  else if(state == 2)
    charging();
  else
    estop(0);
  i+=1;
  delay(200);
}

void standby()
{
  Serial.println("_____I'M IN STANDBY!!!________");
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
  Serial.println("_____I'M RUNNING!!!________");
  toggleWatchDog();
  if(!digitalRead(motherRx))
  {
    state = 0;
    digitalWrite(motherTx, LOW);
  }
  //stopBalance();
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
    Serial.println("______WARN THE DRIVER ABOUT THING_______ -ESTOP<3");
    return 1;
  }
  else if(stopSeverity == 2)
  {
    //Ask plc to shutdown
    state = 0;
    shutdown = 1;
    Serial.println("_____TELL THE PLC TO SHUTDOWN________ -ESTOP<3");
    return 1;
  }
  else
  {
    state = 0;
    shutdown = 1;
    //Pull ohShit line and shutdown car
    Serial.println("_________OH SHIT__________ -ESTOP<3");
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
  digitalWrite(startUp, LOW);
}

void preChargeShutdown()
{
  digitalWrite(startUp, HIGH);
}

void checkCellVoltages()
{
  int numberOfModules = 0;
  int tempPackVoltage = 0;
  int tempLowestCellVoltage = 10000;
  int tempHighestCellVoltage = 0;
    int j;
    numberOfModules = selectedSMB.numModules();
    int* stringVoltages = selectedSMB.readVoltages();
    stringVoltages = selectedSMB.readVoltages();
    Serial.println("READ OUT OF STRING VOLTAGES");
    //Serial.print("NumberOfModules: ");
    //Serial.println(numberOfModules);
    Serial.println("___________________________");
    Serial.println(stringVoltages[0]);
    Serial.println(stringVoltages[1]);
    Serial.println(stringVoltages[2]);
    Serial.println(stringVoltages[3]);
    Serial.println(stringVoltages[4]);
    Serial.println(stringVoltages[5]);
    Serial.println("___________________________");

    for(j = 0; j<numberOfModules; j++)
    {
      //Serial.print(stringVoltages[j]);
      tempPackVoltage += stringVoltages[j];
      if(stringVoltages[j] > tempHighestCellVoltage)
        tempHighestCellVoltage = stringVoltages[j];
      if(stringVoltages[j] < tempLowestCellVoltage)
        tempLowestCellVoltage = stringVoltages[j];
      if (stringVoltages[j]<=softMinModuleVoltage || stringVoltages[j]>=softMaxModuleVoltage)
      {
        //Serial.println("CELL IS BELOW OR ABOVE SOFT VOLTAGE LIMIT, BE CONCERNED!!!");
        estop(3);
      }
      if (stringVoltages[j]<=softManditoryMinModuleVoltage)
      {
        //Serial.println("CELL IS AT/BELOW MANDITORY SOFT VOLTAGE LIMIT, PLEASE SHUT DOWN SIGMA BEFORE I SHUTDOWN!!!");
        int result = discountTripleCheck(selectedSMB, j);

        if(result != 1)
          estop(2);
      }
      if (stringVoltages[j]<=minModuleVoltage || stringVoltages[j]>=maxModuleVoltage)
      {
        //Serial.println("CELL IS BELOW OR ABOVE HARD VOLTAGE LIMIT, HALP!!!");
        int result = discountTripleCheck(selectedSMB, j);
        if(result != 1)
          estop(0);
      }
    }
  if(tempHighestCellVoltage != highestCellVoltage)
    highestCellVoltage = tempHighestCellVoltage;
  if(tempLowestCellVoltage != lowestCellVoltage)
    lowestCellVoltage = tempLowestCellVoltage;

  lowestModuleVoltage = lowestCellVoltage;
  Serial.print("Lowest Cell Voltage: ");
  Serial.println(lowestCellVoltage);
  Serial.print("Highest Cell Voltage: ");
  Serial.println(highestCellVoltage);
  Serial.print("Total Pack Voltage: ");
  Serial.println(tempPackVoltage);
  currentPackVoltage = tempPackVoltage;
}

int discountTripleCheck(SMB smb, int module)
{
  Serial.println("Triple Checking");
  int i;
  int j;
  float cellAverage = 0;
  int* stringVoltages;
  for(i=0; i<3; i++)
  {
    smb.pollSMB();
    smb.pollSMB();
    stringVoltages = smb.readVoltages();
    cellAverage = cellAverage + stringVoltages[module];
    delay(50);
  }
  cellAverage = (cellAverage + stringVoltages[module])/3;
  if (cellAverage<=softMinModuleVoltage || cellAverage>=softMaxModuleVoltage)
  {
    //Serial.println("CELL IS BELOW OR ABOVE SOFT VOLTAGE LIMIT, BE CONCERNED!!!");
    return estop(3);
  }
  if (cellAverage<=softManditoryMinModuleVoltage)
  {
    //Serial.println("CELL IS AT/BELOW MANDITORY SOFT VOLTAGE LIMIT, PLEASE SHUT DOWN SIGMA BEFORE I SHUTDOWN!!!");
    return estop(2);
  }
  if (cellAverage<=minModuleVoltage || cellAverage>=maxModuleVoltage)
  {
    //Serial.println("CELL IS BELOW OR ABOVE HARD VOLTAGE LIMIT HALP!!!");
    return estop(0);
  }

  return -1; //indicates function error
}

void checkCellTemps()
{
  int lowestCellTemp = 1000;
  int highestCellTemp = 0;
  int numberOfModules = 0;

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
        Serial.println("CELL IS BELOW OR ABOVE SOFT TEMP LIMIT, BE CONCERNED!!!");
      }
      if(stringTemps[j]>=softManditoryMaxCellTemp)
      {
        Serial.println("CELL IS ABOVE MANDITORY SOFT TEMP LIMIT, PLEASE STOP SIGMA BEFORE I SHUTDOWN!!!");
      }
      if(stringTemps[j]<=minCellTemp || stringTemps[j]>=maxCellTemp)
      {
        Serial.println("CELL IS BELOW OR ABOVE HARD TEMP LIMIT, HALP!!!");
      }
    }
    delay(100);

  Serial.print("Lowest Cell Temperature: ");
  Serial.println(lowestCellTemp);
  Serial.print("Highest Cell Temperature: ");
  Serial.println(highestCellTemp);
  if(lowestCellTemp<softMinCellTemp || highestCellTemp > softMaxCellTemp)
  {
    estop(0);
  }
}

void balance()
{
  Serial.println("Balance Function exe");
  Serial.println("--------------------");
  int j;
    int numberOfModules = selectedSMB.numModules();
    //selectedSMB.pollSMB();
    //selectedSMB.pollSMB();
    int* stringVoltages = selectedSMB.readVoltages();
    stringVoltages = selectedSMB.readVoltages();
    //Serial.println("String Voltages test stuff");
    //Serial.println(stringVoltages[0]);
    char cellMask = B0;
    for(j=0;j<numberOfModules;j++)
    {
      Serial.println(lowestModuleVoltage);
      Serial.println(stringVoltages[j]);
      Serial.println(stringVoltages[j] - lowestModuleVoltage);
      Serial.println("--------------------");

      if((stringVoltages[j] - lowestModuleVoltage) > balanceThreshold)
        cellMask = cellMask | (1<<j);
    }
    selectedSMB.balance(cellMask);
    selectedSMB.pollSMB();
    selectedSMB.pollSMB();
    Serial.print("Cell Mask for balancing:");
    Serial.println(cellMask,BIN);

}

void stopBalance()
{
  Serial.print("stopBalance Function exe");
  int i;
  SMB selectedSMB;
  for(i=0; i<4; i++)
  {
    if(i == 0)
      selectedSMB = smb1;
    else if(i == 1)
      //selectedSMB = smb2;
      selectedSMB = smb1;
    else if(i == 2)
      //selectedSMB = smb4;
      selectedSMB = smb1;
    else if(i == 3)
      //selectedSMB = smb5;
      selectedSMB = smb1;
    selectedSMB.balance(char(0));
    selectedSMB.stopBalance();
  }
}
