
#include "RTClib.h"
#include <EEPROM.h>


DateTime TimeDoorOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeDoorClose (2000, 1, 1, 20, 0, 0);
DateTime TimeNestOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeNestClose (2000, 1, 1, 20, 0, 0);
DateTime TimeLEDOpen (2000, 1, 1, 8, 0, 0);;
DateTime TimeLEDClose  (2000, 1, 1, 20, 0, 0);
TimeSpan SunsetOffset (0, 0, 0, 0);
TimeSpan SunriseOffset (0, 0, 0, 0);
bool DoorMode = true;
int DoorRunningTime=60000;
int NestRunningTime=60000;
bool DayLightSavings=true;

struct EEpromValues{
DateTime TimeDoorOpen;
DateTime TimeDoorClose;
DateTime TimeNestOpen;
DateTime TimeNestClose;
DateTime TimeLEDOpen;
DateTime TimeLEDClose;
TimeSpan SunsetOffset;
TimeSpan SunriseOffset;
bool DoorMode;
int DoorRunningTime;
int NestRunningTime;
bool DayLightSavings;
};

void setup() {
  // put your setup code here, to run once:
 Serial.begin( 9600 );
 EEpromValues mystruct ={TimeDoorOpen, TimeDoorClose, TimeNestOpen, TimeNestClose, TimeLEDOpen, TimeLEDClose, SunsetOffset, SunriseOffset, DoorMode, DoorRunningTime, NestRunningTime, DayLightSavings};
  char buf2[] = "YYMMDD-hh:mm:ss";
  Serial.print(mystruct.TimeDoorOpen.toString(buf2)); 
  EEPROM.put(0, mystruct);
}

void loop() {
  // put your main code here, to run repeatedly:

}
