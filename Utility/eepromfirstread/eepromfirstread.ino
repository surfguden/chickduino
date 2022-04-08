
#include "RTClib.h"
#include <EEPROM.h>




struct EEpromValues{
DateTime TimeDoorOpen;
DateTime TimeDoorClose;
DateTime TimeNestOpen;
DateTime TimeNestClose;
TimeSpan SunsetOffset;
TimeSpan SunRiseOffset;
bool DoorMode;
int DoorRunningTime;
int NestRunningTime;
};

void setup() {
  // put your setup code here, to run once:
 Serial.begin( 9600 );
   EEpromValues mystruct; 
  EEPROM.get(0, mystruct);
  char buf2[] = "YYMMDD-hh:mm:ss";
  Serial.print(mystruct.TimeDoorOpen.toString(buf2));

}

void loop() {
  // put your main code here, to run repeatedly:

}
