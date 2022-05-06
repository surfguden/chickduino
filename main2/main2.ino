

// ** Dependencies **
// include CMBMenu
#include "CMBMenu.hpp"
#include "RTClib.h"

#include <EEPROM.h>
#include <Dusk2Dawn.h>
/*
 * mblib - MB library
 * https://github.com/mchlbrnhrd/mbLib
 */

// include the Adafruit LCD library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

// include the Timer Library code:
#include <arduino-timer.h>

//Include elapsedmillis
#include <elapsedMillis.h>

// ********************************************
// definitions
// ********************************************

// Adafruit LCD
// These #defines make it easy to set the backlight color
#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

// ********************************************
// Globals
// ********************************************

//Menu 
// define text to display
const char g_MenuDoor[] PROGMEM = {"1. Door"};
const char g_MenuDoorSetMode[] PROGMEM = {"1.1 SetDoorMode"};
const char g_MenuDoorSetTimer[] PROGMEM = {"1.2 SetDoorTimer"};
const char g_MenuDoorSetOffset[] PROGMEM = {"1.3 SetSunOffset"};
const char g_MenuDoorForceOpen[] PROGMEM = {"1.4 Open Door"};
const char g_MenuDoorForceClose[] PROGMEM = {"1.5 Close Door"};
const char g_MenuNest[] PROGMEM = {"2. Nest"};
const char g_MenuNestSetTimer[] PROGMEM = {"2.1 SetNestTimer"};
const char g_MenuNestForceOpen[] PROGMEM = {"2.2 Open Nest"};
const char g_MenuNestForceClose[] PROGMEM = {"2.3 Close Nest"};
const char g_MenuLED[] PROGMEM = {"3. LED"};
const char g_MenuLEDSetTimer[] PROGMEM = {"3.1 SetLEDTimer"};
const char g_MenuLEDForceOpen[] PROGMEM = {"3.2 Turn on LED"};
const char g_MenuLEDForceClose[] PROGMEM = {"3.2 Turn off LED"};
const char g_MenuTime[] PROGMEM = {"4. Time"};
const char g_MenuTimeSetDate[] PROGMEM = {"4.1 Set Date"};
const char g_MenuTimeSetTime[] PROGMEM = {"4.2 Set Time"};
const char g_MenuTimeSetDayLightSavings[] PROGMEM = {"4.3SetSummerTime"};
const char g_MenuInfoScreen[] PROGMEM = {"5. InfoScreen"};


// define function IDs
enum MenuFID {
    MenuDummy,
    MenuDoor,
    MenuDoorSetMode,
    MenuDoorSetTimer,
    MenuDoorSetOffset,
    MenuDoorForceOpen,
    MenuDoorForceClose,
    MenuNest,
    MenuNestSetTimer,
    MenuNestForceOpen,
    MenuNestForceClose,
    MenuLED,
    MenuLEDSetTimer,
    MenuLEDForceOpen,
    MenuLEDForceClose,
    MenuTime,
    MenuTimeSetDate,
    MenuTimeSetTime,
    MenuTimeSetDayLightSavings,
    MenuInfoScreen
};

// define key types
enum KeyType {
  KeyNone, // no key is pressed
  KeyLeft,
  KeyRight,
  KeyUp,
  KeyDown,
  KeySelect
};


//RTC
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// create global LCD instance
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// create global CMBMenu instance
// (here for maximum 100 menu entries)
CMBMenu<100> g_Menu;

//Create RTC Instance
RTC_PCF8523 rtc;


//Time Related
bool DayLightSavings = true; //True is "summer time", F is WinterTime. 

//Other Sketch globals
//Door Related
bool DoorMode = true; //True is Timer, False is Sunset/Sundown 
DateTime TimeDoorOpen (2000, 1, 1, 8, 0, 0); //This is the Timer setting. Should be renamed TimerDoorOpen
DateTime TimeDoorClose (2000, 1, 1, 20, 0, 0); //This is the Timer setting. Should be renamed TimerDoorClose
DateTime Sunrise (2000, 1, 1, 8, 0, 0);
DateTime Sunset (2000, 1, 1, 8, 0, 0);
TimeSpan SunsetOffset;
TimeSpan SunriseOffset;
unsigned long DoorRunningTime=60000; //This is the time (ms) it takes for the door to open or close, I.e the timer after the door stopped function is called
bool DoorRunningOpen=false; // Could be declared as static
bool DoorRunningClose=false; // Could be declared as static
Timer<1, millis, const char *> DoorHandlerTimer;

