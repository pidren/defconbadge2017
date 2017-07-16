#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h> //***TODO*** MIght not need this
#include <ESP8266WebServer.h> //***TODO*** Might not need this
#include <ESP8266mDNS.h> //***TODO*** Might not need this

#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

//--------------------------------------------------------------------------------
// ------------------------------ BITMAP LOGO ------------------------------------
//--------------------------------------------------------------------------------
//*** TODO: Need to change this to some logo that is useful.
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000001, B10000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B00000111, B11110000,
  B11111111, B11111111,
  B00011111, B11111000,
  B00001111, B11110000,
  B00000111, B11100000,
  B00001111, B11110000,
  B00011111, B11111000,
  B00111110, B01111100,
  B01111000, B00011110,
  B11000000, B00000011,
  B00000000, B00000000,
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
//--------------------------------- WIFI SERVER ----------------------------------
//--------------------------------------------------------------------------------
WiFiServer server(80); //ESP8266WebServer serv(80);


//--------------------------------------------------------------------------------
//--------------------------------- TEXT HELPERS ---------------------------------
//--------------------------------------------------------------------------------
void writeStatus(String s){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(s);
  display.display();
}

void writeBanner(void){
  display.clearDisplay();
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
  display.display();
}

//--------------------------------------------------------------------------------
//---------------------------- ANIMATION HELPERS ---------------------------------
//--------------------------------------------------------------------------------
void setupRGBs(void){
  setRGB(BOT_MD_RGB, 0, 0, 0);
  setRGB(BOT_LT_RGB, 0, 0, 0);
  setRGB(BOT_RT_RGB, 0, 0, 0);
}

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
  Serial.println("Showing RGB Rainbow");
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
  Serial.println("Flickering LED" + String(led));
  //Quick flicker animation for LED, best used for visual status
  digitalWrite(led, HIGH);
  delay(10);
  digitalWrite(led, LOW);
}

//--------------------------------------------------------------------------------
//---------------------------- WIFI SERVER HELPERS -------------------------------
//--------------------------------------------------------------------------------
void printWifiStat(int s){
  switch(s){
    case WL_CONNECTED:
      Serial.println( "CONNECTED" );
      break;
    case WL_NO_SHIELD:
      Serial.println( "NO SHIELD" ); 
      break;
    case WL_IDLE_STATUS:
      Serial.println( "Idle" );
      break;
    case WL_NO_SSID_AVAIL:
      Serial.println( "NO SSID" ); 
      break;
    case WL_SCAN_COMPLETED:
      Serial.println( "NETWORK SCAN COMPLETE" );  
      break; 
    case WL_CONNECT_FAILED:
      Serial.println( "FAILED" ); 
      break;
    case WL_CONNECTION_LOST:
      Serial.println( "Connection lost" );
      break;
    case WL_DISCONNECTED:
      Serial.println( "DISCONNECTED " );
      break; 
  }
}

void setupServer(const char* ssid, const char* password){
  WiFi.begin(ssid,password);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    printWifiStat(WiFi.status());
  }  
  server.begin();
}
uint8_t cnt = 0;
void serverListen(void){
  cnt++;
  writeStatus("SERVER LISTENING " + String(cnt));
  //WiFiClient client = server.available();
  //if(client){
  //  if(client.connected()){
  //    Serial.println("Connected to client.");
  //  } else {
  //    Serial.println("Not connected to client."); 
  //  }
  //  client.stop();
  //} else {
  //  Serial.println("No client detected.");
  //}
}

//--------------------------------------------------------------------------------
//----------------------------------- SETUP --------------------------------------
//--------------------------------------------------------------------------------
void setup()   {  
  // Set baud rate to 115200 for Serial Logs
  Serial.begin(115200);
  
  // Generate the high voltage from the 3.3v line internally
  // Initialize with the I2C addr 0x3D (for the 128x64)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  
  //Instead of writing to Serial we'll show loading messages on screen.
  //Keeping showing loading screen until ready
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Loading...");
  display.display();
  
  //Initialize RGB LED
  display.setCursor(0,18);
  display.println("Preparing RGBs...");
  display.display();
  setupRGBs();
  delay(800);

  //Setup Server
  display.setCursor(0,28);
  display.println("Starting server...");
  display.display();
  const char* ssid = "Unicorn"; //***TODO*** CHANGE THIS  
  const char* password = "stankybutt"; //***TODO*** CHANGE THIS
  setupServer(ssid, password);  
  display.setCursor(0,38);
  display.println("Connected to " + String(ssid));
  display.display();
  display.setCursor(0,48);
  display.println("My IP is " + String(WiFi.localIP()));
  display.display();
  delay(800);
  
  // Initialize Pins (note: putting this in a loop does not work.)
  display.setCursor(0,58);
  display.println("Preparing pins...");
  display.display();
  pinMode(BOT_MD_RGB[R],OUTPUT);
  pinMode(BOT_MD_RGB[G],OUTPUT);
  pinMode(BOT_MD_RGB[B],OUTPUT);
  pinMode(BOT_LT_RGB[R],OUTPUT); // This pin is screwing up serial messages --- BYPASS BY WRITING TO SCREEN
  pinMode(BOT_LT_RGB[G],OUTPUT);
  pinMode(BOT_LT_RGB[B],OUTPUT);
  pinMode(BOT_RT_RGB[R],OUTPUT);
  pinMode(BOT_RT_RGB[G],OUTPUT);
  pinMode(BOT_RT_RGB[B],OUTPUT);
  pinMode(TL,OUTPUT);
  pinMode(TR,OUTPUT);
  delay(800);
  
  // Show Default Banner
  writeBanner();
  delay(8000);
}
  

//--------------------------------------------------------------------------------
//--------------------------------- MAIN LOOP ------------------------------------
//--------------------------------------------------------------------------------
void loop() {
  serverListen();  
  rainbowRGB(BOT_MD_RGB);
  flickerLED(TL);
  rainbowRGB(BOT_LT_RGB);
  flickerLED(TR);
  rainbowRGB(BOT_RT_RGB);
  serverListen();  
  flickerLED(TL);
  flickerLED(TR);
  //***TODO*** May want to unroll rainbowRGB and listen in between setRGB calls
}
