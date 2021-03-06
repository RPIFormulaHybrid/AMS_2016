#include "SMB.h"
#include <SoftwareSerial.h>

#define ohShit 9
#define currentSensor A0
#define startUp 4
#define watchDog 7

int state = 0;
//State 0 = Standby
//State 1 = Running Mode
//State 2 = Charging Mode
bool balancingFlag = 0;
float minModuleVoltage = 3.2;
float maxModuleVoltage = 4.2;
float softMinModuleVoltage = 0;
float softMaxModuleVoltage = 0;

float minCellTemp = 0;
float maxCellTemp = 100;
float softMinCellTemp = 0;
float softMaxCellTemp = 0;

SMB smb1 = SMB(0x01, 6);
SMB smb2 = SMB(0x02, 5);
SMB smb3 = SMB(0x03, 5);
SMB smb4 = SMB(0x04, 6);
SMB smb5 = SMB(0x05, 2);

//SoftwareSerial Comm(11, 12);

//int smbs[8] = {0x01, 0x02, 0x03, 0x04, 0x05}; //Address Array for SMBS inside TSAC
//SMB smbs[8] = (smb1, smb2, smb3, smb4, smb5); //Address Array for SMBS inside TSAC
SMB smbs[1] = (smb1); //Address Array for SMBS inside TSAC


void standby(); //Contains the standby loop functions and order
void running(); //Contains the running loop functions and order
void charging(); //Contains the charging loop functions and order
void estop(int stopSeverity); //Function that pulls the ohShit line
                              //low when someting bad is happening
                              //stopSeverity parameter sets how sevear the emergency is,
                              // 0 = pull ohShit super sevear
                              // 2 = Ask PLC to shutdown TSV
                              // 3 = Warn driver

void balance(); //
void toggleWatchDog(); //Toggles the watchDog pin
void preChargeShutdown();
void preChargeStart();
void checkCellVoltages();
void checkCellTemps();
int discountTripleCheck(SMB smb, int module);

void setup() {
  // put your setup code here, to run once:
  pinMode(ohShit, OUTPUT);
  pinMode(startUp, OUTPUT);
  pinMode(watchDog, OUTPUT);

  digitalWrite(startUp, LOW);
  digitalWrite(ohShit, HIGH);
  digitalWrite(watchDog, LOW);

  Serial.begin(9600);
  //Comm.begin(9600);
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(state == 0)
    standby();
  else if(state == 1)
    running();
  else if(state == 2)
    charging();
  else
    estop(0);
  delay(500);
}

void standby()
{
  toggleWatchDog();
  preChargeShutdown();
  checkCellVoltages();
  checkCellTemps();
}

void running()
{

}

void charging()
{

}



void estop(int stopSeverity)
{
  if(stopSeverity == 3)
  {
    //warn driver
  }
  else if(stopSeverity == 2)
  {
    //Ask plc to shutdown
  }
  else
    //Pull ohShit line and shutdown car
    digitalWrite(ohShit, LOW);
}

void toggleWatchDog()
{
  digitalWrite(watchDog, HIGH);
  delay(20);
  digitalWrite(watchDog, LOW);
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
  int i;
  int numberOfModules = 0;
  for(i=0; i<sizeof(smbs); i++)
  {
    int j;
    numberOfModules = smbs[i].numModules();
    //Wire.begin();
    smbs[i].pollSMB();
    //Wire.end();
    int* stringVoltages = smbs[i].readVoltages();
    for(j = 0; j<numberOfModules; j++)
    {
      if (stringVoltages[j]<minModuleVoltage || stringVoltages[j]>maxModuleVoltage)
      {
        discountTripleCheck(smbs[i], j);
      }
    }
  }
}

int discountTripleCheck(SMB smb, int module)
{
  int i;
  for(i=0; i<3; i++)
  {
    smb.checkModule(module);
  }
  return -1; //indicates function error
}

void checkCellTemps()
{
  int i;
  int numberOfModules = 0;
  for(i=0; i<sizeof(smbs); i++)
  {
    int j;
    int numberOfSensors = smbs[i].numSensors();
    int* stringTemps = smbs[i].readTemps();
    for(j = 0; j<numberOfSensors; j++)
    {
      if (stringTemps[j]<minModuleVoltage || stringTemps[j]>maxModuleVoltage)
      {
        discountTripleCheck(smbs[i], j);
      }
    }
  }
}