//Nest Related
DateTime TimeNestOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeNestClose (2000, 1, 1, 20, 0, 0);
unsigned long NestRunningTime=60000; //This is the time (ms) it takes for the nest to open or close, I.e the timer after the door stopped function is called
bool NestRunningOpen=false; // Could be declared as static
bool NestRunningClose=false; // Could be declared as static
Timer<1, millis, const char *> NestHandlerTimer;

//LED Related
DateTime TimeLEDOpen (2000, 1, 1, 8, 0, 0);;
DateTime TimeLEDClose  (2000, 1, 1, 20, 0, 0);
bool LEDRunningOpen=false; // Could be declared as static
bool LEDRunningClose=false; // Could be declared as static

//Idle Timer
elapsedMillis IdleTimeElapsed;
elapsedMillis IdleScrollTimeElapsed;
#define IDLEWAITTIME 60000 // milliseconds

//Eeprom
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
unsigned long DoorRunningTime;
unsigned long NestRunningTime;
bool DayLightSavings;
};

//Dusk2Dawn
Dusk2Dawn City(56.0505, 12.69401, 1);   // Available methods are sunrise() and sunset(). Arguments are year, month,day, and if Daylight Saving Time is in effect.
DateTime Midnight (2000, 1, 1, 0, 0, 0);

//Pins For Serial Relay


byte relay=0;


// ********************************************
// setup
// ********************************************

