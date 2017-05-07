//******************************************************************************************LIBRAIRIES INCLUDED IN PROJECT
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <NeoEffects.h>
#include <NeoStrip.h>
#include <NeoWindow.h>
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Ticker.h>


//******************************************************************************************WifiSetup Variables (Will not be needed when adding in the WifiManager librairy features)
const char* ssid = "Sosebees 2.4g";
//const char* ssid = "SamsungS7";
const char* password = "a1b2c3d4e5";
WiFiUDP Udp;
//******************************************************************************************Incoming UDP Variables
char incomingPacket[255];  // buffer for incoming packets
//char  replyPacekt[] = "Hi there! Got the message :-)";  // a reply string to send back
unsigned int ListenUdpPort = 4210;  // local port to listen on
bool ConnectedToHost = false;

//******************************************************************************************Broadcast ID Variables
IPAddress BroadcastIP = {255,255,255,255}; //broadcast to all IP
const long ChipIdBraodcastInterval = 2000; //Broadcast Interval
unsigned long previousIDBroadcastTime = 0; //millis Last Update for Broadcast
unsigned int BroadcastUdpPort = 4211;  // local port to broadcast on
int32_t ChipID; //Hold the unique ID of the ESP8266 CHIP  
int32_t TempChipID; //Hold the unique ID of the ESP8266 CHIP  
//******************************************************************************************Button Press Variables
bool Active_Button = false; //This determines if the button is waiting to be pressed,
bool App_conn = false; // This determines if the button is actually being used "connected" to the app
#define BUTTON_PIN 12 //Main button pin number
bool oldState = LOW;
bool btnHasBeenClicked = false;
bool btnClickConfirmedByHost = false;
bool chipConfirmed = false;
bool inGame = false; 
int HostResetBtn = 0;
const long ButtonPressResend = 50; //Broadcast Interval
unsigned long previousButtonPressResend = 0; //millis Last Update for Broadcast
//******************************************************************************************NeoPixel Lights Variable 
#define PIXEL_PIN    4 //neopixel SDL pin
#define PIXEL_COUNT 24 //number of Neopixels

NeoStrip strip = NeoStrip(PIXEL_COUNT, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
NeoWindow ring = NeoWindow(&strip, 0, PIXEL_COUNT);

uint8_t Red, Green, Blue, PixelID, Speed, Dur, Anim, Dir, Bright, Num, Qty;//Holds command number ints

//******************************************************************************************Debug Mode
bool DebugMode = false;
bool WifiMode = true;

//******************************************************************************************WifiManager

bool WifiManagerLED = false;
bool WifiManagerLED2 = false;
Ticker ticker;
Ticker ticker2;

//******************************************************************************************WifiManager Status Functions
void tick()
{
   if(!WifiManagerLED)
   {
    WifiManagerLED = true;
    strip.fillStrip(Adafruit_NeoPixel::Color(10,0,0));
    strip.show();
   }
   else{
    WifiManagerLED = false;
    strip.fillStrip(Adafruit_NeoPixel::Color(0,0,0));
    strip.show();
   }

}

void tick2()
{
   if(!WifiManagerLED2)
   {
    WifiManagerLED2 = true;
    strip.fillStrip(Adafruit_NeoPixel::Color(0,0,10));
    strip.show();
   }
   else{
    WifiManagerLED2 = false;
    strip.fillStrip(Adafruit_NeoPixel::Color(0,0,0));
    strip.show();
   }

}


//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
    strip.clearStrip();
    strip.show();
    ticker2.detach();
    ticker.attach(1, tick);
}

//****************************************************************************************************************************************SETUP
//****************************************************************************************************************************************SETUP
void setup()
{
//*********************************************************************PINS
  pinMode(BUTTON_PIN, INPUT_PULLUP);
 //*********************************************************************NEOPIXEL INITIALIZE
 strip.begin();// Initialize Neopixel Control
 strip.setBrightness(255);//Set NeoPixel Brightness
 strip.clearStrip();
 strip.show();

 ticker2.attach(1, tick2);

//*********************************************************************SERIAL PORT
  if(DebugMode){
  Serial.begin(115200);
  Serial.println();
  Serial.println("Debug Mode: ON");
  delay(50);
  if(WifiMode){
  Serial.println("Wifi MOde: ON");
   delay(50);
  }
  else {Serial.println("Wifi MOde: OFF");
   delay(50);
  }
  }
//*********************************************************************WIFI-MANAGER INITIALIZE
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
 //reset saved settings
 //wifiManager.resetSettings();
 //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
 //set custom ip for portal
 wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); 

  if (!wifiManager.autoConnect("JustaButton")) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

 Udp.begin(ListenUdpPort);
 if(DebugMode)Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), ListenUdpPort);
 ticker2.detach();
 ticker.detach();
 
