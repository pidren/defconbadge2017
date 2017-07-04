#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>

#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
//XXX: Need to change this to some logo that is useful.
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


// ------ PIN MAPPINGS --------
#define BUILTIN_LED 16 // G of BOTTOM_MID_RGB
#define D0  16         // G of BOTTOM_MID_RGB
#define D1  5          // ?? -- screen?
#define D2  4          // ?? -- screen?
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

uint8_t all_pins[] = {D0,D3,D4,D5,D6,D7,D8,D9,D10,TL,TR};
uint8_t bottom_mid_rgb[] = {D4,D0,D3}; // R, G, B in indexing order
uint8_t bottom_right_rgb[] = {D7,D5,D6};
uint8_t bottom_left_rgb[] = {D10,D8,D9};



void showText(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println("YAY!!!");  
  display.print("DEFCON 25 BADGE");
  display.display();
  delay(1);
}

void setup()   {  
  //Set baud rate to 115200
  Serial.begin(115200);
  
  // Generate the high voltage from the 3.3v line internally
  // Initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  
  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay(); 

  Serial.println( "Initializing all pins to OUTPUT mode." );
  
  pinMode(D0,OUTPUT);
  pinMode(D3,OUTPUT);
  pinMode(D4,OUTPUT);
  pinMode(D5,OUTPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
  pinMode(D8,OUTPUT);
  pinMode(D9,OUTPUT);
  pinMode(D10,OUTPUT);
  pinMode(TL,OUTPUT);
  pinMode(TR,OUTPUT);
  
  /*
  // XXX: For some reason this code does not work.
  for(int i=0; sizeof(all_pins); i++){
    pinMode(all_pins[i],OUTPUT); 
    Serial.println(all_pins[i]);
    delay(600);
  }
  Serial.println("Done.");
  */
  
  showText();
}
  

void loop() {
  //As a test turn on R,G,B until all white
  //As a test turn on/off top LEDs
  digitalWrite(D4,HIGH);
  digitalWrite(D7,HIGH);
  digitalWrite(D10,HIGH);
  digitalWrite(TL,HIGH);
  digitalWrite(TR,HIGH);
  delay(500);
  digitalWrite(D0,HIGH);
  digitalWrite(D5,HIGH);
  digitalWrite(D8,HIGH);
  digitalWrite(TL,LOW);
  digitalWrite(TR,LOW);
  delay(500);
  digitalWrite(D3,HIGH);
  digitalWrite(D6,HIGH);
  digitalWrite(D9,HIGH);
  digitalWrite(TL,HIGH);
  digitalWrite(TR,HIGH);
  delay(500);
  digitalWrite(D0,LOW);
  digitalWrite(D3,LOW);
  digitalWrite(D4,LOW);
  digitalWrite(D5,LOW);
  digitalWrite(D6,LOW);
  digitalWrite(D7,LOW);
  digitalWrite(D8,LOW);
  digitalWrite(D9,LOW);
  digitalWrite(D10,LOW);
  digitalWrite(TL,LOW);
  digitalWrite(TR,LOW);
  delay(500);
}
