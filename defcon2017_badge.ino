#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

//--------------------------------------------------------------------------------
// ---------------------------------- SCREEN -------------------------------------
//--------------------------------------------------------------------------------
#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 
#define OLED_RESET LED_BUILTIN //4
Adafruit_SSD1306 display(OLED_RESET);


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
//---------------------------- EXPECTED BADGE ID ---------------------------------
//--------------------------------------------------------------------------------
String macIDs[6] = { "A0:20:A6:16:01:62",
                     "A0:20:A6:17:50:6A",
                     "A0:20:A6:1B:B2:DC",
                     "A0:20:A6:16:D1:7D",
                     "A0:20:A6:17:4D:1F",
                     "A0:20:A6:17:47:91" };
String ID;
void getMyID(void){
  for(int i=0; i<(sizeof(macIDs)/sizeof(String));i++){
    if(macIDs[i].equals(String(WiFi.macAddress()))){
      ID = "MRB" + String(i);
    } 
  }
}


//--------------------------------------------------------------------------------
//---------------------------- ANIMATION HELPERS ---------------------------------
//--------------------------------------------------------------------------------
#define START_X 0
#define START_STATUS_Y 0
#define START_TEXT_Y 18

void writeStatus(String s1, String s2="", String s3="", String s4="", String s5=""){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(START_X,START_STATUS_Y);
  display.println(WiFi.localIP());
  display.println(WiFi.macAddress());
  display.setCursor(START_X,START_TEXT_Y);
  display.println(s1);
  display.println(s2);
  display.println(s3);
  display.println(s4);
  display.println(s5);
  display.display();
  delay(500);
}

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

void rainbowAllRGB(void){
  setRGB(BOT_LT_RGB,255,0,0);
  setRGB(BOT_RT_RGB,255,0,0);
  setRGB(BOT_MD_RGB,255,0,0);
  delay(300);
  for(int i=255; i>=0; i--){
    setRGB(BOT_LT_RGB, i, abs(i-255), 0);
    setRGB(BOT_RT_RGB, i, abs(i-255), 0);
    setRGB(BOT_MD_RGB, i, abs(i-255), 0);
    delay(2);
  }
  for(int i=255; i>=0; i--){
    setRGB(BOT_LT_RGB, 0, i, abs(i-255));
    setRGB(BOT_RT_RGB, 0, i, abs(i-255));
    setRGB(BOT_MD_RGB, 0, i, abs(i-255));
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
#define PORT 80
WiFiUDP node;
//I only printed 6 of these so there should be 5 other siblings at most...
IPAddress siblings[5];

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

void setupNode(void){
  WiFi.hostname(ID);
  WiFi.begin("Unicorn","stankybutt");
  while(WiFi.status() != WL_CONNECTED){
    flickerLED(TL);
    delay(250);
    flickerLED(TR);
    printWifiStat(WiFi.status());
    delay(250);
  }  
  node.begin(PORT);
} 

boolean missingSiblings(){
  for(int i=0; i < (sizeof(siblings)/sizeof(IPAddress)); i++){
    if(siblings[i] == IPAddress(0,0,0,0)){
      return true; 
    }    
  }
  return false;
}

boolean addSibling(IPAddress newsib){
  for(int i=0; i < (sizeof(siblings)/sizeof(IPAddress)); i++){
    if(siblings[i] == newsib){
      //If there is a duplicate, early exit
      return false; 
    }
    
    if(siblings[i] == IPAddress(0,0,0,0)){
      //If new sibling added, early exit
      siblings[i] = newsib;
      return true; 
    }
  }
  return false;
}

int numSiblings(void){
  int i=0;
  for(; i < (sizeof(siblings)/sizeof(IPAddress)); i++){
    if(siblings[i] == IPAddress(0,0,0,0)){
      return i;
    }    
  }
  return i;
}


void respond(void){
  writeStatus("Listening...");
  flickerLED(TL);
  delay(200);
  flickerLED(TR);
  delay(200);
  
  //If can add more siblings, try to broadcast yourself
  if(missingSiblings()) {
    IPAddress broadcastIP = WiFi.localIP();
    broadcastIP[3] = 255; //turn last octal to broadcast
    char buf[] = "BROADCAST";
    node.beginPacket(broadcastIP, PORT);
    node.write(buf);
    node.endPacket();
  }
  
  int pkt = node.parsePacket();
  //Make sure we ignore packets that came from the badge itself
  if(pkt && (node.remoteIP() != WiFi.localIP())){
    //Received something!
    flickerLED(TL);
    flickerLED(TR);
     
    //Read in the packet payload and null terminate the end.
    char pktBuf[255];
    int len = node.read(pktBuf,255);
    if (len > 0) {
      pktBuf[len] = 0; 
    }
    
    //If payload contains "BROADCAST" message, try to add the new sibling to list of siblings
    if(String(pktBuf) == "BROADCAST"){
      if(addSibling(node.remoteIP())){
        writeStatus("Added new sibling: " + node.remoteIP().toString()); 
        setRGB(BOT_LT_RGB, 0, 255, 0);
        setRGB(BOT_RT_RGB, 0, 255, 0);
        setRGB(BOT_MD_RGB, 0, 255, 0);
        delay(200);
      } else {
        writeStatus("Didn't add sibling: " + node.remoteIP().toString());
        setRGB(BOT_LT_RGB, 255, 0, 0);
        setRGB(BOT_RT_RGB, 255, 0, 0);
        setRGB(BOT_MD_RGB, 255, 0, 0);
        delay(200);
      }
      writeStatus("Number of Siblings: " + String(numSiblings()));
      delay(200);
    } else {
      rainbowAllRGB();
      writeStatus("Received: " + String(pktBuf), "From: " + node.remoteIP().toString());
      delay(200);
    }
      
    //Reply with a simple hello!
    String msg = "Hello from " + WiFi.localIP().toString();
    char replyBuf[msg.length()+1];
    msg.toCharArray(replyBuf,msg.length()+1);
    node.beginPacket(node.remoteIP(), PORT);
    node.write(replyBuf);
    node.endPacket();
    
    //Let's say hello to any other siblings we already have
    writeStatus("Saying hi to all siblings");
    for(int i=0; i<(sizeof(siblings)/sizeof(IPAddress)); i++){
      node.beginPacket(siblings[i], PORT);
      node.write(replyBuf);
      node.endPacket();
    }
  } else {
    //Didn't receive a packet.
    writeStatus("Received nothing.");
  }
}


//--------------------------------------------------------------------------------
//----------------------------------- SETUP --------------------------------------
//--------------------------------------------------------------------------------
void setup()   {  
  delay(500);
  // Set baud rate to 115200 for Serial Logs
  Serial.begin(115200);
  
  //Clear siblings IPAddress list
  memset(siblings, 0, sizeof(siblings));
  
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
  
  // Initialize Pins (note: putting this in a loop does not work.)
  display.setCursor(0,16);
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
  //pinMode(TR,OUTPUT); // This needs to be commented out for green board
  delay(500);
  
  //Initialize RGB LED
  display.setCursor(0,26);
  display.println("Preparing RGBs...");
  display.display();
  setupRGBs();
  delay(800);

  //Setup Server
  display.setCursor(0,36);
  display.print("Starting server...");
  display.display();
  setupNode();  
  display.setCursor(0,46);
  display.println("Done.");
  display.display();
  delay(800);
}


//--------------------------------------------------------------------------------
//--------------------------------- MAIN LOOP ------------------------------------
//--------------------------------------------------------------------------------
void loop() { 
  respond();
  delay(200);
}

