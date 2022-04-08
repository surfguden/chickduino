
#include "RTClib.h"
#include <EEPROM.h>


DateTime TimeDoorOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeDoorClose (2000, 1, 1, 20, 0, 0);
DateTime TimeNestOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeNestClose (2000, 1, 1, 20, 0, 0);
TimeSpan SunsetOffset (0, 0, 0, 0);
TimeSpan SunRiseOffset (0, 0, 0, 0);
bool DoorMode = true;
int DoorRunningTime=10000;
int NestRunningTime=10000;

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
 EEpromValues mystruct ={TimeDoorOpen, TimeDoorOpen, TimeNestOpen, TimeNestClose, SunsetOffset, SunRiseOffset, DoorMode, DoorRunningTime, NestRunningTime};
  char buf2[] = "YYMMDD-hh:mm:ss";
  Serial.print(mystruct.TimeDoorOpen.toString(buf2)); 
  EEPROM.put(0, mystruct);
}

void loop() {
  // put your main code here, to run repeatedly:

}
