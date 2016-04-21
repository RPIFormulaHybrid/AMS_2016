#import "SMB.h"
#define ohShit 9

int state = 0;
//State 0 = Standby
//State 1 = Running Mode
//State 2 = Charging Mode
SMB smb1 = SMB(0x01);
SMB smb2 = SMB(0x02);
SMB smb3 = SMB(0x03);
SMB smb4 = SMB(0x04);
SMB smb5 = SMB(0x05);

//int smbs[8] = {0x01, 0x02, 0x03, 0x04, 0x05}; //Address Array for SMBS inside TSAC
SMB smbs[8] = (smb1, smb2, smb3, smb4, smb5); //Address Array for SMBS inside TSAC


void standby(); //Contains the standby loop functions and order
void running(); //Contains the running loop functions and order
void charging(); //Contains the charging loop functions and order
void estop(int stopSeverity); //Function that pulls the ohShit line
                              //low when someting bad is happening
                              //stopSeverity parameter sets how sevear the emergency is,
                              // 0 = pull ohShit super sevear
                              // 2 = Ask PLC to shutdown TSV
                              // 3 = Warn driver


void balance(); //

void setup() {
  // put your setup code here, to run once:
  pinMode(ohShit, OUTPUT);
  digitalWrite(ohShit, HIGH);
  Serial.begin(9600);
  Wire.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if(state == 0)
    standby();
  else if(state == 1)
    running();
  else if(state == 2)
    charging();
  else
    estop(0);
  smb1.pollSMB();
  delay(500);
}

void standby()
{

}

void running()
{

}

void charging()
{

}

void estop(int stopSeverity)
{
  if(stopSeverity == 3)
  {
    //warn driver
  }
  else if(stopSeverity == 2)
  {
    //Ask plc to shutdown
  }
  else
    //Pull ohShit line and shutdown car
    digitalWrite(ohShit, LOW);
}
