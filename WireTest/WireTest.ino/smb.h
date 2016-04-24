#ifndef SMB_H
#define SMB_H

#include <Arduino.h> //It is very important to remember this! note that if you are using Arduino 1.0 IDE, change "WProgram.h" to "Arduino.h"
#include <Wire.h>
class SMB
{
  public:
    SMB(int address, int numberOfModulesConnected);
    ~SMB();
    void balance(int cells);
    int currentlyBalancing(); //Returns which cells are currently being balanced
    int* readTemps(); //Returns pointer to array of read in cell temperatures
    int* readVoltages(); //Returns pointer to array of read cell voltages
    float checkModule(int module); //Returns the cell voltage for a specific module
    int numModules(); //Returns the number of modules the smb is handling
    int numSensors(); //Returns the number of sensors connected to the smb
    void pollSMB();
  private:
    char balancingMask = 163;
    int smbAddress = 0;
    int numberOfSensors = 12;
    int numberOfModules = 0;
    bool cellsCurrentlyBalancing[6] = {0}; //Boolean array that stores which cells on the SMB are currently balancing
    bool cellsInNeedOfBalancing[6] = {0}; //Stores which cells still need balancing
    float cellVoltages[6] = {0}; //Stores the last read cell voltages
    float cellTemps[12] = {0}; //Stores the last read cell temperatures
};

SMB::SMB(int address, int numberOfModulesConnected)
{
  smbAddress = address; //Stores the address for each instance of SMB
  numberOfModules = numberOfModulesConnected;
}

SMB::~SMB(){/*Nothing to destruct*/};

void SMB::balance(int cells)
{
  cellsInNeedOfBalancing[cells] = 1;
}

int SMB::currentlyBalancing()
{
  int i;
  int mask = 0;
  for(i=0;i<numberOfModules;i++)
  {
    if(cellsCurrentlyBalancing[i] == 1)
    mask = mask | (1<<i);
  }
  return mask;
}

int* SMB::readTemps()
{
  return 0;
}

int* SMB::readVoltages()
{
  return 0;
}

void SMB::pollSMB()
{
  int i = 0;
  int j = 0;
  int k = 0;
  int readData[24];

  Wire.beginTransmission(smbAddress);
  Wire.write(balancingMask);
  int wireReturn = Wire.endTransmission();
  Serial.print(wireReturn, DEC);
  delay(500);
  Serial.print("<-WireTransmission:OtherStuff->");
  Wire.requestFrom(smbAddress, 24);
  //Serial.println(":" + String(Wire.available()) + ":");
  while (Wire.available()) 
  { 
    readData[i] = Wire.read()&0xFF;
    i++;
  }
  if(readData[23] != 0)
  {
    for(int z = 0; z<12; z++)
    {
      Serial.print(",");
      Serial.print((((readData[(z*2)]<<8)&0xFF00)|(readData[(z*2)+1]&0xFF))&0xFFF, DEC);
      
    }
    Serial.println("------------------------------");
  }
  else if(readData[23] == 0)
  {
   Serial.println("");
   Serial.print("Cell 1 V: ");
   Serial.println(((readData[0] & 0xFF) | (readData[1] & 0x0F) << 8)* 0.0015,3);
   Serial.print("Cell 2 V: ");
   Serial.println(((readData[1] & 0xF0) >> 8 | (readData[2] & 0xFF) << 4 )*.0015,3);
   Serial.print("Cell 3 V: ");
   Serial.println(((readData[3] & 0xFF) | (readData[4] & 0x0F) << 8)*.0015,3);
   Serial.print("Cell 4 V: ");
   Serial.println(((readData[4] & 0xF0) >> 8 | (readData[5] & 0xFF) << 4 )*.0015,3);
   Serial.print("Cell 5 V: ");
   Serial.println(((readData[6] & 0xFF) | (readData[7] & 0x0F) << 8)*.0015,3);
   Serial.print("Cell 6 V: ");
   Serial.println((((readData[7] & 0xF0) >> 8 | (readData[8] & 0xFF) << 4))*.0015,3);
   Serial.println("------------------------------");
  }
}


int SMB::numModules()
{
  return numberOfModules;
}

float SMB::checkModule(int module)
{
  if(module > numberOfModules)
    return -1; //-1 indicates there was an error with the function
  pollSMB();
  return cellVoltages[module];

}

int SMB::numSensors()
{
  return numberOfSensors;
}

#endif

