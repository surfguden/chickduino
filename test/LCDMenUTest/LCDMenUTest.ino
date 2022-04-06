//TBD NestHandler, Battery, SunsetSundown, EEPROM, MotorShield


// ** Dependencies **
// include CMBMenu
#include "CMBMenu.hpp"
#include "RTClib.h"
#include <Adafruit_MotorShield.h>
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
const char g_MenuNestForceClose[] PROGMEM = {"2.1 Close Nest"};
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

//Other Sketch globals
//Door Related
bool DoorMode = true; //True is Timer, False is Sunset/Sundown 
int DoorTimerOpenHour = 11;
int DoorTimerCloseHour = 0;
int DoorTimerOpenMinute = 52;
int DoorTimerCloseMinute = 35;
int DoorRunningTime=10000; //This is the time (ms) it takes for the door to open or close, I.e the timer after the door stopped function is called
bool DoorRunningOpen=false;
bool DoorRunningClose=false;
Timer<1, millis, const char *> DoorHandlerTimer;
//Nest Related
int NestTimerOpenHour = 8;
int NestTimerCloseHour = 20;
int NestTimerOpenMinute = 00;
int NestTimerCloseMinute = 00;
int NestRunningTime=10000; //This is the time (ms) it takes for the door to open or close, I.e the timer after the door stopped function is called
bool NestRunningOpen=false;
bool NestRunningClose=false;



// ********************************************
// setup
// ********************************************

uint8_t i=0;
void setup()
{
  // LCD
  lcd.begin(16, 2);
  lcd.print("Hello, eggmakers");
  lcd.noAutoscroll();
  
  delay(1000);

  Serial.begin(9600);
  Serial.println("===========================");
  Serial.println("mblib - example for CMBMenu");
  Serial.println("===========================");
  Serial.println("");
  Serial.println("l: left, r: right, e: enter, x: exit, m: print menu");
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

  // ** menu **
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
  // turn on motor
  DoorMotor->run(RELEASE);
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

  // determine pressed key
  KeyType key = getKey();



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
        DisplayTime(now);
        break;
      case MenuDoorSetTimer:
        SetDoorTimer(); //Also set values to eeprom here
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
  // here for demonstration: get "pressed" key from terminal
  // replace code when using push buttons
 /* 
 while(Serial.available() > 0) {
    String Key_s = Serial.readString();
    Key_s.trim();
    Serial.println("");
    if(Key_s.equals("l")) { // left
      key = KeyLeft;
      Serial.println("<left>");
    } else if (Key_s.equals("r")) { // right
      key = KeyRight;
      Serial.println("<right>");
    } else if (Key_s.equals("e")) { // enter
      key = KeyEnter;
      Serial.println("<enter>");
    } else if (Key_s.equals("x")) { // exit
      key = KeyExit;
      Serial.println("<exit>");
    } else if (Key_s.equals("m")) { // print menu
      g_Menu.printMenu();
      }
  }
 */    
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      lcd.clear();
      lcd.setCursor(0,0);
      delay(150);
    if (buttons & BUTTON_UP) {
      lcd.print("UP ");      
      key = KeyUp;
    }
    if (buttons & BUTTON_DOWN) {
      lcd.print("DOWN ");     
      key = KeyDown;
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("LEFT ");
      key = KeyLeft;
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.print("RIGHT ");
      key = KeyRight;
    }
    if (buttons & BUTTON_SELECT) {
      lcd.print("SELECT ");
      key = KeySelect;
    }
  }
 return key;
}

