#define ohShit 9

int state = 0;
//State 0 = Standby
//State 1 = Running Mode
//State 2 = Charging Mode

void standby(); //Contains the standby loop functions and order
void running(); //Contains the running loop functions and order
void charging(); //Contains the charging loop functions and order
void estop(int stopSeverity); //Function that pulls the ohShit line
                              //low when someting bad is happening
                              //stopSeverity parameter sets how sevear the emergency is,
                              // 0 = pull ohShit super sevear
                              // 2 = Ask PLC to shutdown TSV
                              // 3 = Warn driver




void setup() {
  // put your setup code here, to run once:
  pinMode(ohShit, OUTPUT);
  digitalWrite(ohShit, HIGH);
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
    //warn driver
    continue;
  else if(stopSeverity == 2)
    //Ask plc to shutdown
    continue;
  else
    //Pull ohShit line and shutdown car
    digitalWrite(ohShit, LOW);
}
