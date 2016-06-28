
////CC3000 WiFi card Imports
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include "utility/sntp.h"

////2.8" TFT Breakout Imports
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <Streaming.h>
#define textSize 3

////BMP085 - Barometer, Altitude and Pressure imports
#include <Wire.h>
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

/******************START CC3000 WiFi Set-up******************/
// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  4
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIV2); // you can change this clock speed but DI

#define WLAN_SSID       "KDG-BD0D0"   // cannot be longer than 32 characters!
#define WLAN_PASS       "7e5rCtCvNyHW"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

//Arguments to SNTP client constructor:
//  1 - Primary Network Time Server URL (can be NULL)
//  2 - Secondary Network Time Server URL (also can be NULL)
//  3 - Local UTC offset in minutes (US Eastern Time is UTC - 5:00
//  4 - Local UTC offset in minutes for Daylight Savings Time (US Eastern DST is UTC - 4:00
//  5 - Enable Daylight Savings Time adjustment (not implemented yet)
//
sntp mysntp = sntp(NULL, "nist1.symmetricom.com", (short)(-5 * 60), (short)(-4 * 60), true);

// Type SNTP_Timestamp is 64-bit NTP time. High-order 32-bits is seconds since 1/1/1900
//   Low order 32-bits is fractional seconds
SNTP_Timestamp_t now;

// Type NetTime_t contains NTP time broken out to human-oriented values:
//  uint16_t millis; ///< Milliseconds after the second (0..999)
//  uint8_t  sec;    ///< Seconds after the minute (0..59)
//  uint8_t  min;    ///< Minutes after the hour (0..59)
//  uint8_t  hour;   ///< Hours since midnight (0..23)
//  uint8_t  mday;   ///< Day of the month (1..31)
//  uint8_t  mon;    ///< Months since January (0..11)
//  uint16_t year;   ///< Year.
//  uint8_t  wday;   ///< Days since Sunday (0..6)
//  uint8_t  yday;   ///< Days since January 1 (0..365)
//  bool   isdst;  ///< Daylight savings time flag, currently not supported 
NetTime_t timeExtract;

#define pF(string_pointer) (reinterpret_cast<const __FlashStringHelper *>(pgm_read_word(string_pointer)))

const prog_char   janStr[] PROGMEM = "January";
const prog_char   febStr[] PROGMEM = "February";
const prog_char   marStr[] PROGMEM = "March";
const prog_char   aprStr[] PROGMEM = "April";
const prog_char   mayStr[] PROGMEM = "May";
const prog_char   junStr[] PROGMEM = "June";
const prog_char   julStr[] PROGMEM = "July";
const prog_char   augStr[] PROGMEM = "August";
const prog_char   sepStr[] PROGMEM = "September";
const prog_char   octStr[] PROGMEM = "October";
const prog_char   novStr[] PROGMEM = "November";
const prog_char   decStr[] PROGMEM = "December"; 

PROGMEM const char* const monthStrs[] = { janStr, febStr, marStr, aprStr, mayStr, junStr,
                                          julStr, augStr, sepStr, octStr, novStr, decStr}; 

const prog_char   sunStr[] PROGMEM = "Sunday";
const prog_char   monStr[] PROGMEM = "Monday";
const prog_char   tueStr[] PROGMEM = "Tuesday";
const prog_char   wedStr[] PROGMEM = "Wednesday";
const prog_char   thuStr[] PROGMEM = "Thursday";
const prog_char   friStr[] PROGMEM = "Friday";
const prog_char   satStr[] PROGMEM = "Saturday"; 

PROGMEM const char* const dayStrs[] = { sunStr, monStr, tueStr,  wedStr,
                                        thuStr, friStr, satStr};
                                        
/******************END WiFi Set-up******************/

/****************** START 2.8" TFT Breakout Set-up******************/

// The control pins for the LCD can be assigned to any digital or
// analog pins...but we'll use the analog pins as this allows us to
// double up the pins with the touch screen (see the TFT paint example).
#ifdef __MK20DX128__   /*teensy3 */
#define LCD_CS A9 // Chip Select goes to Analog 9
#define LCD_CD A3 // Command/Data goes to Analog 5
#define LCD_WR A2 // LCD Write goes to Analog 2
#define LCD_RD A1 // LCD Read goes to Analog 1
#define LCD_RESET A8 // this can move to the reset pin but this is more breadboard friendly
#else
#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0--A8 on teensy3,14 is A0 and is on the port we use
#define LCD_RESET A4 // Can alternately just connect to Arduino's reset pin
#endif

