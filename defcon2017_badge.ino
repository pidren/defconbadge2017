#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

//--------------------------------------------------------------------------------
// ------------------------------ BITMAP LOGO ------------------------------------
//--------------------------------------------------------------------------------
//*** TODO: Need to change this to some logo that is useful.
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B00000000,
  B00000001, B10000000,
  B00000011, B11000000,
  B00000111, B11100000,
  B00001111, B11110000,
  B00011111, B11111000,
  B00111111, B11111100,
  B01111111, B11111110,
  B00111111, B11111100,
  B00011111, B11111000,
  B00001111, B11110000,
  B00000111, B11100000,
  B00000011, B11000000,
  B00000001, B10000000,
  B00000000, B00000000,
  B00000000, B00000000 };

//--------------------------------------------------------------------------------
// ------------------------------ PIN MAPPINGS -----------------------------------
//--------------------------------------------------------------------------------
#define BUILTIN_LED 16 // G of BOTTOM_MID_RGB
#define D0  16         // G of BOTTOM_MID_RGB
#define D1  5          // screen - DO NOT USE!!! Already handled by library.
#define D2  4          // screen - DO NOT USE!!! Already handled by library.
#define D3  0          // B of BOTTOM_MID_RGB
#define D4  2          // R of BOTTOM_MID_RGB, 
#define D5  14         // G of BOTTOM_RIGHT_RGB
#define D6  12         // B of BOTTOM_RIGHT_RGB
#define D7  13         // R of BOTTOM_RIGHT_RGB
#define D8  15         // G of BOTTOM_LEFT_RGB
#define D9  3          // B of BOTTOM_LEFT_RGB
#define D10 1          // R of BOTTOM_LEFT_RGB
#define TL  10         // TOPLEFT LED -- this is also SDD3
#define TR  9          // TOPRIGHT LED -- tihs is also SDD2

//RGB grouping per RGB LED in R, G, B order
uint8_t BOT_MD_RGB[] = {D4,D0,D3};
uint8_t BOT_RT_RGB[] = {D7,D5,D6};
uint8_t BOT_LT_RGB[] = {D10,D8,D9};

//R, G, B constants for easy access.
#define R 0
#define G 1
#define B 2


//--------------------------------------------------------------------------------
//--------------------------------- TEXT HELPERS ---------------------------------
//--------------------------------------------------------------------------------
void setBanner(void){
  // *** TODO: Want to use logo16_glcd_bmp for bitmap logo here ***
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(12,0);
  display.print("DEFCON 25");
  display.setCursor(12,20);
  display.print("<< MRB >>");
  display.setCursor(28,55);
  display.setTextSize(1);
  display.print("CUSTOM BADGE");
}

void setLoading(void){
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,25);
  display.println("Loading...");
}

void setupRGBs(void){
  setRGB(BOT_MD_RGB, 0, 0, 0);
  setRGB(BOT_LT_RGB, 0, 0, 0);
  setRGB(BOT_RT_RGB, 0, 0, 0);
}

// *** TODO: TEST SCROLL in 3 LANES


//--------------------------------------------------------------------------------
//---------------------------- ANIMATION HELPERS ---------------------------------
//--------------------------------------------------------------------------------
void setRGB(uint8_t* rgb, uint8_t r_chan, uint8_t g_chan, uint8_t b_chan){
  //Set RGB channels for the RGB LEDs
  analogWrite(rgb[R], r_chan);
  analogWrite(rgb[G], g_chan);
  analogWrite(rgb[B], b_chan);
}

void setLED(uint8_t led, uint8_t v){
  analogWrite(led, v);
}

void rainbowRGB(uint8_t* rgb){
  //Tries to simulate rainbow for all RGB LEDs
  //Red(255,0,0) --> Green(0,255,0) --> Blue(0,0,255)
  setRGB(rgb,255,0,0);
  delay(300);
  for(int i=255; i>=0; i--){
    setRGB(rgb, i, abs(i-255), 0);
    delay(2);
  }
  for(int i=255; i>=0; i--){
    setRGB(rgb,0, i, abs(i-255));
    delay(2); 
  }    
}

void flickerLED(uint8_t led){
  //Quick flicker animation for LED, best used for visual status
  digitalWrite(led, HIGH);
  delay(10);
  digitalWrite(led, LOW);
}

//--------------------------------------------------------------------------------
//---------------------------- WIFI SERVER HELPERS -------------------------------
//--------------------------------------------------------------------------------
//*** TODO: listen to packet and synchronously scroll text



//--------------------------------------------------------------------------------
//----------------------------------- SETUP --------------------------------------
//--------------------------------------------------------------------------------
void setup()   {  
  // Set baud rate to 115200 for Serial Logs
  Serial.begin(115200);
  Serial.print("Starting badge");
  
  // Generate the high voltage from the 3.3v line internally
  // Initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  

  // Initialize Pins (note: putting this in a loop does not work.)
  pinMode(BOT_MD_RGB[R],OUTPUT);
  pinMode(BOT_MD_RGB[G],OUTPUT);
  pinMode(BOT_MD_RGB[B],OUTPUT);
  pinMode(BOT_LT_RGB[R],OUTPUT);
  pinMode(BOT_LT_RGB[G],OUTPUT);
  pinMode(BOT_LT_RGB[B],OUTPUT);
  pinMode(BOT_RT_RGB[R],OUTPUT);
  pinMode(BOT_RT_RGB[G],OUTPUT);
  pinMode(BOT_RT_RGB[B],OUTPUT);
  pinMode(TL,OUTPUT);
  pinMode(TR,OUTPUT);
  
  //Initialize RGB LED
  setupRGBs();
  
  // Show loading screen.
  display.clearDisplay(); 
  setLoading();
  display.display();
  // *** TODO: Initialize WiFi Server Here ***
  delay(2000);
  
  // Show Default Banner
  display.clearDisplay();
  setBanner();
  display.display();
  delay(100);
}
  

//--------------------------------------------------------------------------------
//--------------------------------- MAIN LOOP ------------------------------------
//--------------------------------------------------------------------------------
void loop() {
  rainbowRGB(BOT_MD_RGB);
  flickerLED(TL);
  //*** TODO: LISTEN FOR PACKET ***
  rainbowRGB(BOT_LT_RGB);
  flickerLED(TR);
  //*** TODO: LISTEN FOR PACKET ***
  rainbowRGB(BOT_RT_RGB);
  flickerLED(TL);
  flickerLED(TR);
  //*** TODO: LISTEN FOR PACKET ***
  //*** May want to unroll rainbowRGB and listen in between setRGB calls
}
