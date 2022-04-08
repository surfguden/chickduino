//TBD NestHandler, Battery, SunsetSundown, EEPROM, MotorShield


// ** Dependencies **
// include CMBMenu
#include "CMBMenu.hpp"
#include "RTClib.h"
#include <Adafruit_MotorShield.h>
#include <EEPROM.h>
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

//Menu 
// define text to display
const char g_MenuDoor[] PROGMEM = {"1. Door"};
const char g_MenuDoorSettings[] PROGMEM = {"1.1 Door Settings"};
const char g_MenuDoorMode[] PROGMEM = {"1.1.1 Door Mode"};
const char g_MenuDoorModeSun[] PROGMEM = {"1.1.1.1 Sunset"};
const char g_MenuDoorModeTimer[] PROGMEM = {"1.1.1.2 Timer"};
const char g_MenuDoorSetTimer[] PROGMEM = {"1.1.2 Set Timer"};
const char g_MenuDoorForceOpen[] PROGMEM = {"1.2 Open Door"};
const char g_MenuDoorForceClose[] PROGMEM = {"1.3 Close Door"};
const char g_MenuNest[] PROGMEM = {"2. Nest"};
const char g_MenuNestForceOpen[] PROGMEM = {"2.1 Open Nest"};
const char g_MenuNestForceClose[] PROGMEM = {"2.2 Close Nest"};
const char g_MenuBattery[] PROGMEM = {"3. Battery"};
const char g_MenuDisplayTime[] PROGMEM = {"4. Display Time"};
const char g_MenuInfoScreen[] PROGMEM = {"5. InfoScreen"};


