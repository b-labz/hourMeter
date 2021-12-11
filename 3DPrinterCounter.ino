//V 1.0. Created by B-Labz! 

#include "Wire.h"
#include "FRAM.h"

//Locations in memory: Address

#define LOC_SEC 0
#define LOC_MIN 4
#define LOC_HRL 8
#define LOC_HRM 12

unsigned int  prHr;
unsigned char prSec, prMin;
unsigned int  counter10Sec;
unsigned char clearCtr;

FRAM fram(0b00);
#define FRAM_PAGE 0

//Library for SSD1306
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//Pin where the Clear switch is connected
#define CLEAR_PIN 2

U8G2_SSD1306_128X32_UNIVISION_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);  // Adafruit ESP8266/32u4/ARM Boards + FeatherWing OLED

//#define RUNONCE //Activate this to clear timer once from there on its non resettable
#undef RUNONCE


//function gets the hour meter values from FRAM location
void getHM();

void setup() {
  // put your setup code here, to run once:
  u8g2.begin();
  u8g2.clearBuffer();                      // clear the internal memory
  u8g2.setFont(u8g2_font_crox2h_tf );      // choose a suitable font
  u8g2.drawStr(5,20,"B-Labz-Hour Meter!"); // write text to the internal memory
  u8g2.sendBuffer();                       // transfer internal memory to the display
  pinMode(LED_BUILTIN, OUTPUT);           
  pinMode(CLEAR_PIN, INPUT);               //Config switch pin

//If RUNONCE hash define is active it clears the values in memory location to zero
//Add val, bytes
#ifdef RUNONCE
  fram.WriteByte(FRAM_PAGE, LOC_SEC, 0);
  fram.WriteByte(FRAM_PAGE, LOC_MIN, 0);
  fram.WriteByte(FRAM_PAGE, LOC_HRL, 0);
  fram.WriteByte(FRAM_PAGE, LOC_HRM, 0);
#endif
  delay(2000);
  getHM();         //Get the value from FRAM
  printTime();     //Display on Screen
  
}


void loop() 
{
  // A simple clock implementation
  if(prSec<59)prSec++;
  else
  {
    prSec = 0;
    if(prMin<59)prMin++;
    else
    {
      prMin = 0;
      if(prHr<0xFFFF)prHr++;
      else prHr = 0;
    }
  }
  //write back the updated value into FRAM once every 1 sec and display the same
  if(counter10Sec<1)counter10Sec++;
  else
  {
    counter10Sec = 0;
    writeHM(prHr,prMin,prSec);//Write to FRAM
    printTime();//Display the time
  }

  //Toggle builtin LED ~one sec once
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(250); 
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(250); 
  //If the clear Switch is pressed and held for 3 seconds clear the values
  if(digitalRead(CLEAR_PIN) == 0)
  {
    clearCtr++;
  }
  else clearCtr = 0;

  //Clear after 3 second of continious press
  if(clearCtr == 3)
  {
    fram.WriteByte(FRAM_PAGE, LOC_SEC, 0);
    fram.WriteByte(FRAM_PAGE, LOC_MIN, 0);
    fram.WriteByte(FRAM_PAGE, LOC_HRL, 0);
    fram.WriteByte(FRAM_PAGE, LOC_HRM, 0);
    getHM();//Get the value after reset
  }
}


//This function writes the value into FRAM
void writeHM(unsigned int hmHour,unsigned char hmMin, unsigned char hmSec)
{
  fram.WriteByte(FRAM_PAGE, LOC_SEC, hmSec);
  fram.WriteByte(FRAM_PAGE, LOC_MIN, hmMin);
  fram.WriteByte(FRAM_PAGE, LOC_HRL, hmHour & 0xff);
  fram.WriteByte(FRAM_PAGE, LOC_HRM, (hmHour & 0xff00) >> 8);
}


//This function gets value from FRAM.
//Hour variable is a 16 bit 
void getHM()

{
  prSec = fram.ReadByte(FRAM_PAGE, LOC_SEC);
  prMin = fram.ReadByte(FRAM_PAGE, LOC_MIN);
  prHr  = ((fram.ReadByte(FRAM_PAGE, LOC_HRM))<<8 | (fram.ReadByte(FRAM_PAGE, LOC_HRL)));
}


//This functions builds string to move it to u8g2 on SSD1306
#define countof(a) (sizeof(a) / sizeof(a[0]))
void printTime()
{
  char hourmeterString[20];
  snprintf_P(hourmeterString,
        countof(hourmeterString),
        PSTR("%04u:%02u:%02u"),
        prHr,
        prMin,
        prSec );
  u8g2.clearBuffer();                  // clear the internal memory
  u8g2.setFont(u8g2_font_helvR18_tn ); // choose a suitable font
  u8g2.drawStr(5,25,hourmeterString); // write the constructed time value to internal memory
  u8g2.sendBuffer();                   // transfer internal memory to the display
}
