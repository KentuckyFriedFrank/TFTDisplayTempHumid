
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include "TouchScreen.h"

/* For the 8 data pins:
Duemilanove/Diecimila/UNO/etc ('168 and '328 chips) microcontoller:
D0 connects to digital 8
D1 connects to digital 9
D2 connects to digital 2
D3 connects to digital 3
D4 connects to digital 4
D5 connects to digital 5
D6 connects to digital 6
D7 connects to digital 7

For Mega's use pins 22 thru 29 (on the double header at the end)
*/

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 309);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0 
// optional
#define LCD_RESET A4

#ifdef __MK20DX128__   /*teensy3 */

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 5   // can be a digital pin
#define XP 6   // can be a digital pin
#define LCD_CS 12 // Chip Select goes to Analog 3
#define LCD_CD 11 // Command/Data goes to Analog 2
#define LCD_WR 10 // LCD Write goes to Analog 1
#define LCD_RD 9 // LCD Read goes to Analog 0--A8 on teensy3,14 is A0 and is on the port we use

#define LCD_RESET 13 // Can alternately just connect to Arduino's reset pin
#endif
// Color definitions
#define	BLACK           0x0000
#define	BLUE            0x001F
#define	RED             0xF800
#define	GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0 
#define WHITE           0xFFFF
#define ORANGE          0xFC80


Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXSIZE 30
#define PENRADIUS 1
int oldcolor, currentcolor;

void setup(void) {
  Serial.begin(9600);
  Serial.println("Paint!");
  
  tft.reset();
  
  uint16_t identifier = tft.readRegister(0x0);
  if (identifier == 0x9325) {
    Serial.println("Found ILI9325");
  } else if (identifier == 0x9328) {
    Serial.println("Found ILI9328");
  } else {
    Serial.print("Unknown driver chip ");
    Serial.println(identifier, HEX);
    while (1);
  }

  tft.initDisplay(); 
  tft.fillScreen(BLACK);
  
  tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
  tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
  tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, GREEN);
  tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, CYAN);
  tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, BLUE);
  tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, MAGENTA);
  tft.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, WHITE);
  tft.fillRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, ORANGE);
 
 tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
 currentcolor = RED;
 
  pinMode(13, OUTPUT);
}



#define MINPRESSURE 3
#define MAXPRESSURE 1000



void loop()
{
  digitalWriteFast(13, HIGH);
  Point p = ts.getPoint();
  digitalWriteFast(13, LOW);

  // if you're sharing pins, you'll need to fix the directions of the touchscreen pins!
  pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  pinMode(YM, OUTPUT);

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    /*
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    */
    
    if (p.y < (TS_MINY-5)) {
      Serial.println("erase");
     // press the bottom of the screen to erase 
      tft.fillRect(0, BOXSIZE, tft.width(), tft.height()-BOXSIZE, BLACK);
      
    }
    // turn from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    /*
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.println(")");
    */
    if (p.y < BOXSIZE) {
       oldcolor = currentcolor;
 
  
       if (p.x < BOXSIZE) { 
         currentcolor = RED; 
         tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
       } 
       else if (p.x < BOXSIZE*2) {
         currentcolor = YELLOW; 
         tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, BLACK);
       }
       else if (p.x < BOXSIZE*3) {
         currentcolor = GREEN; 
         tft.drawRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, WHITE);
       }
       else if (p.x < BOXSIZE*4) {
         currentcolor = CYAN; 
         tft.drawRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, BLACK);
       }
       else if (p.x < BOXSIZE*5) {
         currentcolor = BLUE; 
         tft.drawRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, WHITE);
       }
       else if (p.x < BOXSIZE*6) {
         currentcolor = MAGENTA; 
         tft.drawRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, WHITE);
       }
        else if (p.x < BOXSIZE*7) {
         currentcolor = WHITE; 
         tft.drawRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, BLACK);
       }
        else if (p.x < BOXSIZE*8) {
         currentcolor = ORANGE; 
         tft.drawRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, WHITE);
       }
       
       if (oldcolor != currentcolor) {
          if (oldcolor == RED) tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
          if (oldcolor == YELLOW) tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
          if (oldcolor == GREEN) tft.fillRect(BOXSIZE*2, 0, BOXSIZE, BOXSIZE, GREEN);
          if (oldcolor == CYAN) tft.fillRect(BOXSIZE*3, 0, BOXSIZE, BOXSIZE, CYAN);
          if (oldcolor == BLUE) tft.fillRect(BOXSIZE*4, 0, BOXSIZE, BOXSIZE, BLUE);
          if (oldcolor == MAGENTA) tft.fillRect(BOXSIZE*5, 0, BOXSIZE, BOXSIZE, MAGENTA);
          if (oldcolor == WHITE) tft.fillRect(BOXSIZE*6, 0, BOXSIZE, BOXSIZE, WHITE);
          if (oldcolor == ORANGE) tft.fillRect(BOXSIZE*7, 0, BOXSIZE, BOXSIZE, ORANGE);
       }
    }
    if (((p.y-PENRADIUS) > BOXSIZE) && ((p.y+PENRADIUS) < tft.height())) {
      tft.fillCircle(p.x, p.y, PENRADIUS, currentcolor);
    }
  }
}
