#ifndef NAT_H
#define NAT_H

#include <Arduino.h> //It is very important to remember this! note that if you are using Arduino 1.0 IDE, change "WProgram.h" to "Arduino.h"

class NAT
{
  public:
    NAT();
    ~NAT();
    void balance(int cells);
    void currentlyBalancing(); //Returns which cells are currently being balanced
    void readTemps(); //Returns pointer to array of read in cell temperatures
    void readVoltages(); //Returns pointer to array of read cell voltages
    void pollSMB();
};

#endif