// Assign human-readable names to some common 16-bit color values:
// definitions moved to AdafruitTFTLCD.h
#define BLACK   0x0000
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

/****************** END 2.8" TFT Breakout Set-up******************/

void setup(void) {
 // Serial.begin(115200);
  /////////TFT LCD Debugging
  progmemPrintln(PSTR("TFT LCD test"));

#ifdef USE_ADAFRUIT_SHIELD_PINOUT
  progmemPrintln(PSTR("Using Adafruit 2.8\" TFT Arduino Shield Pinout"));
#else
  progmemPrintln(PSTR("Using Adafruit 2.8\" TFT Breakout Board Pinout"));
#endif

  tft.reset();

  uint16_t identifier = tft.readID();

  if(identifier == 0x9325) {
    progmemPrintln(PSTR("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    progmemPrintln(PSTR("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    progmemPrintln(PSTR("Found HX8347G LCD driver"));
  } else {
    progmemPrint(PSTR("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    progmemPrintln(PSTR("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    progmemPrintln(PSTR("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    progmemPrintln(PSTR("should appear in the library header (Adafruit_TFT.h)."));
    progmemPrintln(PSTR("If using the breakout board, it should NOT be #defined!"));
    progmemPrintln(PSTR("Also if using the breakout, double-check that all wiring"));
    progmemPrintln(PSTR("matches the tutorial."));
    return;
  }
  tft.begin(identifier);
  tft.setRotation(2);
  tft.fillScreen(BLACK);
  
  tft(0,0,TFTLCD_GREEN,1)<<"TFT LCD: Connected";
 
  /////////CC3000 Debugging
  Serial.println(F("Hello, CC3000!\n")); 
  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  tft(0,10,TFTLCD_GREEN,1)<<"Free RAM: ";
  tft(60,10,TFTLCD_GREEN,1)<<getFreeRam(), DEC;
  
  /* Initialise the module */
  Serial.println(F("\nInitialising the CC3000 ..."));
    tft(0,20,TFTLCD_GREEN,1)<<"Initialising the CC3000...";
  if (!cc3000.begin())
  {
    Serial.println(F("Unable to initialise the CC3000! Check your wiring?"));
    tft(0,30,TFTLCD_GREEN,1)<<"Unable to initialise the CC3000!";
    while(1);
  }

  /* Optional: Update the Mac Address to a known value */
/*
  uint8_t macAddress[6] = { 0x08, 0x00, 0x28, 0x01, 0x79, 0xB7 };
   if (!cc3000.setMacAddress(macAddress))
   {
     Serial.println(F("Failed trying to update the MAC address"));
     while(1);
   }
*/
  
  uint16_t firmware = checkFirmwareVersion();
  if ((firmware != 0x113) && (firmware != 0x118)) {
    Serial.println(F("Wrong firmware version!"));
    tft(0,30,TFTLCD_GREEN,1)<<"Wrong firmware version!";
    for(;;);
  }
  else{
    tft(0,30,TFTLCD_GREEN,1)<<"Firmware Version: ";
    tft(105,30,TFTLCD_GREEN,1)<<firmware;
  }
  
  /* Delete any old connection data on the module */
  Serial.println(F("\nDeleting old connection profiles"));
  tft(0,40,TFTLCD_GREEN,1)<<"Deleting old connection profiles";
  if (!cc3000.deleteProfiles()) {
    Serial.println(F("Failed!"));
    tft(0,50,TFTLCD_GREEN,1)<<"Failed!";
    while(1);
  }
  else{
    tft(0,50,TFTLCD_GREEN,1)<<"Success";
  }

  /* Attempt to connect to an access point */
  char *ssid = WLAN_SSID;             /* Max 32 chars */
  Serial.print(F("\nAttempting to connect to ")); Serial.println(ssid);
  tft(0,60,TFTLCD_GREEN,1)<<"Connecting to: ";
  tft(90,60,TFTLCD_GREEN,1)<<ssid;
  
  
  /* NOTE: Secure connections are not available in 'Tiny' mode! */
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    tft(0,70,TFTLCD_GREEN,1)<<"Failed!";
    while(1);
  }
   
  Serial.println(F("Connected!"));
  tft(0,70,TFTLCD_GREEN,1)<<"Success";
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  tft(0,80,TFTLCD_GREEN,1)<<"Request DHCP";
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  
  tft(0,90,TFTLCD_GREEN,1)<<"Success";

  Serial.println(F("UpdateNTPTime"));
  tft(0,100,TFTLCD_GREEN,1)<<"UpdateNTPTime";
  mysntp.UpdateNTPTime();
  tft(0,110,TFTLCD_GREEN,1)<<"Success";
  Serial.println(F("Current local time is:"));
  mysntp.ExtractNTPTime(mysntp.NTPGetTime(&now, true), &timeExtract);

  Serial.print(timeExtract.hour); Serial.print(F(":")); Serial.print(timeExtract.min); Serial.print(F(":"));Serial.print(timeExtract.sec); Serial.print(F("."));Serial.println(timeExtract.millis);
  Serial.print(pF(&dayStrs[timeExtract.wday])); Serial.print(F(", ")); Serial.print(pF(&monthStrs[timeExtract.mon])); Serial.print(F(" ")); Serial.print(timeExtract.mday); Serial.print(F(", "));Serial.println(timeExtract.year);
  Serial.print(F("Day of year: ")); Serial.println(timeExtract.yday + 1); 

  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time you try to connect ... */
  Serial.println(F("\n\nClosing the connection"));
  tft(0,120,TFTLCD_GREEN,1)<<"Closing Connection";
  cc3000.disconnect();

/////////BMP085 Debugging
  tft(0,130,TFTLCD_GREEN,1)<<"Initialising the BMP085 Sensors...";
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    tft(0,140,TFTLCD_GREEN,1)<<"Could not find a valid BMP085 sensor, check wiring!";
    while (1) {}
  }
  else{
      tft(0,140,TFTLCD_GREEN,1)<<"Success";
  }
  tft(0,150,TFTLCD_GREEN,1)<<"Loading Program";
  delay(10000);
  tft.fillScreen(BLACK);
  delay(10);
  //////Create Static units to be displayed on screeen
  tft(50 + (textSize * 12),30,TFTLCD_BLUE,textSize)<<"C";
  tft(120 + (textSize * 12),30,TFTLCD_BLUE,textSize)<<"F";
  tft(100 + (textSize * 12),60,TFTLCD_BLUE,textSize)<<"Pa";
  tft(50 + (textSize * 12),90,TFTLCD_BLUE,textSize)<<"m";
  tft(140 + (textSize * 12),90,TFTLCD_BLUE,textSize)<<"Ft";
}

long previousMillis = 0;
long interval = 1000;

void loop(void) {
  
  
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) {
    previousMillis = currentMillis;
  showTime();  
  showTemp();
  showPressure();
  showAltitude();
  }
}

int currentHour;
int previousHour;
int currentMin;
int previousMin;
int previousSec;
int currentSec;
void showTime() {
  mysntp.ExtractNTPTime(mysntp.NTPGetTime(&now, true), &timeExtract);
  Serial.print(timeExtract.hour); Serial.print(F(":")); Serial.print(timeExtract.min); Serial.print(F(":"));Serial.print(timeExtract.sec); Serial.print(F("."));Serial.println(timeExtract.millis);
  Serial.print(pF(&dayStrs[timeExtract.wday])); Serial.print(F(", ")); Serial.print(pF(&monthStrs[timeExtract.mon])); Serial.print(F(" ")); Serial.print(timeExtract.mday); Serial.print(F(", "));Serial.println(timeExtract.year);
  Serial.print(F("Day of year: ")); Serial.println(timeExtract.yday + 1); 
  currentHour = timeExtract.hour;
  if (currentHour >= 0 && currentHour <= 3){ ///Adjusting for PST -4
    currentHour = currentHour + 20;
  }
  else {
    currentHour = currentHour - 4;
  }
  currentMin = timeExtract.min;
  currentSec = timeExtract.sec;
  if(currentHour != previousHour){
    tft(30,120,TFTLCD_BLACK,textSize)<<previousHour; // display new altitude
    tft(30,120,TFTLCD_BLUE,textSize)<<currentHour; // display new altitude
    previousHour = currentHour;
  }
  if(currentMin != previousMin){
    tft(70,120,TFTLCD_BLACK,textSize)<<previousMin; // display new altitude
    tft(70,120,TFTLCD_BLUE,textSize)<<currentMin; // display new altitude
    previousMin = currentMin;
  }
//  if(currentSec < 10)
//  {
//    tft(120,110,TFTLCD_BLUE,textSize)<<"0";
//  }
  tft(120,120,TFTLCD_BLACK,textSize)<<previousSec; // display new altitude
  tft(120,120,TFTLCD_BLUE,textSize)<<currentSec; // display new altitude
  previousSec = currentSec;
}


int currentBMPtempc;
int previousBMPtempc;
int currentBMPtempf;
int previousBMPtempf;
int BMPpressure;
void showTemp(){
  currentBMPtempc = bmp.readTemperature();
  if (currentBMPtempc == previousBMPtempc) {
    return;
  }
  //convert C temp to F
  currentBMPtempf = currentBMPtempc * (9/5) + 32;
  //display temps 
  tft(50,30,TFTLCD_BLACK,textSize)<<previousBMPtempc; // erase previous C temp
  tft(120,30,TFTLCD_BLACK,textSize)<<previousBMPtempf; //erase previous F temp
  tft(50,30,TFTLCD_BLUE,textSize)<<currentBMPtempc; //display current C temp
  tft(120,30,TFTLCD_BLUE,textSize)<<currentBMPtempf; // display current F temp
  previousBMPtempc = currentBMPtempc;  
  previousBMPtempf = currentBMPtempf;
}


int currentBMPpress;
int previousBMPpress;
void showPressure(){
  currentBMPpress = bmp.readPressure();
  currentBMPpress = currentBMPpress / 100;
  if (currentBMPpress == previousBMPpress) {
    return;
  }
  tft(50,60,TFTLCD_BLACK,textSize)<<previousBMPpress; // erase previous pressure
  tft(50,60,TFTLCD_BLUE,textSize)<<currentBMPpress; // display new pressure
  tft(50,60,TFTLCD_BLUE,textSize)<<currentBMPpress;
  previousBMPpress = currentBMPpress;
}

int currentBMPaltitudem; //variable for altitude Meter
int previousBMPaltitudem;
int currentBMPaltitudeft; //variable for altitude Feet
int previousBMPaltitudeft;
void showAltitude() {
  currentBMPaltitudem = bmp.readAltitude(101810);
  if (currentBMPaltitudem == previousBMPaltitudem) {
    return;
  }
  currentBMPaltitudeft = currentBMPaltitudem * 3.3;
  tft(50,90,TFTLCD_BLACK,textSize)<<previousBMPaltitudem; // erase previous altitude meeters
  tft(120,90,TFTLCD_BLACK,textSize)<<previousBMPaltitudeft; // erase previous altitude feet
  tft(50,90,TFTLCD_BLUE,textSize)<<currentBMPaltitudem; // display new altitude
  tft(120,90,TFTLCD_BLUE,textSize)<<currentBMPaltitudeft; // display new altitude
  previousBMPaltitudem = currentBMPaltitudem;
  previousBMPaltitudeft = currentBMPaltitudeft;
}

// Copy string from flash to serial port
// Source string MUST be inside a PSTR() declaration!
void progmemPrint(const char *str) {
  char c;
  while((c = pgm_read_byte(str++))) Serial.print(c);
}

// Same as above, with trailing newline
void progmemPrintln(const char *str) {
  progmemPrint(str);
  Serial.println();
}

/**************************************************************************/
/*!
    @brief  Tries to read the CC3000's internal firmware patch ID
*/
/**************************************************************************/
uint16_t checkFirmwareVersion(void)
{
  uint8_t major, minor;
  uint16_t version;
  
#ifndef CC3000_TINY_DRIVER  
  if(!cc3000.getFirmwareVersion(&major, &minor))
  {
    Serial.println(F("Unable to retrieve the firmware version!\r\n"));
    version = 0;
  }
  else
  {
    Serial.print(F("Firmware V. : "));
    Serial.print(major); Serial.print(F(".")); Serial.println(minor);
    version = major; version <<= 8; version |= minor;
  }
#endif
  return version;
}