uint8_t i=0;
void setup()
{
    //Pins for  Relay
  pinMode(0, OUTPUT);
  pinMode(1,  OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4,  OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7,  OUTPUT);
  SendSerial();
  
  // LCD
  lcd.begin(16, 2);
  lcd.print(F("Hello, Eggmakers"));
  lcd.noAutoscroll();
  
  delay(1000);

  // ** menu **
  // add nodes to menu (layer, string, function ID)
  g_Menu.addNode(0, g_MenuDoor, MenuDoor);
  g_Menu.addNode(1, g_MenuDoorSetMode, MenuDoorSetMode);
  g_Menu.addNode(1, g_MenuDoorSetTimer, MenuDoorSetTimer);
  g_Menu.addNode(1, g_MenuDoorSetOffset, MenuDoorSetOffset);
  g_Menu.addNode(1, g_MenuDoorForceOpen, MenuDoorForceOpen);
  g_Menu.addNode(1, g_MenuDoorForceClose, MenuDoorForceClose);
  g_Menu.addNode(0, g_MenuNest, MenuNest);
  g_Menu.addNode(1, g_MenuNestSetTimer, MenuNestSetTimer);
  g_Menu.addNode(1, g_MenuNestForceOpen, MenuNestForceOpen);
  g_Menu.addNode(1, g_MenuNestForceClose, MenuNestForceClose);
  g_Menu.addNode(0, g_MenuLED, MenuLED);
  g_Menu.addNode(1, g_MenuLEDSetTimer, MenuLEDSetTimer);
  g_Menu.addNode(1, g_MenuLEDForceOpen, MenuLEDForceOpen);
  g_Menu.addNode(1, g_MenuLEDForceClose, MenuLEDForceClose);
  g_Menu.addNode(0, g_MenuTime, MenuTime);
  g_Menu.addNode(1, g_MenuTimeSetDate, MenuTimeSetDate);
  g_Menu.addNode(1, g_MenuTimeSetTime, MenuTimeSetTime);
  g_Menu.addNode(1, g_MenuTimeSetDayLightSavings, MenuTimeSetDayLightSavings);  
  g_Menu.addNode(0, g_MenuInfoScreen, MenuInfoScreen);


  // ** menu **
  // build menu and print menu
  // (see terminal for output)
  const char* info;
  g_Menu.buildMenu(info);
  g_Menu.printMenu();

  // print current menu entry
   printMenuEntry(info);


   //RTC
  if (! rtc.begin()) {

    while (1) delay(10);
  }
  if (! rtc.initialized() || rtc.lostPower()) {
    //Serial.println(F("RTC is NOT initialized, let's set the time!"));
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    //Important! Set Computer time to winter time equivalent!
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
  rtc.start();

  //EEPROM
  ReadFromEeprom();
}



// ********************************************
// loop
// ********************************************

void loop()
{
  //get time
   DateTime now = rtc.now();
  //Adjust For Daylight Savings. No need to adjust RTC
  now= now+TimeSpan (0,DayLightSavings,0,0);
  // Run SunriseCalculator After DaylightSavings Compensation!
  SunriseCalculator(now);
  //Run Door Handler
  RunDoorHandler(now);
  //Run Nest Handler
  RunNestHandler(now);
    //Run LED Handler
  RunLEDHandler(now);


  
  // function ID
  int fid = 0;
  // info text from menu
  const char* info;
  // go to deeper or upper layer?
  bool layerChanged=false;
  // determine pressed key by polling buttons
  KeyType key = getKey();
  // Check and Run idle task if no key is pressed
  IdleTask(key, now);

  // ** menu **
  // call menu methods regarding pressed key
  switch(key) {
    case KeyLeft:
      g_Menu.exit();
      break;
    case KeyRight:
      g_Menu.enter(layerChanged);
      break;
    case KeyUp:
      g_Menu.left();
      break;
    case KeyDown:
      g_Menu.right();
      break;
    default:
      break;
  }

  // ** menu **
  // pint/update menu when key was pressed
  // and get current function ID "fid"
  if (KeyNone != key) {
    fid = g_Menu.getInfo(info);
    printMenuEntry(info);
  }
  // ** menu **
  // do action regarding function ID "fid"
  if ((0 != fid) && (KeyRight == key) && (!layerChanged)) {
    switch (fid) {
      case MenuDoorSetMode:
        SetDoorMode();
        break;
      case MenuDoorSetTimer:
        SetDoorTimer(); 
        break;
      case MenuDoorSetOffset:
        SetSunriseSunsetOffset(); 
        break;
      case MenuDoorForceOpen:
        OpenDoor();
        break;
      case MenuDoorForceClose:
        CloseDoor();
        break; 
      case MenuNestSetTimer:
        SetNestTimer(); 
        break;  
      case MenuNestForceOpen:
        OpenNest();
        break;
      case MenuNestForceClose:
        CloseNest();
        break;
      case MenuLEDSetTimer:
        SetLEDTimer(); 
        break;
      case MenuLEDForceOpen:
        OpenLED(); 
        break;
      case MenuLEDForceClose:
        CloseLED(); 
        break;
      case MenuTimeSetTime:
        SetDayLightSavings();
        SetCurrentTime(now);
        break;
      case MenuTimeSetDate:
        SetCurrentDate(now);
        break;
      case MenuTimeSetDayLightSavings:
        SetDayLightSavings(); 
        break;
      case MenuInfoScreen:
        DisplayInfo(now); 
        break;

      default:
        break;
    }
  }
}


// ********************************************
// Functions Used in Sketch Below:
// ********************************************


// ********************************************
// ** menu **
// printMenuEntry
// ********************************************
void printMenuEntry(const char* f_Info)
{
  String info_s;
  MBHelper::stringFromPgm(f_Info, info_s);

  // when using LCD: add/replace here code to
  // display info on LCD
  //Serial.println("----------------");
  //Serial.println(info_s);
  //Serial.println("----------------");
  
  // print on LCD
  lcd.clear();
  lcd.print(info_s);

  // you can print here additional infos into second line of LCD
  // g_Lcd.setCursor(0, 1);
  // g_Lcd.print(info_s);
}

// ********************************************
// getKey
// ********************************************
KeyType getKey()
{
  KeyType key = KeyNone;

    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      lcd.clear();
      lcd.setCursor(0,0);
      delay(150);
    if (buttons & BUTTON_UP) {      
      key = KeyUp;
    }
    if (buttons & BUTTON_DOWN) {    
      key = KeyDown;
    }
    if (buttons & BUTTON_LEFT) {
      key = KeyLeft;
    }
    if (buttons & BUTTON_RIGHT) {
      key = KeyRight;
    }
    if (buttons & BUTTON_SELECT) {
      key = KeySelect;
    }
  }
 return key;
}

// ********************************************
// SendSerial
// ********************************************
//Turn off all relays except 7 and 8 (LED)before changing them, inorder to prevent voltage surge. 
void SendSerial(){
  for (i=0; i<8; i++){
    digitalWrite(i, bitRead(relay, i));
    delay(100); //Turn on the relay slowly
  }
}