//*********************************************************************WIFI INITIALIZE //uncomment this for basic wifimade but must comment wifimanager section
/*
  if(WifiMode){
  if(DebugMode)Serial.printf("Trying to connect to %s ", ssid);
  delay(50);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {    
    delay(500);
    strip.fillStrip(Adafruit_NeoPixel::Color(0,0,255));
    strip.show();
    delay(500);
    strip.clearStrip();
    strip.show();    
  }
  
 if(DebugMode) Serial.println(" connected");
    strip.clearStrip();
    strip.show();
    strip.fillStrip(Adafruit_NeoPixel::Color(0,255,0));
    strip.show();
    delay(1500);  
  }

 if(WifiMode){
 Udp.begin(ListenUdpPort);
 if(DebugMode)Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), ListenUdpPort);
 }

   */

 //*********************************************************************PrepareBroadcast
 ChipID = ESP.getChipId();

 //*********************************************************************LED COnnection Animation
     strip.clearStrip();
     strip.show();
     ring.setBlinkEfx(Adafruit_NeoPixel::Color(10,6,0),5000, 1000);  
     strip.show();
    
}
//*********************************************************************************************************************************************MAIN LOOP:START
//*********************************************************************************************************************************************MAIN LOOP:START
void loop()
{
  
  NeoWindow::updateTime();
  
  if(DebugMode && Serial.available()){
    delay(50);
    SerialPortListen();  
  }  

if(WifiMode)
  {   
  ButtonIsPressed();
  if(inGame)sendButtonStatus();
  braodcastButtonID();
  UDPGetPacket(); 
  }

  if(!ring.effectDone())
  {
    ring.updateWindow();
    strip.show();
  }


  
 
}
//*********************************************************************************************************************************************MAIN LOOP:END
//*********************************************************************************************************************************************MAIN LOOP:END
//********************************************************************************************************Send CHIP_ID:BUTTON STATUS
void sendButtonStatus(){
  
 unsigned long currentMillis = millis();
 if (currentMillis - previousButtonPressResend >= ButtonPressResend) {
     previousButtonPressResend = currentMillis;
    
      if(btnHasBeenClicked == true && btnClickConfirmedByHost == false){      
          Udp.beginPacket(BroadcastIP, BroadcastUdpPort);
          Udp.printf( "$%d:*?",ChipID);
          Udp.endPacket();
          if(DebugMode)Serial.printf("ChipID %d",ESP.getChipId());
          if(DebugMode)Serial.println();
        }

 }
}


//********************************************************************************************************BROADCAST CHIP/BUTTON ID
void braodcastButtonID()
{  
  unsigned long currentMillis = millis();
  if (currentMillis - previousIDBroadcastTime >= ChipIdBraodcastInterval) 
  {
    previousIDBroadcastTime = currentMillis;
    Udp.beginPacket(BroadcastIP, BroadcastUdpPort);
    Udp.printf("$%d:%d?",ChipID,BatLevel());
    Udp.endPacket();
    
    if(DebugMode)Serial.printf("$%d:%d?",ChipID,BatLevel());
    if(DebugMode)Serial.println();
  }
}

