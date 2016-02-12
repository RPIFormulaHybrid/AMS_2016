void setup() {
  // put your setup code here, to run once:
pinMode(9,OUTPUT);
pinMode(6,OUTPUT);
pinMode(5,OUTPUT);
digitalWrite(9,HIGH);
digitalWrite(6,HIGH);
digitalWrite(5,HIGH);
delay(5000);
}

void loop() {
  // put your main code here, to run repeatedly:
  int i = 0;
for(i=0;i<255;i++){
  analogWrite(9,i);
  analogWrite(6,i);
  analogWrite(5,i);
  delay(10);
}


for(i=255;i>0;i--){
  analogWrite(9,i);
  analogWrite(6,i);
  analogWrite(5,i);
  delay(10);
}
for(i=0;i<255;i++){
  analogWrite(9,i);
  delay(10);
}
for(i=255;i>0;i--){
  analogWrite(9,i);
  delay(10);
}
for(i=0;i<255;i++){
  analogWrite(6,i);
  delay(10);
}
for(i=255;i>0;i--){
  analogWrite(6,i);
  delay(10);
}
for(i=0;i<255;i++){
  analogWrite(5,i);
  delay(10);
}
for(i=255;i>0;i--){
  analogWrite(5,i);
  delay(10);
}
} 