// ********************************************
// OpenDoor
// ********************************************
void OpenDoor()
{
  if(DoorRunningOpen==false){
    DoorHandlerTimer.cancel();
    DoorHandlerTimer.in(DoorRunningTime, StopDoor);
    lcd.clear();
    lcd.print(F("Opening Door"));
    DoorRunningOpen=true;
    DoorRunningClose=false;
    bitWrite(relay, 0, 0);
    bitWrite(relay, 1, 0);
    SendSerial();
    bitWrite(relay, 0, 1);
    bitWrite(relay, 1, 0);
    SendSerial();
  }
}

// ********************************************
// CloseDoor
// ********************************************
void CloseDoor()
{
  if(DoorRunningClose==false){
    DoorHandlerTimer.cancel();
    DoorHandlerTimer.in(DoorRunningTime, StopDoor);
    lcd.clear();
    lcd.print(F("Closing Door"));
    DoorRunningOpen=false;
    DoorRunningClose=true;
    bitWrite(relay, 0, 0);
    bitWrite(relay, 1, 0);
    SendSerial();
    bitWrite(relay, 0, 0);
    bitWrite(relay, 1, 1);
    SendSerial();

  }  
}
// ********************************************
// StopDoor
// ********************************************
bool StopDoor()  //This function is only used by the DoorHandlerTimer
{
  lcd.clear();
  lcd.print(F("Stopping Door"));
  DoorRunningOpen=false;
  DoorRunningClose=false;
  bitWrite(relay, 0, 0);
  bitWrite(relay, 1, 0);
  SendSerial();
  return false;
}
// ********************************************
// OpenNest
// ********************************************
void OpenNest()
{
  if(NestRunningOpen==false){
    NestHandlerTimer.cancel();
    NestHandlerTimer.in(NestRunningTime, StopNest);
    lcd.clear();
    lcd.print(F("Opening Nest"));
    NestRunningClose=false;
    NestRunningOpen=true;
    bitWrite(relay, 2, 0);
    bitWrite(relay, 3, 0);
    bitWrite(relay, 4, 0);
    bitWrite(relay, 5, 0);
    SendSerial();
    bitWrite(relay, 2, 1);
    bitWrite(relay, 3, 0);
    bitWrite(relay, 4, 1);
    bitWrite(relay, 5, 0);
    SendSerial();
  }
}

// ********************************************
// CloseNest
// ********************************************
void CloseNest()
{
  if(NestRunningClose==false){
    NestHandlerTimer.cancel();
    NestHandlerTimer.in(NestRunningTime, StopNest);
    lcd.clear();
    lcd.print(F("Closing Nest"));
    NestRunningClose=true;
    NestRunningOpen=false;
    bitWrite(relay, 2, 0);
    bitWrite(relay, 3, 0);
    bitWrite(relay, 4, 0);
    bitWrite(relay, 5, 0);
    SendSerial();
    bitWrite(relay, 2, 0);
    bitWrite(relay, 3, 1);
    bitWrite(relay, 4, 0);
    bitWrite(relay, 5, 1);
    SendSerial();
  }  
}

// ********************************************
// StopNest
// ********************************************
bool StopNest()  //This function is only used by the DoorHandlerTimer
{
  lcd.clear();
  lcd.print(F("Stopping Nest"));
  NestRunningOpen=false;
  NestRunningClose=false;
  bitWrite(relay, 2, 0);
  bitWrite(relay, 3, 0);
  bitWrite(relay, 4, 0);
  bitWrite(relay, 5, 0);
  SendSerial();
  return false;
}

// ********************************************
// OpenLED  (The same as turn on LED)
// ********************************************
void OpenLED()
{
  if(LEDRunningOpen==false){
    lcd.clear();
    lcd.print(F("Turning On LED"));
    LEDRunningOpen=true;
    LEDRunningClose=false; //This can be Improved
    bitWrite(relay, 6, 1);
    SendSerial();
  }
}

// ********************************************
// CloseLED
// ********************************************
void CloseLED()
{
  if(LEDRunningClose==false){
    lcd.clear();
    lcd.print(F("Turning off LED"));
    LEDRunningClose=true;
    LEDRunningOpen=false; //This can be Improved
    bitWrite(relay, 6, 0);
    SendSerial();
  }  
}