int BatLevel()
{
  int level = analogRead(A0);
  level = map(level, 580, 774, 0, 100);
  return(level);
}
//********************************************************************************************************BUTTON PRESS ACTION
void ButtonIsPressed()
{
  bool newState = digitalRead(BUTTON_PIN);  // Get current button state.i
  if (newState == HIGH && oldState == LOW)   // Check if state changed from high to low (button press).
  {
    delay(20);    // Short delay to debounce button.
    newState = digitalRead(BUTTON_PIN);    // Check if button is still low after debounce.
    if (newState == HIGH) 
    {
     btnHasBeenClicked = true;
     btnClickConfirmedByHost = false;
     HostResetBtn = 0;
    }   
  }
  oldState = newState;  // Set the last button state to the old state.
}
//********************************************************************************************************PIXEL ANIMATION HANDLER
//This handles the NEopixel command by looking at the command number and then issuing the correct animation base on variables set. 

    /*
    EACH COMMANDSET MUST BEGIN WITH '!' AND END WITH '#'
    Example Code Set char* buf = "!&0:0&1:0&2:0&3:150&4:24&5:100&6:5000&7:2&8:2&9:60&10:100&11:10&12:0&#";
    
    By sending int of 0 instead of the ChipID the Neopixel handler will still execute. Allowing you to use this to send the same Neopixel command to all buttons.

    Command Definitions
   0. Chip ID - Chip ID of the Button 
   1. RED - LED COLOR TYPE INT_8 (0-255)
   2. GREEN - LED COLOR TYPE INT_8 (0-255)
   3. BLUE - LED COLOR TYPE INT_8 (0-255)
   4. PixelID - calls a single Neopixel number (0-23) 
   5. Speed - int (0-100)
   6. Duration - animation duraction int_32 (0-5000)
   7. Animation number int_8(0-100)
   8. Direction int_8 1 = clockwise ; 2 = counter clockwise
   9. Brightness int_8 (0-255)
  10. Number- number of pixels to animate int_8 (0-23)
  11. Qty- can be used to call any number such as number of flashes
  12. Reset Button - Hopst will send a "1" to notify the button that it recieved its notice that is has been triggered so no reset button

  NeoPixel Animation List by Command Number

  1. All Off
  2. Solid Color
  3. Hold : hold current pixel colors for time period
  4. Circle : one dot of specified color moves across window with time period between changes
  5. Wipe : wipe (fill) window one dot of color at a time, with delay between changes
  6. RandomWipe : wipe with random color in specified range
  7. Blink : blink all pixels in window on/off with specified color, rate and number blinks
  8. Sparkle : blink a random pixel in window with color (on/off time) then chose new pixel
  9. MultiSparkle : blink N random pixels in window on/off
 10. Fade : linear fade between two colors (once, cycling in/out, or jumping back to start)
 11. Police : Strobes Police like lights
   */
void NeoPixelHandler()// See Notes Above
{
  strip.clearStrip();
  strip.show();
  ring.setNoEfx();
  strip.show();
  
  switch (Anim) {
    case 1://All Off    
        ring.setNoEfx();
        break;
    case 2://Solid Color    
        strip.fillStrip(Adafruit_NeoPixel::Color(Red,Green,Blue));       
        break;
    case 3:   
      ring.setHoldEfx(Dur);
      break;
    case 4:   
      ring.setCircleEfx(Adafruit_NeoPixel::Color(Red,Green,Blue),Speed);     
      break;
    case 5:
      ring.setWipeEfx(Adafruit_NeoPixel::Color(Red,Green,Blue),Speed);
      break;
    case 6:
      ring.setWipeEfx(strip.randomColor(),Speed);
      break;  
    case 7:
      ring.setBlinkEfx(Adafruit_NeoPixel::Color(Red,Green,Blue),Speed, Qty); 
      break;
    case 8:
      ring.setSparkleEfx(Adafruit_NeoPixel::Color(Red,Green,Blue),Speed, Bright, Qty); 
      break;
    case 9:
      ring.setMultiSparkleEfx(Adafruit_NeoPixel::Color(Red,Green,Blue),Speed, Bright, PixelID, Qty); 
      break;
    case 10:
      ring.setFadeEfx(0,Adafruit_NeoPixel::Color(Red,Green,Blue), Speed, Bright, Qty); 
      break;
    case 12:
     //Do nothing. Use this when sending commands that have no LED control such as Button reset
      return;
    case 13:
    
       ring = NeoWindow(&strip, PixelID, Qty);
       ring.setWipeEfx(Adafruit_NeoPixel::Color(Red,Green,Blue),0);  
       break;       
  }

}
//********************************************************************************************************UDP PACKET HANDLER
//Checks to see if there is a packet, if so then convert to c string and parse to UDPPacketSepereator
void UDPGetPacket()
{
    int packetSize = Udp.parsePacket();
  if (packetSize) 
  {
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;// Add the final 0 to end the C string
      UDPPacketSeperator(incomingPacket, len);
    }
  }
}

