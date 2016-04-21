#include <SPI.h>
const int CS = 9;
const int LTC = 10;
int returnedValue = 0;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(CS, OUTPUT);
pinMode(LTC, OUTPUT);
digitalWrite(LTC,HIGH);
digitalWrite(CS,HIGH);
SPI.setBitOrder(MSBFIRST);
SPI.setClockDivider(SPI_CLOCK_DIV16);
SPI.begin();
delay(500);
digitalWrite(CS,LOW);
SPI.transfer16(0x1840);
digitalWrite(CS,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
digitalWrite(CS,LOW);
Serial.println(SPI.transfer16(0x1080)&0xFFF); //
digitalWrite(CS,HIGH);

Serial.println("-------------------------");
delay(500);
}