// ********************************************
// SunRiseCalculator
// ********************************************
void SunriseCalculator(DateTime now){
    int SunriseMinutes = City.sunrise(now.year(), now.month(), now.day(), DayLightSavings);  //Returns Sunrise as minutes since midnight
    int SunsetMinutes = City.sunset(now.year(), now.month(), now.day(), DayLightSavings);  //Returns Sunrise as minutes since midnight
    Sunrise= Midnight +TimeSpan (0,SunriseMinutes/60,SunriseMinutes%60, 0) ;
    Sunset= Midnight +TimeSpan (0,SunsetMinutes/60,SunsetMinutes%60, 0);
}


// ********************************************
// RunDoorHandler
// ********************************************
void RunDoorHandler(DateTime now) // This function checks time. If any conditions are true (on the Second) these actions will be executed. Door is automatically stopped after int DoorRunningTime
{
  DoorHandlerTimer.tick();
  if(DoorMode){ //Timer Mode
    if(now.hour()==TimeDoorOpen.hour() && now.minute()==TimeDoorOpen.minute() ){    
      OpenDoor();    
    }
    else if(now.hour()==TimeDoorClose.hour() && now.minute()==TimeDoorClose.minute() ){
      CloseDoor();
    }
  }
  if (!DoorMode){ //SunsetMode
    DateTime SunOpen = Sunrise + SunriseOffset; 
    DateTime SunClose = Sunset + SunsetOffset; 
    if(now.hour()==SunOpen.hour() && now.minute()==SunOpen.minute() ){    
      OpenDoor();    
    }
    else if(now.hour()==SunClose.hour() && now.minute()==SunClose.minute()) {
      CloseDoor();
    }
  }
}



//DateTime Sunrise (2000, 1, 1, 0, 0, 0);
//DateTime Sunset (2000, 1, 1, 0, 0, 0);
// ********************************************
// RunNestHandler
// ********************************************
void RunNestHandler(DateTime now) //This function checks time. If any conditions are true (on the Second) these actions will be executed. Door is automatically stopped after int NestRunningTime
{
  NestHandlerTimer.tick();
  if(now.hour()==TimeNestOpen.hour() && now.minute()==TimeNestOpen.minute()){    
    OpenNest();    
  }
  else if(now.hour()==TimeNestClose.hour() && now.minute()==TimeNestClose.minute()){
    CloseNest();
    }
}

// ********************************************
// RunLEDHandler
// ********************************************
void RunLEDHandler(DateTime now) //This function checks time. If any conditions are true (on the Second) these actions will be executed. 
{
  if(now.hour()==TimeLEDOpen.hour() && now.minute()==TimeLEDOpen.minute() ){    
    OpenLED();    
  }
  else if(now.hour()==TimeLEDClose.hour() && now.minute()==TimeLEDClose.minute() ){
    CloseLED();
    }
}

// ********************************************
//void SetLEDTimer()
// ********************************************
void SetLEDTimer()
{
  lcd.setBacklight(RED);
  TimeLEDOpen=SetTimerHour(TimeLEDOpen, "LED ON");
  TimeLEDOpen=SetTimerMinute(TimeLEDOpen, "LED ON");
  TimeLEDClose=SetTimerHour(TimeLEDClose,"LED OFF");
  TimeLEDClose=SetTimerMinute(TimeLEDClose, "LED OFF");
  WriteToEeprom();
  lcd.setBacklight(TEAL);
}

// ********************************************
//void SetNestTimer()
// ********************************************
void SetNestTimer()
{
  lcd.setBacklight(RED);
  TimeNestOpen=SetTimerHour(TimeNestOpen, "nest open");
  TimeNestOpen=SetTimerMinute(TimeNestOpen, "nest open");
  TimeNestClose=SetTimerHour(TimeNestClose,"nest close");
  TimeNestClose=SetTimerMinute(TimeNestClose, "nest close");
  WriteToEeprom();
  lcd.setBacklight(TEAL);
}

// ********************************************
//void SetDoorTimer()
// ********************************************
void SetDoorTimer()
{
  lcd.setBacklight(RED);
  TimeDoorOpen=SetTimerHour(TimeDoorOpen, "door open");
  TimeDoorOpen=SetTimerMinute(TimeDoorOpen, "door open");
  TimeDoorClose=SetTimerHour(TimeDoorClose,"door close");
  TimeDoorClose=SetTimerMinute(TimeDoorClose, "door close");
  WriteToEeprom();
  lcd.setBacklight(TEAL);
}