//********************************************************************************************************UDP PACKET SENDER
//Send the information of a character array
void UDPSendPacket(char  replyPacekt[])
{
    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(BroadcastIP, BroadcastUdpPort);
    Udp.write(replyPacekt);
    Udp.endPacket();
}

//********************************************************************************************************UDP PACKET SEPERATOR
//Seperates makes sure the UDP is complete (starts with ! ends with #), If so then is seperates each command (by '&') and the seperates the command to from the command (':')
void UDPPacketSeperator(char UDPPacket[255], int len)
{
    
  if(UDPPacket[0] == '!' && UDPPacket[len-1] == '#') //Checksum to makesure the packet is a whole paceket.
{  
  if(DebugMode)Serial.print("Good Packet: ");
  if(DebugMode)Serial.println(UDPPacket);
     
    if(WifiMode)BroadcastIP = Udp.remoteIP();//Since the packet is confirmed, lets grab the IP it came from instead of broadcasting to 255.255.255.255. This will speed up the data.
       
    // Read each command pair 
    char* command = strtok(UDPPacket, "&");
    while (command != 0)
    {
        // Split the command in two values
        char* separator = strchr(command, ':');
        if (separator != 0)
        {
            // Actually split the string in 2: replace ':' with 0
            *separator = 0;
            int commandNumber = atoi(command);
            ++separator;
            int commandValue = atoi(separator);

          switch(commandNumber)
          {
            case 0:
              if(commandValue == ChipID)
              {
              chipConfirmed = true;               
              break;
              }
              else if(commandValue == 0)
              {
              break;  
              }
              else
              {              
                return; //If the CHIP ID of the packet doesn't match this ChipID then ignore the command as it is for a different button.
              }
            case 1:
              Red = commandValue;
              break;
            case 2:
              Green = commandValue;
              break;
            case 3:
              Blue = commandValue;
            case 4:
             PixelID = commandValue;
             break;
            case 5:
             Speed = commandValue;
             break;
            case 6:
              Dur = commandValue;
              break;
            case 7:
              Anim = commandValue;
              break;
            case 8:
              Dir = commandValue;
              break;
            case 9:
              Bright = commandValue;
              break;
            case 10:
              Num = commandValue;
              break;
            case 11:
              Qty = commandValue;
              break;
            case 12:
              HostResetBtn = commandValue;
              break;     
            case 13:
              if (commandValue == 1)
              {
                inGame = true; 
              }
              else if(commandValue == 0){
                inGame = false;
              }
              break;            
          }    
        }
        // Find the next command in input string
        command = strtok(0, "&");
    }

     if(chipConfirmed == true && ConnectedToHost == false)
     {        
      ConnectedToHost = true;
      //strip.fillStrip(Adafruit_NeoPixel::Color(0,255,0));
      //strip.show();
     // delay(2000);    

        Red = 0;
        Green = 10;
        Blue = 0;
        PixelID = 0;
        Speed = 0;
        Dur = 0;
        Anim = 2;
        Dir = 0;
        Bright = 0;
        Num = 0;
        Qty = 0;
     }
    

    if(chipConfirmed == true && HostResetBtn == 1)
    {
       btnHasBeenClicked = false;
       btnClickConfirmedByHost = true;    
    }
    
    if(chipConfirmed)NeoPixelHandler(); //NOW THAT NEOPIXELHANDLER VARIABLES HAVE BEEN SET, EXECUTE NEOPIXEL HANDLER
}
else
{
  if(DebugMode)Serial.print("Bad Packet: ");
  if(DebugMode)Serial.println(UDPPacket);
}
}

//********************************************************************************************************Serial Port Listen for and Execute Command

void SerialPortListen()
{
  int incomingSerialDataIndex = 0; 
  char incomingSerialData[255];
  while(Serial.available())
       {
         incomingSerialData[incomingSerialDataIndex] = Serial.read();
         incomingSerialDataIndex++;
         incomingSerialData[incomingSerialDataIndex] = '\0';
       }
       UDPPacketSeperator(incomingSerialData, incomingSerialDataIndex);      
}

//******************************************************************************************************** NeoPixelEffect Color Range





