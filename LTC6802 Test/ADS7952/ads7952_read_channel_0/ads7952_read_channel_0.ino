#include <SPI.h>
const int CS = 9;

int returnedValue = 0;
void setup() {
  // put your setup code here, to run once:
Serial.begin(9600);
pinMode(CS, OUTPUT);
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
returnedValue = SPI.transfer16(0x1000);
digitalWrite(CS,HIGH);
Serial.println(returnedValue);
Serial.println("-------------------------");
delay(500);
}
