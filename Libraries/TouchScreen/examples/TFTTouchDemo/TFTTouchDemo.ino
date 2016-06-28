#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <Streaming.h>
#include <TouchScreen.h>

#define LCD_CS 12 // Chip Select goes to Analog 3
#define LCD_CD 11 // Command/Data goes to Analog 2
#define LCD_WR 10 // LCD Write goes to Analog 1
#define LCD_RD 9 // LCD Read goes to Analog 0--A8 on teensy3,14 is A0 and is on the port we use

#define XP A0   // can be a digital pin and can reuse pins
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 21   // can be a digital pin and can reuse pins

/* Color definitions  moved to Adafruit_TFTLCD.h
#define	TFTLCD_BLACK           0x0000
#define TFTLCD_WHITE           0xFFFF
#define	TFTLCD_BLUE            0x001F
#define	TFTLCD_RED             0xF800
#define	TFTLCD_GREEN           0x07E0
#define TFTLCD_CYAN            0x07FF
#define TFTLCD_MAGENTA         0xF81F
#define TFTLCD_YELLOW          0xFFE0 
#define TFTLCD_ORANGE          0xFC80 
#define TFTLCD_PURPLE          0x781F
#define TFTLCD_SIENNA          0xF811
#define TFTLCD_NEONYELLOW      0x9FE0
#define TFTLCD_EMERALD         0x05E5
#define TFTLCD_BLUE3           0x03FF
#define TFTLCD_ROYALBLUE       0x3333
#define TFTLCD_BROWN2          0x8888
#define TFTLCD_PINK            0xFADF
#define TFTLCD_DKPINK          0xFCDF
#define TFTLCD_BROWN1          0x7083
#define TFTLCD_OLIVE           0x6BC3
*/

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD,0);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);


#define MINPRESSURE 10
#define MAXPRESSURE 1000

#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

void writeLabels() {
  
  tft.fillScreen(TFTLCD_BLACK);
  tft(0,0,TFTLCD_WHITE,2)<<"Power";
  tft(80,0,TFTLCD_RED)<<"TV";
  tft(120,0,TFTLCD_YELLOW)<<"Radio";
  tft(200,0,TFTLCD_BLUE)<<"DVD";
}

void setup(void) {
  delay(1000);
  Serial.begin(57600);
  tft.reset();
  
  tft.begin(tft.readID());
  writeLabels();
}

void loop(void) {

  Point p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    writeLabels();
    tft(0,40,TFTLCD_GREEN,2)<<"X = "<<p.x;
    tft(0,100,TFTLCD_GREEN,2)<<"Y = "<<p.y;
    tft(0,180,TFTLCD_GREEN,2)<<"Pressure = "<<p.z;
      // scale from 0->1023 to tft.width
    p.x = map(p.x, TS_MINX, TS_MAXX, tft.width(), 0);
    p.y = map(p.y, TS_MINY, TS_MAXY, tft.height(), 0);
    tft(0,60,TFTLCD_RED)<<"mapped X="<<p.x;
    tft(0,120,TFTLCD_RED)<<"mapped Y="<<p.y;
    }

}


