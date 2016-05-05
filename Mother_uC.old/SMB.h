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
  char readData[12] = {0};

  Wire.beginTransmission(smbAddress);
  Wire.write(balancingMask);
  int wireReturn = Wire.endTransmission();
  Serial.print(wireReturn, DEC);
  delay(500);
  Serial.print("<-WireTransmission:OtherStuff->");
  Wire.requestFrom(smbAddress, 24);
  //Serial.println(":" + String(Wire.available()) + ":");
  while (Wire.available()) { // slave may send less than requested
  //  int c = Wire.read(); // receive a byte as character
  //  Serial.println(c);         // print the character
  //Serial.println(Wire.read(),DEC);
    if((i%2)==0)
    {
      j = 0;//reset variable that's being concatinated
      i = 0;
      k += 1;
      int c = Wire.read();
      j = (c&0xFF) <<8;//Read High byte in and shift it to the left then store it to variable
      /*Serial.print(":HighByte:");
      Serial.print(c);
      Serial.print(":");
      Serial.print(j,BIN);
      Serial.print(":");*/
    }
    else
    {
      int c = Wire.read();
      j = ((j&0xFF00) | (c & 0xFF)); //Read in low byte, mask it and append it to previous High byte
      readData[k] = j&0xFFF;
      /*Serial.print(":LowByte:");
      Serial.print(c&0xFF,BIN);
      Serial.print(":");
      Serial.print(j&0xFFF,BIN);
      Serial.print(":");
      Serial.print(j&0xFFF,DEC);
      Serial.print(": Channel ");
      Serial.println((j&0xF000)>>12);*/
      k += 1;
    }
    i++;
  }
  for(int z = 0; z<sizeof(readData); z++)
    Serial.print("," + String(readData[z]));
  Serial.print("------------------------------\n");
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