// define function IDs
enum MenuFID {
    MenuDummy,
    MenuDoor,
    MenuDoorMode,
    MenuDoorSettings,
    MenuDoorModeSun,
    MenuDoorModeTimer,
    MenuDoorForceOpen,
    MenuDoorForceClose,
    MenuNest,
    MenuNestForceOpen,
    MenuNestForceClose,
    MenuDisplayTime,
    MenuDoorSetTimer,
    MenuBattery
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

// ********************************************
// Globals
// ********************************************

// create global LCD instance
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// create global CMBMenu instance
// (here for maximum 100 menu entries)
CMBMenu<100> g_Menu;

//Create RTC Instance
RTC_PCF8523 rtc;

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

// Select which 'port' M1, M2, M3 or M4. In this case, M1
Adafruit_DCMotor *DoorMotor = AFMS.getMotor(1);
Adafruit_DCMotor *NestMotorLeft = AFMS.getMotor(2);
Adafruit_DCMotor *NestMotorRight = AFMS.getMotor(3);


//Other Sketch globals
//Door Related
bool DoorMode = true; //True is Timer, False is Sunset/Sundown 
DateTime TimeDoorOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeDoorClose (2000, 1, 1, 20, 0, 0);
DateTime Sunrise (2000, 1, 1, 8, 0, 0);
DateTime Sunset (2000, 1, 1, 8, 0, 0);
TimeSpan SunsetOffset;
TimeSpan SunRiseOffset;
int DoorRunningTime=10000; //This is the time (ms) it takes for the door to open or close, I.e the timer after the door stopped function is called
bool DoorRunningOpen=false;
bool DoorRunningClose=false;
Timer<1, millis, const char *> DoorHandlerTimer;


//Nest Related
DateTime TimeNestOpen (2000, 1, 1, 8, 0, 0);
DateTime TimeNestClose (2000, 1, 1, 20, 0, 0);
int NestRunningTime=10000; //This is the time (ms) it takes for the door to open or close, I.e the timer after the door stopped function is called
bool NestRunningOpen=false; // Could be declared as static
bool NestRunningClose=false; // Could be declared as static
Timer<1, millis, const char *> NestHandlerTimer;

//Idle Timer
elapsedMillis IdleTimeElapsed;
elapsedMillis IdleScrollTimeElapsed;
#define IDLEWAITTIME 6000 // milliseconds

//Eeprom
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


// ********************************************
// setup
// ********************************************

uint8_t i=0;
void setup()
{
  // LCD
  lcd.begin(16, 2);
  lcd.print(F("Hello, Eggmakers"));
  lcd.noAutoscroll();
  
  delay(1000);

  Serial.begin(9600);
  Serial.println("===========================");
  Serial.println(F("mblib - example for CMBMenu"));
  Serial.println("===========================");
  Serial.println("");
  Serial.println(F("l: left, r: right, e: enter, x: exit, m: print menu"));
  Serial.println("");

  // ** menu **
  // add nodes to menu (layer, string, function ID)
  g_Menu.addNode(0, g_MenuDoor, MenuDoor);
  g_Menu.addNode(1, g_MenuDoorSettings, MenuDoorSettings);
  g_Menu.addNode(2, g_MenuDoorMode, MenuDoorMode);
  g_Menu.addNode(3, g_MenuDoorModeSun, MenuDoorModeSun);
  g_Menu.addNode(3, g_MenuDoorModeTimer, MenuDoorModeTimer);
  g_Menu.addNode(2, g_MenuDoorSetTimer, MenuDoorSetTimer);
  g_Menu.addNode(1, g_MenuDoorForceOpen, MenuDoorForceOpen);
  g_Menu.addNode(1, g_MenuDoorForceClose, MenuDoorForceClose);
  g_Menu.addNode(0, g_MenuNest, MenuNest);
  g_Menu.addNode(1, g_MenuNestForceOpen, MenuNestForceOpen);
  g_Menu.addNode(1, g_MenuNestForceClose, MenuNestForceClose);
  g_Menu.addNode(0, g_MenuBattery, MenuBattery);
  g_Menu.addNode(0, g_MenuDisplayTime, MenuDisplayTime);


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
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
   if (! rtc.initialized() || rtc.lostPower()) {
    Serial.println("RTC is NOT initialized, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   }
   rtc.start();

  //MotorShield
  AFMS.begin();
    // Set the speed to start, from 0 (off) to 255 (max speed)
  DoorMotor->setSpeed(150);
  DoorMotor->run(FORWARD);
  DoorMotor->run(RELEASE);
  NestMotorLeft->setSpeed(150);
  NestMotorLeft->run(FORWARD);
  NestMotorLeft->run(RELEASE);
  NestMotorRight->setSpeed(150);
  NestMotorRight->run(FORWARD);
  NestMotorRight->run(RELEASE);
  
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

  //Run Door Handler
  RunDoorHandler(now);
  
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
      case MenuDoorForceOpen:
        OpenDoor();
        break;
      case MenuDoorForceClose:
        CloseDoor();
        break;
      case MenuDummy:
        Test1();
        break;
      case MenuDisplayTime:
        DisplayDateTime(now);
        break;
      case MenuDoorSetTimer:
        SetDoorTimer(); //Also set values to eeprom here
        break;
      case MenuNestForceOpen:
        OpenNest();
        break;
      case MenuNestForceClose:
        CloseNest();
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
  Serial.println("----------------");
  Serial.println(info_s);
  Serial.println("----------------");
  
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
    DoorMotor->setSpeed(200);
    DoorMotor->run(FORWARD);
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
    DoorRunningClose=true;
    DoorMotor->run(BACKWARD);
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
  delay(1000);
  DoorMotor->run(RELEASE);
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
    NestRunningOpen=true;
    NestMotorLeft->setSpeed(200);
    NestMotorLeft->run(FORWARD);
    NestMotorRight->setSpeed(200);
    NestMotorRight->run(FORWARD);
  }
}

// ********************************************
// CloseNest
// ********************************************
void CloseNest()
{
  if(NestRunningClose==false){
    NestHandlerTimer.cancel();
    NestHandlerTimer.in(DoorRunningTime, StopDoor);
    lcd.clear();
    lcd.print(F("Closing Nest"));
    NestRunningClose=true;
    NestMotorLeft->run(BACKWARD);
    NestMotorRight->run(BACKWARD);

  }  
}

// ********************************************
// StopNest
// ********************************************
bool StopNest()  //This function is only used by the DoorHandlerTimer
{
  lcd.clear();
  lcd.print("F(Stopping Nest)");
  NestRunningOpen=false;
  NestRunningClose=false;
  delay(1000);
  NestMotorLeft->run(RELEASE);
  NestMotorRight->run(RELEASE);
  return false;
   
}
// ********************************************
// RunDoorHandler
// ********************************************
void RunDoorHandler(DateTime now) // This function checks time. If any conditions are true (on the Second) these actions will be executed. Door is automatically stopped after int DoorRunningTime
{
  DoorHandlerTimer.tick();
  if(now.hour()==TimeDoorOpen.hour() && now.minute()==TimeDoorOpen.minute() &&now.second()==0){    
    OpenDoor();    
  }
  else if(now.hour()==TimeDoorClose.hour() && now.minute()==TimeDoorClose.minute() && now.second()==0){
    CloseDoor();
    }
}

// ********************************************
// RunNestHandler
// ********************************************
void RunNestHandler(DateTime now) //This function checks time. If any conditions are true (on the Second) these actions will be executed. Door is automatically stopped after int NestRunningTime
{
  NestHandlerTimer.tick();
  if(now.hour()==TimeNestOpen.hour() && now.minute()==TimeNestOpen.minute() &&now.second()==0){    
    OpenNest();    
  }
  else if(now.hour()==TimeNestClose.hour() && now.minute()==TimeNestClose.minute() && now.second()==0){
    CloseNest();
    }
}



// ********************************************
// FooA
// ********************************************
void FooA()
{
  Serial.println("Function FooA() was called.");

}

// ********************************************
// Test1
// ********************************************
void Test1()
{
  Serial.println("Function Test1() was called.");

}

// ********************************************
// Test2
// ********************************************
void Test2()
{
  Serial.println("Function Test2() was called.");

}

// ********************************************
// BarA
// ********************************************
void BarA()
{
  Serial.println("Function BarA() was called.");
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
// SetTimerHour.  
// ********************************************

DateTime SetTimerHour(DateTime time, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Hour");
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
  lcd.print("Set" +mystring +" Min");
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
   EEpromValues mystruct ={TimeDoorOpen, TimeDoorClose, TimeNestOpen, TimeNestClose, SunsetOffset, SunRiseOffset, DoorMode, DoorRunningTime, NestRunningTime};
   EEPROM.put(0, mystruct);
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
  SunsetOffset=mystruct.SunsetOffset;
  SunRiseOffset=mystruct.SunRiseOffset;
  DoorMode=mystruct.DoorMode;
  DoorRunningTime=mystruct.DoorRunningTime;
  NestRunningTime=mystruct.NestRunningTime;
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
    DisplayBattery();
    break;
    case 3:
    DisplayDoorMode();
    break;
    case 4:
    DisplayTimeGeneral(TimeDoorOpen, F("DoorTimer Open"));
    break;
    case 5:
    DisplayTimeGeneral(TimeDoorClose, F("DoorTimer Close"));
    break;
    default:
    InfoIndex=0;
    break;
  }
  InfoIndex++;

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