// ********************************************
//void SetCurrentDate() 
// ********************************************
void SetCurrentDate(DateTime now)
{
  lcd.setBacklight(RED);
  now=SetTimerYear(now, "current");
  now=SetTimerMonth(now, "current");
  now=SetTimerDay(now,"current");
  rtc.adjust(now);
  lcd.setBacklight(TEAL);
}

// ********************************************
//void SetCurrenTime() Implement a RTC adjust here
// ********************************************
void SetCurrentTime(DateTime now)
{
  lcd.setBacklight(RED);
  now=SetTimerHour(now, "current");
  now=SetTimerMinute(now, "current");
  rtc.adjust(now- TimeSpan (0,DayLightSavings,0,0));
  lcd.setBacklight(TEAL);
}


// ********************************************
//void SetDoorMode()
// ********************************************
void SetDoorMode()
{
  lcd.setBacklight(RED);
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Set Door MODE"));
  lcd.setCursor(0,1);
  if (DoorMode){
    lcd.print(F("Timer Mode  "));
  }
  if (!DoorMode){
    lcd.print(F("Sunset Mode  "));
  }

  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        DoorMode=!DoorMode;    
      }
      if (buttons & BUTTON_DOWN) {
        DoorMode=!DoorMode;  
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
      lcd.setCursor(0,1);  
      if (DoorMode){
        lcd.print(F("Timer Mode "));
      }
      if (!DoorMode){
        lcd.print(F("Sunset Mode "));
      }
    }   
  }
  lcd.noBlink();
  WriteToEeprom();
  lcd.setBacklight(TEAL);
}


// ********************************************
//void SetDayLightSavings()
// ********************************************
void SetDayLightSavings()
{
  lcd.setBacklight(RED);
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("SetDayLghtSaving"));
  lcd.setCursor(0,1);
  if (!DayLightSavings){
    lcd.print(F("Winter Time  "));
  }
  if (DayLightSavings){
    lcd.print(F("Summer Time  "));
  }

  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        DayLightSavings=!DayLightSavings;    
      }
      if (buttons & BUTTON_DOWN) {
        DayLightSavings=!DayLightSavings;  
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
      lcd.setCursor(0,1);  
      if (!DayLightSavings){
        lcd.print(F("Winter Time  "));
      }
      if (DayLightSavings){
        lcd.print(F("Summer Time  "));
      }
    }   
  }
  lcd.noBlink();
  WriteToEeprom();
  lcd.setBacklight(TEAL);
}


// ********************************************
//Void SetSunriseSunsetOffset()
// ********************************************
void SetSunriseSunsetOffset(){
  SunriseOffset=SetOffset(SunriseOffset, "SetSunriseOffset");
  SunsetOffset=SetOffset(SunsetOffset, "SetSunsetOffset");
  WriteToEeprom();
}

// ********************************************
//TimeSpan SetOffset()
// ********************************************
TimeSpan SetOffset(TimeSpan span, String mystring)
{
  int offset=span.hours()*60+span.minutes();
  lcd.setBacklight(RED);
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(mystring);
  lcd.setCursor(0,1);
  lcd.print(offset);
  lcd.print("minutes");

  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(50);
      if (buttons & BUTTON_UP) {
        offset++;   
      }
      if (buttons & BUTTON_DOWN) {
        offset--;  
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(mystring);
      lcd.setCursor(0,1);
     
      lcd.print(offset);
      lcd.print("minutes");
    }   
  }
  lcd.noBlink();
  lcd.setBacklight(TEAL);
  lcd.noAutoscroll();
  return TimeSpan(0, offset/60, offset%60, 0);
}



