#ifndef SMB_H
#define SMB_H

#include <Arduino.h> //It is very important to remember this! note that if you are using Arduino 1.0 IDE, change "WProgram.h" to "Arduino.h"
#include <Wire.h>
class SMB
{
  public:
    SMB();
    SMB(int address, int numberOfModulesConnected);
    ~SMB();
    void balance(char cells);
    int currentlyBalancing(); //Returns which cells are currently being balanced
    float* readTemps(); //Returns pointer to array of read in cell temperatures
    float* readVoltages(); //Returns pointer to array of read cell voltages
    float checkModule(int module); //Returns the cell voltage for a specific module
    int numModules(); //Returns the number of modules the smb is handling
    int numSensors(); //Returns the number of sensors connected to the smb
    void pollSMB();
    void stopBalance();
  private:
    unsigned long currentTime = 0;
    unsigned long previousTime = 0;
    char balancingMask = B101010; //BalancingMask, 6 bits set which cell is balancing
    int smbAddress = 0;
    int numberOfSensors = 12;
    int numberOfModules = 0;
    char cellsCurrentlyBalancing = B0; //Boolean array that stores which cells on the SMB are currently balancing
    char cellsInNeedOfBalancing = B0; //Stores which cells still need balancing
    float cellVoltages[6] = {0}; //Stores the last read cell voltages
    float cellTemps[12] = {0}; //Stores the last read cell temperatures
};

SMB::SMB()
{

}

SMB::SMB(int address, int numberOfModulesConnected)
{
  smbAddress = address; //Stores the address for each instance of SMB
  numberOfModules = numberOfModulesConnected;
}

SMB::~SMB(){/*Nothing to destruct*/};

void SMB::balance(char cells)
{
  cellsInNeedOfBalancing = cells;
}

int SMB::currentlyBalancing()
{
  int i;
  int mask = 0;
/*  for(i=0;i<numberOfModules;i++)
  {
    if(cellsCurrentlyBalancing[i] == 1)
    mask = mask | (1<<i);
  }*/
  return mask;
}

float* SMB::readTemps()
{
  return cellTemps;
}

float* SMB::readVoltages()
{
  return cellVoltages;
}

void SMB::pollSMB()
{
  int i = 0;
  int j = 0;
  int k = 0;
  char cellGroup1 = B010101;
  char cellGroup2 = B101010;
  int readData[24];
  currentTime = millis();
  //Determins what balancing mask to send to smb based on cells that need balancing, changes every 30seconds until balanced
  if(currentTime - previousTime > 30000)
  {
    previousTime = currentTime;
    if(cellsInNeedOfBalancing != 0)
    {
      if(cellsInNeedOfBalancing & cellGroup1 != 0)
      {
        if((cellsInNeedOfBalancing & cellGroup2 == 0) || (cellsInNeedOfBalancing & cellGroup1 != cellsCurrentlyBalancing))
        {
          cellsCurrentlyBalancing = cellsInNeedOfBalancing & cellGroup1;
          balancingMask = cellsInNeedOfBalancing & cellGroup1;
        }
      }
      if(cellsInNeedOfBalancing & cellGroup2 != 0)
      {
        if((cellsInNeedOfBalancing & cellGroup1 == 0) || (cellsInNeedOfBalancing & cellGroup2 != cellsCurrentlyBalancing))
        {
          cellsCurrentlyBalancing = cellsInNeedOfBalancing & cellGroup2;
          balancingMask = cellsInNeedOfBalancing & cellGroup2;
        }
      }
    }
  }

  Serial.print("I AM SMB ");
  Serial.print(smbAddress);
  Serial.println(":");
  Wire.beginTransmission(smbAddress);
  Wire.write(balancingMask);
  int wireReturn = Wire.endTransmission();
  Serial.print(wireReturn, DEC);
  delay(10);
  Serial.print("<-WireTransmission:OtherStuff->");
  Wire.requestFrom(smbAddress, 24);
  Serial.println(":" + String(Wire.available()) + ":");
  while (Wire.available())
  {
    readData[i] = Wire.read()&0xFF;
    i++;
  }
  if(readData[23] != 0)
  {
    for(int z = 0; z<12; z++)
    {
    /*  Serial.print(",");
      Serial.print((((readData[(z*2)]<<8)&0xFF00)|(readData[(z*2)+1]&0xFF))&0xFFF, DEC);*/
      cellTemps[z] = (((readData[(z*2)]<<8)&0xFF00)|(readData[(z*2)+1]&0xFF))&0xFFF;

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
   cellVoltages[0] = ((readData[0] & 0xFF) | (readData[1] & 0x0F) << 8)* 0.0015;
   cellVoltages[1] = ((readData[1] & 0xF0) >> 8 | (readData[2] & 0xFF) << 4 )*.0015;
   cellVoltages[2] = ((readData[3] & 0xFF) | (readData[4] & 0x0F) << 8)*.0015;
   cellVoltages[3] = ((readData[4] & 0xF0) >> 8 | (readData[5] & 0xFF) << 4 )*.0015;
   cellVoltages[4] = ((readData[6] & 0xFF) | (readData[7] & 0x0F) << 8)*.0015;
   cellVoltages[5] = (((readData[7] & 0xF0) >> 8 | (readData[8] & 0xFF) << 4))*.0015;
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

void SMB::stopBalance()
{
  Wire.beginTransmission(smbAddress);
  Wire.write(0);
  int wireReturn = Wire.endTransmission();
}

#endif
