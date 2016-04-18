#include <SPI.h>
#include <Wire.h>

// SDO - PIN 12
// SDI - PIN 11
#define WRCFG 0x01 //LTC Write Configuration Registers
#define RDCFG 0x02 //LTC Read config
#define RDCV 0x04 // LC Read cells
#define STCVAD 0x10 //LTC Start all A/D's - poll status
#define RDFLG 0x06 //LTC Read Flags
#define RDTMP 0x08 //LTc Read Temperatures
#define STCDC 0x60 //LTC A/D converter and poll Status
#define STOWAD 0x20 //LTC Start Test - poll status
#define STTMPAD 0x30//LTC Temperature Reading - ALL
#define ltcAddress 0x80  //LTC address
#define ltcCS 10//LTC chip select
#define adcCS 9//ADC chip select
#define balCell1 2
#define balCell2 3
#define balCell3 4
#define balCell4 5
#define balCell5 6
#define balCell6 7
#define status 8
#define TEMP1 0x1000
#define TEMP2 0x1080
#define TEMP3 0x1200
#define TEMP4 0x1300
#define TEMP5 0x1400
#define TEMP6 0x1500
#define TEMP7 0x1600
#define TEMP8 0x1700
#define TEMP9 0x1800
#define TEMP10 0x1900
#define TEMP11 0x1A00
#define TEMP12 0x1B00

int returnedValue = 0;
byte temp[24]; //Temperature array
char byteTemp;
int balanceByte = 0; //Defines which cells to balance (bits 0-7 correspond to modules)

boolean balanceArray[8];

//Functions

void setup()
{
  pinMode(ltcCS,OUTPUT);
  pinMode(adcCS,OUTPUT);
  pinMode(balCell1, OUTPUT);
  pinMode(balCell2, OUTPUT);
  pinMode(balCell3, OUTPUT);
  pinMode(balCell4, OUTPUT);
  pinMode(balCell5, OUTPUT);
  pinMode(balCell6, OUTPUT);
  pinMode(status, OUTPUT);

  digitalWrite(ltcCS, HIGH);

  digitalWrite(balCell1, LOW); //Set all balancing pins low to open FETS
  digitalWrite(balCell2, LOW);
  digitalWrite(balCell3, LOW);
  digitalWrite(balCell4, LOW);
  digitalWrite(balCell5, LOW);
  digitalWrite(balCell6, LOW);
  digitalWrite(status, LOW);

  SPI.setBitOrder(MSBFIRST); //Most sig. bit first
  SPI.setDataMode(SPI_MODE3); //Sets SPI polarity and phase
  SPI.setClockDivider(SPI_CLOCK_DIV16); //Sends and requests 2 bytes at a time
  SPI.begin(); //Initiate SPI
  Serial.begin(9600); //Initiate serial FIXME:Remove after debug
  Wire.begin(1); //Sets address for I2C slave REVIEW: UPDATE FOR EACH SMB
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
//  writeReg(); //Configure registers
  //readReg(); //Print configurations FIXME: remove call after debug

  digitalWrite(adcCS,LOW); //Program ADC configuration
  SPI.transfer16(0x1840);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  delay(20);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  delay(20);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  delay(20);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  delay(20);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  delay(20);
  digitalWrite(adcCS,HIGH);

}

void loop()
{
  //readV();
  //readT();
  //(100);
}

void writeReg() //Writes configuration settings
{
  Serial.println("Writing config..."); //FIXME: remove after debugging
  digitalWrite(ltcCS, LOW); //Trigger LTC chip select
  SPI.transfer(ltcAddress);
  SPI.transfer(WRCFG); //Command to write to configuration registers
  SPI.transfer(0x01);//0 Set comparator duty cycle to default
  SPI.transfer(0x00);//1 Don't trigger ANY cell balancing
  SPI.transfer(0x00);//2 Enable interrupts for cells 1-4
  SPI.transfer(0x00);//3 Enable interrupts for cells 5-12
  SPI.transfer(0x71);//4 2.712V low voltage reference point REVIEW: can we use these?
  SPI.transfer(0xAB);//5 4.104V over voltage reference points REVIEW: can we use these?
  digitalWrite(ltcCS, HIGH); //End write
}

void readReg()
{
  Serial.println("Reading config..."); //FIXME remove after debug
  digitalWrite(ltcCS, LOW); //Trigger chip select
  SPI.transfer(ltcAddress);
  SPI.transfer(RDCFG); //Command to read from configuration registers
  for(int i = 0; i < 6; i++) //Cycle through each of the 6 config registers
  {
  byteTemp = SPI.transfer(RDCFG);
  Serial.println(byteTemp, HEX);
  }
  digitalWrite(ltcCS, HIGH); //Release LTC chip select
}

