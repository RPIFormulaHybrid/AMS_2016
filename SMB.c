#include "SMB.h"

SMB::SMB(int address)
{
  int smbAddress = address; //Stores the address for each instance of SMB
  bool cellsCurrentlyBalancing[6] = {0}; //Boolean array that stores which cells on the SMB are
                          //currently balancing
  int cellVoltages[6] = {0}; //Stores the last read cell voltages
  int cellTemps[12] = {0}; //Stores the last read cell temperatures
}

SMB::~SMB(){/*Nothing to destruct*/};

SMB::balance(int cells)
{

}

SMB::currentlyBalancing()
{

}

SMB::readTemps()
{

}

SMB::readVoltages()
{

}
