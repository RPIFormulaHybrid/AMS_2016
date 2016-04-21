#ifndef SMB_H
#define SMB_H

#include <Arduino.h> //It is very important to remember this! note that if you are using Arduino 1.0 IDE, change "WProgram.h" to "Arduino.h"

class SMB
{
  public:
    SMB();
    ~SMB();
    void balance(int cells);
    void currentlyBalancing(); //Returns which cells are currently being balanced
    int* readTemps(); //Returns pointer to array of read in cell temperatures
    int* readVoltages(); //Returns pointer to array of read cell voltages
};

#endif