// ********************************************
// DisplayTime
// ********************************************
void DisplayTime(DateTime now)
{
  lcd.clear();
  char buf2[] = "YYMMDD-hh:mm:ss";
  lcd.print(now.toString(buf2));
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
    lcd.print("Opening Door");
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
    lcd.print("Closing Door");
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
  lcd.print("Stopping Door");
  DoorRunningOpen=false;
  DoorRunningClose=false;
  delay(1000);
  DoorMotor->run(RELEASE);
  return false;
   
}
// ********************************************
// RunDoorHandler
// ********************************************
void RunDoorHandler(DateTime now) //TBD Add which motor here as input argument. First This function checks time. If any conditions are true (on the second) these actions will be executed. Door is automatically stopped after int DoorRunningTime
{
  DoorHandlerTimer.tick();
  if(now.hour()==DoorTimerOpenHour && now.minute()==DoorTimerOpenMinute &&now.second()==0){    
    OpenDoor();    
  }
  else if(now.hour()==DoorTimerCloseHour && now.minute()==DoorTimerCloseMinute && now.second()==0){
    CloseDoor();
    }
}

//int DoorTimerOpenHour = 0;
//int DoorTimerCloseHour = 0;


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
  DoorTimerOpenHour=SetTimerHour(DoorTimerOpenHour,DoorTimerOpenMinute, "door open");
  DoorTimerOpenMinute=SetTimerMinute(DoorTimerOpenHour,DoorTimerOpenMinute, "door open");
  DoorTimerCloseHour=SetTimerHour(DoorTimerCloseHour,DoorTimerCloseMinute,"door close");
  DoorTimerCloseMinute=SetTimerMinute(DoorTimerCloseHour,DoorTimerCloseMinute, "door close");
  lcd.setBacklight(TEAL);
  //Write Values To EEprom here
}

// ********************************************
// SetTimerHour.  
// ********************************************

int SetTimerHour(int hour,int minute, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Hr");
  lcd.setCursor(0,1);
  lcd.print(hour);
  lcd.print(":");
  lcd.print(minute);
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        hour=IncreaseHour(hour);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set" +mystring +" Hr");
        lcd.setCursor(0,1);
        lcd.print(hour);
        lcd.print(":");
        lcd.print(minute);       
      }
      if (buttons & BUTTON_DOWN) {
        hour=DecreaseHour(hour);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set" +mystring +"Hr");
        lcd.setCursor(0,1);
        lcd.print(hour);
        lcd.print(":");
        lcd.print(minute);   
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print("Value Set!");
        set=false;
      }
    }     
  }
  lcd.noBlink();
  return hour;
}

// ********************************************
// SetTimerMinute.  
// ********************************************

int SetTimerMinute(int hour, int minute, String mystring)
{
  lcd.blink();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Set" +mystring +" Min");
  lcd.setCursor(0,1);
  lcd.print(hour);
  lcd.print(":");
  lcd.print(minute);
  bool set=true;
  while(set){
    uint8_t buttons = lcd.readButtons();
    if (buttons) {
      delay(150);
      if (buttons & BUTTON_UP) {
        minute=IncreaseMinute(minute);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set" +mystring +" Min");
        lcd.setCursor(0,1);
        lcd.print(hour);
        lcd.print(":");
        lcd.print(minute);     
      }
      if (buttons & BUTTON_DOWN) {
        minute=DecreaseMinute(minute);
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Set" +mystring +" Min");
        lcd.setCursor(0,1);
        lcd.print(hour);
        lcd.print(":");
        lcd.print(minute); 
      }
      if (buttons & BUTTON_RIGHT) {
        lcd.clear();
        lcd.print("Value Set!");
        set=false;
        
      }
    }   
  }
  lcd.noBlink();
  return minute;
}
// ********************************************
// IncreaseHour
// ********************************************
int IncreaseHour(int hour)
{
  hour=hour+1;
  if(hour>23){
    hour=23;
    }
  return hour;
  }
// ********************************************
// DecreaseHour
// ********************************************  
int DecreaseHour(int hour)
{
  hour=hour-1;
  if(hour<0){
    hour=0;
    }
  return hour;
  }
// ********************************************
// IncreaseMinute
// ********************************************
int IncreaseMinute(int minute)
{
  minute=minute+1;
  if(minute>59){
    minute=59;
    }
  return minute;
  }
// ********************************************
// DecreaseMinute
// ********************************************  
int DecreaseMinute(int minute)
{
  minute=minute-1;
  if(minute<0){
    minute=0;
    }
  return minute;
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
