#ifndef SMB_H
#define SMB_H

#include <Arduino.h> //It is very important to remember this! note that if you are using Arduino 1.0 IDE, change "WProgram.h" to "Arduino.h"
#include <Wire.h>
class SMB
{
  public:
    SMB(int address);
    ~SMB();
    void balance(int cells);
    void currentlyBalancing(); //Returns which cells are currently being balanced
    int* readTemps(); //Returns pointer to array of read in cell temperatures
    int* readVoltages(); //Returns pointer to array of read cell voltages
    void pollSMB();
  private:
    char balancingMask = 163;
    int smbAddress = 0;
};

SMB::SMB(int address)
{
  smbAddress = address; //Stores the address for each instance of SMB
  bool cellsCurrentlyBalancing[6] = {0}; //Boolean array that stores which cells on the SMB are
                          //currently balancing
  int cellVoltages[6] = {0}; //Stores the last read cell voltages
  int cellTemps[12] = {0}; //Stores the last read cell temperatures
}

SMB::~SMB(){/*Nothing to destruct*/};

void SMB::balance(int cells)
{

}

void SMB::currentlyBalancing()
{

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
  Wire.beginTransmission(smbAddress);
  Wire.write(balancingMask);
  Serial.print(Wire.endTransmission(), DEC);
  delay(500);
  Serial.print("<-WireTransmission:OtherStuff->");
  Wire.requestFrom(smbAddress, 24);
  Serial.print(":" + String(Wire.available()) + ":");
  while (Wire.available()) { // slave may send less than requested
  //  int c = Wire.read(); // receive a byte as character
  //  Serial.println(c);         // print the character
  //Serial.println(Wire.read(),DEC);
    if((i%2)==0)
    {
      j = 0;//reset variable that's being concatinated
      i = 0;
      int c = Wire.read();
      j = (c&0xFF) <<8;//Read High byte in and shift it to the left then store it to variable
      Serial.print(":HighByte:");
      Serial.print(c);
      Serial.print(":");
      Serial.print(j,BIN);
      Serial.print(":");
    }
    else
    {
      int c = Wire.read();
      j = ((j&0xF00) | (c & 0xFF)); //Read in low byte, mask it and append it to previous High byte
      Serial.print(":LowByte:");
      Serial.print(c&0xFF,BIN);
      Serial.print(":");
      Serial.println(j,BIN);
      Serial.print(":");
      Serial.println(j,DEC);
    }
    i++;
  }
  Serial.print("------------------------------\n");
}

#endif