// ********************************************
// SetTimerYear.  
// ********************************************
DateTime SetTimerYear(DateTime time, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Year");
  lcd.setCursor(0,1);
  char buf2[] = "YYYYMMMDD-hh:mm";
  lcd.print(time.toString(buf2));
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        time=time+TimeSpan(365,0,0,0);    
      }
      if (buttons & BUTTON_DOWN) {
        time=time+TimeSpan(-365,0,0,0); 
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
    lcd.setCursor(0,1);
   char buf2[] = "YYYYMMMDD-hh:mm";
   lcd.print(time.toString(buf2));
    }   
  }
  lcd.noBlink();
  return time;
}
// ********************************************
// SetTimerMonth.  
// ********************************************
DateTime SetTimerMonth(DateTime time, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Month");
  lcd.setCursor(0,1);
  char buf2[] = "YYYYMMMDD-hh:mm";
  lcd.print(time.toString(buf2));
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        if(time.month()<12){
          time= DateTime (time.year(), time.month()+1, time.day(), time.hour(), time.minute(), time.second());    
        }
      }
      if (buttons & BUTTON_DOWN) {
        if(time.month()>1){
          time= DateTime (time.year(), time.month()-1, time.day(), time.hour(), time.minute(), time.second());
        }
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
    lcd.setCursor(0,1);
   char buf2[] = "YYYYMMMDD-hh:mm";
   lcd.print(time.toString(buf2));
    }   
  }
  lcd.noBlink();
  return time;
}

// ********************************************
// SetTimerDay.  
// ********************************************
DateTime SetTimerDay(DateTime time, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Day");
  lcd.setCursor(0,1);
  char buf2[] = "YYYYMMMDD-hh:mm";
  lcd.print(time.toString(buf2));
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        time=time+TimeSpan(1,0,0,0);    
      }
      if (buttons & BUTTON_DOWN) {
        time=time+TimeSpan(-1,0,0,0); 
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
    lcd.setCursor(0,1);
   char buf2[] = "YYYYMMMDD-hh:mm";
   lcd.print(time.toString(buf2));
    }   
  }
  lcd.noBlink();
  return time;
}

// ********************************************
// SetTimerHour.  
// ********************************************
DateTime SetTimerHour(DateTime time, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Hr");
  lcd.setCursor(0,1);
  char buf1[] = "hh:mm";
  lcd.print(time.toString(buf1));
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        time=time+TimeSpan(0,1,0,0);    
      }
      if (buttons & BUTTON_DOWN) {
        time=time+TimeSpan(0,-1,0,0); 
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
    lcd.setCursor(0,1);
    char buf1[] = "hh:mm";  
    lcd.print(time.toString(buf1));
    }   
  }
  lcd.noBlink();
  return time;
}

// ********************************************
// SetTimerMinute.  
// ********************************************

DateTime SetTimerMinute(DateTime time, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +"Min");
  lcd.setCursor(0,1);
  char buf1[] = "hh:mm";
  lcd.print(time.toString(buf1));
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        time=time+TimeSpan(0,0,1,0);    
      }
      if (buttons & BUTTON_DOWN) {
        time=time+TimeSpan(0,0,-1,0); 
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print(F("Value Set!"));
        set=false;    
      }
    lcd.setCursor(0,1);
    char buf1[] = "hh:mm"; 
    lcd.print(time.toString(buf1)); 
    }   
  }
  lcd.noBlink();
  return time;
}

// ********************************************
// IdleTask
// ********************************************
void IdleTask(KeyType key, DateTime now) 
{
  if(key==KeyNone)
  {
    if (IdleTimeElapsed > IDLEWAITTIME)
    {
      if(IdleScrollTimeElapsed>2000)
      {  
        DisplayInfo(now);
        IdleScrollTimeElapsed=0;      
      }
    }
  }
  else
  {
   IdleTimeElapsed = 0;    
  }
}

// ********************************************
// WriteToEeprom
// ********************************************

void WriteToEeprom(){
   EEpromValues mystruct ={TimeDoorOpen, TimeDoorClose, TimeNestOpen, TimeNestClose, TimeLEDOpen, TimeLEDClose, SunsetOffset, SunriseOffset, DoorMode, DoorRunningTime, NestRunningTime, DayLightSavings};
   EEPROM.put(0, mystruct);
   lcd.clear();
   lcd.print(F("Values Stored!"));
   delay(1000);
}

