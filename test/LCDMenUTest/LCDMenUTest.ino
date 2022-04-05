


// ** Dependencies **
// include CMBMenu
#include "CMBMenu.hpp"
/*
 * mblib - MB library
 * https://github.com/mchlbrnhrd/mbLib
 */

// include the Adafruit LCD library code:
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>


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
const char g_MenuDoorMode[] PROGMEM = {"1.1 Door Mode"};
const char g_MenuDoorModeSun[] PROGMEM = {"1.1.1 Sunset"};
const char g_MenuDoorModeTimer[] PROGMEM = {"1.1.2 Timer"};
const char g_MenuDoorForceOpen[] PROGMEM = {"1.2.1 Open Door"};
const char g_MenuDoorForceClose[] PROGMEM = {"1.2.2 Close Door"};
const char g_MenuNest[] PROGMEM = {"2. Nest"};
const char g_MenuNestForceOpen[] PROGMEM = {"2.1 Open Nest"};
const char g_MenuNestForceClose[] PROGMEM = {"2.1 Close Nest"};

// define function IDs
enum MenuFID {
    MenuDummy,
    MenuDoor,
    MenuDoorMode,
    MenuDoorModeSun,
    MenuDoorModeTimer,
    MenuDoorForceOpen,
    MenuDoorForceClose,
    MenuNest,
    MenuNestForceOpen,
    MenuNestForceClose
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


// create global LCD instance
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

// create global CMBMenu instance
// (here for maximum 100 menu entries)
CMBMenu<100> g_Menu;


// ********************************************
// setup
// ********************************************

uint8_t i=0;
void setup()
{
  // LCD
  lcd.begin(16, 2);
  lcd.print("Hello, eggmakers");
  
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
  g_Menu.addNode(1, g_MenuDoorMode, MenuDoorMode);
  g_Menu.addNode(2, g_MenuDoorModeSun, MenuDoorModeSun);
  g_Menu.addNode(2, g_MenuDoorModeTimer, MenuDoorModeTimer);
  g_Menu.addNode(1, g_MenuDoorForceOpen, MenuDoorForceOpen);
  g_Menu.addNode(1, g_MenuDoorForceClose, MenuDoorForceClose);
  g_Menu.addNode(0, g_MenuNest, MenuNest);
  g_Menu.addNode(1, g_MenuNestForceOpen, MenuNestForceOpen);
  g_Menu.addNode(1, g_MenuNestForceClose, MenuNestForceClose);


  // ** menu **
  // build menu and print menu
  // (see terminal for output)
  const char* info;
  g_Menu.buildMenu(info);
  g_Menu.printMenu();

  // ** menu **
  // print current menu entry
   printMenuEntry(info);
}


// ********************************************
// loop
// ********************************************

void loop()
{
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
        lcd.clear();
        lcd.print("opening door");
        break;
      case MenuDoorForceClose:
        lcd.clear();
        lcd.print("closing door");
        break;
      case MenuDummy:
        Test1();
        break;
      default:
        break;
    }
  }
}

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
      lcd.setBacklight(RED);
      key = KeyUp;
    }
    if (buttons & BUTTON_DOWN) {
      lcd.print("DOWN ");
      lcd.setBacklight(YELLOW);
      key = KeyDown;
    }
    if (buttons & BUTTON_LEFT) {
      lcd.print("LEFT ");
      lcd.setBacklight(GREEN);
      key = KeyLeft;
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.print("RIGHT ");
      lcd.setBacklight(TEAL);
      key = KeyRight;
    }
    if (buttons & BUTTON_SELECT) {
      lcd.print("SELECT ");
      lcd.setBacklight(VIOLET);
      key = KeySelect;
    }
  }
    
  

 return key;
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