void readV()
{
  digitalWrite(ltcCS,LOW);
  SPI.transfer(STCVAD); //LTC Start all A/D's - poll status (all cells)
  delay(12); // wait at least 12ms as per data sheet, p.24 REVIEW: changed from 50 to 12ms
  digitalWrite(ltcCS,HIGH); //Exit polling
  char volt[18]; //Create volt array
  digitalWrite(ltcCS,LOW); //Trigger LTC chip select
  SPI.transfer(ltcAddress); //Iniitate transaction with LTC
  SPI.transfer(RDCV); //Command to read cell voltage reigster group
  for(int j = 0; j<18;j++) //Cycle through the 18 registers
  {
    volt[j] = SPI.transfer(RDCV);
  }
  digitalWrite(ltcCS,HIGH); //Release LTC chip select
  printV(volt); //FIXME: Remove after debugging
}

void printV(char volt[18])
{
  digitalWrite(status, HIGH); //Trigger status reading LED REVIEW: replace so status can signal an I2C transfer
  Serial.print("Cell 1 V: ");
  Serial.println(((volt[0] & 0xFF) | (volt[1] & 0x0F) << 8)* 0.0015,3);
  Serial.print("Cell 2 V: ");
  Serial.println(((volt[1] & 0xF0) >> 8 | (volt[2] & 0xFF) << 4 )*.0015,3);
  Serial.print("Cell 3 V: ");
  Serial.println(((volt[3] & 0xFF) | (volt[4] & 0x0F) << 8)*.0015,3);
  Serial.print("Cell 4 V: ");
  Serial.println(((volt[4] & 0xF0) >> 8 | (volt[5] & 0xFF) << 4 )*.0015,3);
  Serial.print("Cell 5 V: ");
  Serial.println(((volt[6] & 0xFF) | (volt[7] & 0x0F) << 8)*.0015,3);
  Serial.print("Cell 6 V: ");
  Serial.println((((volt[7] & 0xF0) >> 8 | (volt[8] & 0xFF) << 4))*.0015,3);
  Serial.println("--------------------");
  digitalWrite(status, LOW); //REVIEW: see above
}

void receiveEvent(int howMany)
{
  digitalWrite(status,HIGH);
  readT();
  digitalWrite(status,LOW);

  while (1 < Wire.available())
  {
  }
  balanceByte = Wire.read();
  //Serial.println(balanceByte,BIN);
  balanceFunction();
}

void requestEvent()
{

  while( 1 < Wire.available())
  {
  }
    Wire.write(temp,24);
    Serial.println(temp[1]);
}

void balanceFunction()
{
  char mask = 0x01;
  for(int i = 0; i<8; i++)
  {
    balanceArray[i] = (balanceByte & mask);
    //Serial.println(balanceByte);
    balanceByte = balanceByte >> 1;
  }
}

void readT()
{
  int tempTemp = 0;
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP1);
  temp[0] = (tempTemp & 0xF00) >> 8;
  Serial.println(temp[0],BIN);
  temp[1] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP2);
  temp[2] = (tempTemp & 0xF00) >> 8;
  temp[3] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP3);
  temp[4] = (tempTemp & 0xF00) >> 8;
  temp[5] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP4);
  temp[6] = (tempTemp & 0xF00) >> 8;
  temp[7] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP5);
  temp[8] = (tempTemp & 0xF00) >> 8;
  temp[9] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP6);
  temp[10] = (tempTemp & 0xF00) >> 8;
  temp[11] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP7);
  temp[12] = (tempTemp & 0xF00) >> 8;
  temp[13] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP8);
  temp[14] = (tempTemp & 0xF00) >> 8;
  temp[15] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP9);
  temp[16] = (tempTemp & 0xF00) >> 8;
  temp[17] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP10);
  temp[18] = (tempTemp & 0xF00) >> 8;
  temp[19] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP11);
  temp[20] = (tempTemp & 0xF00) >> 8;
  temp[21] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
  digitalWrite(adcCS,LOW);
  tempTemp = SPI.transfer16(TEMP12);
  temp[22] = (tempTemp & 0xF00) >> 8;
  temp[23] = (tempTemp & 0x0FF);
  digitalWrite(adcCS,HIGH);
}

void printMeanTemp ()
{
  unsigned int averageTemp = 0;
  for(int i = 0; i<11; i++)
  {
    averageTemp += temp[i];
  }
  averageTemp = averageTemp/12;

//  Serial.println(averageTemp);
}