// ********************************************
// ReadFromEeprom
// ********************************************
void ReadFromEeprom(){
  EEpromValues mystruct; 
  EEPROM.get(0, mystruct);
  TimeDoorOpen=mystruct.TimeDoorOpen;
  TimeDoorClose=mystruct.TimeDoorClose;
  TimeNestOpen=mystruct.TimeNestOpen;
  TimeNestClose=mystruct.TimeNestClose;
  TimeLEDOpen=mystruct.TimeLEDOpen;
  TimeLEDClose=mystruct.TimeLEDClose;
  SunsetOffset=mystruct.SunsetOffset;
  SunriseOffset=mystruct.SunriseOffset;
  DoorMode=mystruct.DoorMode;
  DoorRunningTime=mystruct.DoorRunningTime;
  NestRunningTime=mystruct.NestRunningTime;
  DayLightSavings=mystruct.DayLightSavings;
}

// Different Display Functions Below

// ********************************************
// DisplayInfo
// ********************************************
void DisplayInfo(DateTime now){
  static int InfoIndex;
  switch(InfoIndex){
    case 1:
      DisplayDateTime(now);
      break;
    case 2:
      DisplayDayLightSavings();
      break;
    case 3:
      //DisplayBattery();
      break;
    case 4:
      DisplayDoorMode();
      break;
    case 5:
      DisplayTimeGeneral(TimeDoorOpen, F("DoorTimer Open"));
      break;
    case 6:
      DisplayTimeGeneral(TimeDoorClose, F("DoorTimer Close"));
      break;
    case 7:
      DisplayTimeGeneral(TimeNestOpen, F("NestTimer Open"));
      break;
    case 8:
      DisplayTimeGeneral(TimeNestClose, F("Nest Timer Close"));
      break;
    case 9:
      DisplayTimeGeneral(TimeLEDOpen, F("LED Timer ON"));
      break;
    case 10:
      DisplayTimeGeneral(TimeLEDClose, F("LED Timer Off"));
      break;
    case 11:
      DisplayTimeGeneral(Sunrise, F("Todays Sunrise:"));
      break;
    case 12:
      DisplayTimeGeneral(Sunset, F("Todays Sunset:"));
      break;
    case 13:
      DisplayDoorOpeningTime();
      break;
      case 14:
      DisplayDoorClosingTime();
      break;
    default:
      InfoIndex=0;
      break;
  }
  InfoIndex++;
}

// ********************************************
// DisplayDoorOpeningTime
// ********************************************
void DisplayDoorOpeningTime(){
  if(!DoorMode){
    DisplayTimeGeneral(Sunset+SunsetOffset, "Door Will Close");  
  }
  if(DoorMode){
    DisplayTimeGeneral(TimeDoorClose, "Door Will Close");  
  }
}

// ********************************************
// DisplayDoorClosingTime
// ********************************************
void DisplayDoorClosingTime(){
  if(!DoorMode){
    DisplayTimeGeneral(Sunrise+SunriseOffset, "Door Will Open");  
  }
  if(DoorMode){
    DisplayTimeGeneral(TimeDoorOpen, "Door Will Open");  
  }
}

// ********************************************
// DisplayDateTime
// ********************************************
void DisplayDateTime(DateTime now)
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Current DateTime:"));
  lcd.setCursor(0,1);
  char buf2[] = "YYMMDD-hh:mm:ss";
  lcd.print(now.toString(buf2)); 
}
// ********************************************
// DisplayBattery
// ********************************************
void DisplayBattery()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Battery Level:"));
  lcd.setCursor(0,1);
  lcd.print(F("11,7V (Dummyval)"));
}
// ********************************************
// DisplayDoor Mode
// ********************************************
void DisplayDoorMode()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Door Mode:"));
  lcd.setCursor(0,1);
  if (DoorMode){
    lcd.print(F("Timer Mode"));
  }
  if (!DoorMode){
  lcd.print(F("Sunset Mode"));
  }
}

// ********************************************
// DisplayDayLightSavings
// ********************************************
void DisplayDayLightSavings()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Daylight Savings"));
  lcd.setCursor(0,1);
  if (!DayLightSavings){
    lcd.print(F("WinterTime"));
  }
  if (DayLightSavings){
  lcd.print(F("SummerTime"));
  }
}

// ********************************************
// DisplayTimeGeneral
// ********************************************
void DisplayTimeGeneral(DateTime time, String info){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(info);
  lcd.setCursor(0,1); 
  char buf1[] = "hh:mm"; 
  lcd.print(time.toString(buf1));
}


  

// ========================================================================
// naming convention
// prefix:
//   g  : global variable
//   f  : function/method variable
//   m  : private member of class
//      : without prefix: public member of class
//
//
// ========================================================================
