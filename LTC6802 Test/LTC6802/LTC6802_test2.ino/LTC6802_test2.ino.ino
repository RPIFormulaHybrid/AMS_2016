#include <SPI.h>
// SDO - PIN 12
// SDI - PIN 11
#define WRCFG 0x01 //Write Configuration Registers
#define RDCFG 0x02 // Read config
#define RDCV 0x04 // Read cells
#define STCVAD 0x10 // Start all A/D's - poll status
#define RDFLG 0x06 //Read Flags
#define RDTMP 0x08 //Read Temperatures
#define STCDC 0x60 //A/D converter and poll Status
#define STOWAD 0x20 //Start Test - poll status
#define STTMPAD 0x30// Temperature Reading - ALL
#define address 0x80
#define balCell1  
#define balCell2
#define balCell3
#define balCell4
#define balCell5
#define balCell6
//Functions
byte byteTemp;
void setup()
{
  pinMode(10,OUTPUT);
  digitalWrite(10, HIGH);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE3);
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.begin();
  Serial.begin(9600);
  delay(2000);
  writeReg();
}
void loop()
{
  readV();
  delay(2000);
}
void writeReg()
{
  Serial.println("Writing config...");
  digitalWrite(10, LOW);
  SPI.transfer(address);
  SPI.transfer(WRCFG);
  SPI.transfer(0x01);//0
  SPI.transfer(0x00);//1
  SPI.transfer(0x00);//2
  SPI.transfer(0x00);//3
  SPI.transfer(0x71);//4
  SPI.transfer(0xAB);//5
  digitalWrite(10, HIGH);
  delay(500);
  readReg();
}

void readReg()
{
  Serial.println("Reading config...");
  digitalWrite(10, LOW);
  SPI.transfer(address);
  SPI.transfer(RDCFG);
  for(int i = 0; i < 6; i++)
  {
  byteTemp = SPI.transfer(RDCFG);
  Serial.println(byteTemp, HEX);
  }
  digitalWrite(10, HIGH);
}

void readV()
{
  digitalWrite(10,LOW);
  SPI.transfer(STCVAD);
  delay(50); // wait at least 12ms as per data sheet, p.24
  digitalWrite(10,HIGH);
  byte volt[18];
  digitalWrite(10,LOW);
  SPI.transfer(0x80);
  SPI.transfer(RDCV);
  for(int j = 0; j<18;j++)
  {
  volt[j] = SPI.transfer(RDCV);
  }
  digitalWrite(10,HIGH);

  Serial.print("Cell 1 V: ");
  Serial.println(((volt[0] & 0xFF) | (volt[1] & 0x0F) << 8)*.0015);
  Serial.print("Cell 2 V: ");
  Serial.println(((volt[1] & 0xF0) >> 8 | (volt[2] & 0xFF) << 4 )*.0015);
  Serial.print("Cell 3 V: ");
  Serial.println(((volt[3] & 0xFF) | (volt[4] & 0x0F) << 8)*.0015);
  Serial.print("Cell 4 V: ");
  Serial.println(((volt[4] & 0xF0) >> 8 | (volt[5] & 0xFF) << 4 )*.0015);
  Serial.print("Cell 5 V: ");
  Serial.println(((volt[6] & 0xFF) | (volt[7] & 0x0F) << 8)*.0015);
  Serial.print("Cell 6 V: ");
  Serial.println((((volt[7] & 0xF0) >> 8 | (volt[8] & 0xFF) << 4))*.0015);
  Serial.println("--------------------");
}
