//***************************************************************
// UPDATE LOG | Date - Comment
//
// 12/3/19 - Inital file created. 
// 12/13/19 - Added new RF24 Radio Class
// 12/23/19 - Conversion back to MySensor Library. Changed to PA level high
// 09/03/20 - Converted to ESP
// 09/10/20 - Problem with setallone. Times out. possibly use yield()? 
//
//***************************************************************
//
//When uploading. Make shure connect to PRIVATE network
//
//Enable NetBIOS over IP
//
//***************************************************************

#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ArduinoOTA.h>

#include "index.h"




//Web socket: https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html

//#ifndef APSSID
#define APSSID "Im a Car Morty! Im Car RICK"
#define APPSK  "thereisnopassword"
//#endif

const char *ssid = APSSID;
const char *password = APPSK;

IPAddress local_IP(172,20,48,25);
IPAddress gateway(172,20,48,1);
IPAddress subnet(255,255,255,0);
IPAddress dns(8, 8, 8, 8);  //DNS


//Define Pin Usage
#define PIN_NEOPIXEL 3 //RX Dosent go high on boot.

#define PIN_UPLOAD 0 //High during upload. Ground to load bootloader. 
#define PIN_TX 1 //TX Dosent go high on boot.
#define PIN_LED 2 //LED //High during upload


#define STRIPLENGTH            40 //Really 39 but 1st light is in the conroller box

/* Set these to your desired credentials. */
const char *OTAName = "frontnodeesp";           // A name and a password for the OTA service

const char* PARAM_ALL = "all";
const char* PARAM_ONOFF = "of";
const char* PARAM_MODE = "m";
const char* PARAM_BRIGHTNESS = "br";
const char* PARAM_RED = "r";
const char* PARAM_GREEN = "g";
const char* PARAM_BLUE = "b";
const char* PARAM_DELAY = "d";


AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "404 Not found");
}



class HeartBeat
{
public:
  int setup();
  int loop();
  void recievedHeartbeat();

  //Varables

private:
  void processHeartbeat();

  boolean _HeartBeatRecieved = true;
  boolean _LastHeartBeatRecieved = false;
  
  //Varables
  const uint16_t _Delay = 60 * 1000;
  uint8_t _CurrentValue = 1;
  uint8_t _LastValue = 1;
  unsigned long _LastRun = 0;
};
class Lights
{
public:
  int setup();
  int loop();

  int valueOnOff();
  int valueMode();
  int valueBrightness();
  int valueRed();
  int valueGreen();
  int valueBlue();
  int valueDelay();

  bool newCommand = false;

  void setNextState(int onOff, int mode, int brightness, int red, int green, int blue, int delay);
private:
  void setLights();
  void setAll(uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t brightness);
  void setInside(uint8_t r, uint8_t g, uint8_t b);
  void setOutside(uint8_t r, uint8_t g, uint8_t b);
  void setAllone(uint8_t r, uint8_t g, uint8_t b);
  void setAlltwo(uint8_t r, uint8_t g, uint8_t b);
  void rainbowAllOutside(uint8_t r, uint8_t g, uint8_t b);
  void rainbowCycle();

  uint32_t Wheel(byte WheelPos);

  //Varables
  uint16_t _color = 200;

  uint8_t _OnOff = 1;
  uint8_t _Mode = 1;
  uint8_t _Brightness = 255;
  uint8_t _Red = 255;
  uint8_t _Green = 255;
  uint8_t _Blue = 150;

  uint8_t _outOne[7] = { 14, 15, 16, 17, 18, 19, 20 };
  uint8_t _outTwo[7] = { 27, 28, 29, 30, 31, 32, 33 };
  uint8_t _outThree[7] = { 7, 6, 5, 4, 3, 2, 1 };
  uint8_t _outsideC[21] = { 14, 15, 16, 17, 18, 19, 20, 27, 28, 29, 30, 31, 32, 33, 7, 6, 5, 4, 3, 2, 1 };

  uint8_t _inOne[6] = { 26, 25, 24, 23, 22, 21 };
  uint8_t _inTwo[6] = { 39, 38, 37, 36, 35, 34 };
  uint8_t _inThree[6] = { 8, 9, 10, 11, 12, 13 };
  uint8_t _insideC[18] = { 26, 25, 24, 23, 22, 21, 39, 38, 37, 36, 35, 34, 8, 9, 10, 11, 12, 13 };

  uint16_t _rainbowWheelInput[392];
  uint16_t _rainbowWheelInputSize;
  uint16_t _rainbowLocation = 0;

  uint8_t _setAllLocation = 0;
  uint8_t _setAllPos = 0;

  Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
  unsigned long _LastRun = 0;
  uint16_t _Delay = 200;
};


int Lights::setup()
{

  _rainbowWheelInputSize = (sizeof(_rainbowWheelInput) / sizeof(uint16_t));

  pinMode(PIN_NEOPIXEL, OUTPUT);
  //setAll(0, 0, 0);
  //setBrightness(30);
  strip.setPixelColor(0, strip.Color(0, 0, 0)); //First Pixel stays off forever
  strip.show();

  //36-231
  int r = 36;
  for (int i = 0; i < 195; i++)
  {
    _rainbowWheelInput[i] = r;
    r++;
    //Serial.println(rainbow[i]);
  }

  r = 231;
  for (int i = 195; i < 391; i++)
  {
    _rainbowWheelInput[i] = r;
    r--;
    //Serial.println(rainbow[i]);
    //Serial.println(i);
  }
  _rainbowWheelInput[391] = 0;



  setNextState(1, 1, 180, 0, 255, 150, 500);
  return 1;
}
int Lights::loop()
{
  if ((millis() - _LastRun) >= _Delay)
  {
    setLights();
    //Serial.println(_OnOffValue);
    _LastRun = millis();

  }

  return 1;
}

void Lights::setLights()
{
  if (_OnOff == 1)
  {//Power on
    setBrightness(_Brightness);

    switch (_Mode)
    {
      //OnOff, Mode, Brightness, Red, Green, Blue, Delay
    case 0: //Solid Color
      setAllone(_Red, _Green, _Blue);
      break;
    case 1: //Solid Color
      setAlltwo(_Red, _Green, _Blue);
      break;
    case 2: //Rainbow Cycle
      rainbowCycle();
      break;
    case 3: //Rainbow All
      rainbowAllOutside(_Red, _Green, _Blue);
      //setAllone(_Red, _Green, _Blue);
      break;
    }

  }
  else //Off
  {
    setAll(0, 0, 0);
    setBrightness(0);
  }
  newCommand = false;
}
void Lights::setNextState(int onOff = 0, int modee = 0, int brightness = 0, int red = 0, int green = 0, int blue = 0, int delayy = 5000)
{
  _OnOff = onOff;
  _Mode = modee;
  _Brightness = brightness;
  _Red = red;
  _Green = green;
  _Blue = blue;
  _Delay = delayy;
  
  setLights();
  _LastRun = millis();
}
int Lights::valueOnOff()
{
  return _OnOff;
}
int Lights::valueMode()
{
  return _Mode;
}
int Lights::valueBrightness()
{
  return _Brightness;
}
int Lights::valueRed()
{
  return _Red;
}
int Lights::valueGreen()
{
  return _Green;
}
int Lights::valueBlue()
{
  return _Blue;
}
int Lights::valueDelay()
{
  return _Delay;
}


void Lights::setAll(uint8_t r, uint8_t g, uint8_t b)
{
  setOutside(r, g, b);
  setInside(r, g, b);
}
void Lights::setOutside(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t color = strip.Color(r, g, b);
  for (uint16_t i = 0; i < sizeof(_outsideC); i++)
  {
    strip.setPixelColor(_outsideC[i], color);
  }
  strip.show();
}
void Lights::setInside(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t color = strip.Color(r, g, b);
  for (uint16_t i = 0; i < sizeof(_insideC); i++)
  {
    strip.setPixelColor(_insideC[i], color);
  }
  strip.show();
}
void Lights::setAllone(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t color = strip.Color(r, g, b);
  
  if (newCommand)
  {
    _setAllLocation = 0;
    _setAllPos = 0;
  }
  
  switch (_setAllLocation)
  {
    case 0:
      if (_setAllPos < sizeof(_outOne))
      {
        strip.setPixelColor(_outOne[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllPos = 0;
        _setAllLocation++;
      }
      break;
    case 1:
      if (_setAllPos < sizeof(_outTwo))
      {
        strip.setPixelColor(_outTwo[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllLocation++;
        _setAllPos = 0;
      }
      break;
    case 2:
      if (_setAllPos < sizeof(_outThree))
      {
        strip.setPixelColor(_outThree[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllLocation++;
        _setAllPos = 0;
      }
      break;
    case 3:      
      if (_setAllPos == 0)
      {
        _setAllPos = 2;
        strip.setPixelColor(_inOne[_setAllPos], color);
        _setAllPos = 3;
        strip.setPixelColor(_inOne[_setAllPos], color);
        strip.show();
        _Delay = _Delay + 500;
        _setAllPos++;
      }
      else if (_setAllPos == 1)
      {
        _setAllPos = 2;
        strip.setPixelColor(_inTwo[_setAllPos], color);
        _setAllPos = 3;
        strip.setPixelColor(_inTwo[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else if (_setAllPos == 2)
      {
        _setAllPos = 2;
        strip.setPixelColor(_inThree[_setAllPos], color);
        _setAllPos = 3;
        strip.setPixelColor(_inThree[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllLocation++;
        _setAllPos = 0;
      }
      break;
    case 4:
      if (_setAllPos == 0)
      {
        _setAllPos = 1;
        strip.setPixelColor(_inOne[_setAllPos], color);
        _setAllPos = 4;
        strip.setPixelColor(_inOne[_setAllPos], color);
        strip.show();
        _Delay = _Delay + 500;
        _setAllPos++;
      }
      else if (_setAllPos == 1)
      {
        _setAllPos = 1;
        strip.setPixelColor(_inTwo[_setAllPos], color);
        _setAllPos = 4;
        strip.setPixelColor(_inTwo[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else if (_setAllPos == 2)
      {
        _setAllPos = 1;
        strip.setPixelColor(_inThree[_setAllPos], color);
        _setAllPos = 4;
        strip.setPixelColor(_inThree[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllLocation++;
        _setAllPos = 0;
      }
      break;
    case 5:
      if (_setAllPos == 0)
      {
        _setAllPos = 0;
        strip.setPixelColor(_inOne[_setAllPos], color);
        _setAllPos = 5;
        strip.setPixelColor(_inOne[_setAllPos], color);
        strip.show();
        _Delay = _Delay + 500;
        _setAllPos++;
      }
      else if (_setAllPos == 1)
      {
        _setAllPos = 0;
        strip.setPixelColor(_inTwo[_setAllPos], color);
        _setAllPos = 5;
        strip.setPixelColor(_inTwo[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else if (_setAllPos == 2)
      {
        _setAllPos = 0;
        strip.setPixelColor(_inThree[_setAllPos], color);
        _setAllPos = 5;
        strip.setPixelColor(_inThree[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllLocation++;
        _setAllPos = 0;
      }
      break;
    case 6:
      _Delay = 5000;
      _setAllLocation++;
      break;
    case 7:
      break;
  }
}
void Lights::setAlltwo(uint8_t r, uint8_t g, uint8_t b) //Outside, then inside
{
  uint32_t color = strip.Color(r, g, b);

  if (newCommand)
  {
    _setAllLocation = 0;
    _setAllPos = 0;
  }
  
  switch (_setAllLocation)
  {
    case 0:
      if (_setAllPos < sizeof(_outOne))
      {
        strip.setPixelColor(_outOne[_setAllPos], color);
        strip.setPixelColor(_outTwo[_setAllPos], color);
        strip.setPixelColor(_outThree[_setAllPos], color);
        strip.show();
        _setAllPos++;
      }
      else
      {
        _setAllPos = sizeof(_inOne);
        _setAllLocation++;
      }
      break;
    case 1:
      if( _setAllPos > 0)
      {
        _setAllPos--;
        strip.setPixelColor(_inOne[_setAllPos], color);
        strip.setPixelColor(_inTwo[_setAllPos], color);
        strip.setPixelColor(_inThree[_setAllPos], color);
        strip.show();
        
      }
      else
      {
        _setAllLocation++;
        _setAllPos = 0;
      }
      break;
    case 2:
      _Delay = 5000;
      _setAllLocation++;
      _setAllPos = 0;
      break;
    case 3:
      break;
  }
}
void Lights::setBrightness(uint8_t brightness)
{
  strip.setBrightness(brightness);
  strip.show();
}
void Lights::rainbowCycle()
{
  if (_rainbowLocation < (_rainbowWheelInputSize - 1))
  {
    for (int pixNum = 1; pixNum < STRIPLENGTH; pixNum++) //Set color and increment based on position
    {
      strip.setPixelColor(pixNum, Wheel(_rainbowWheelInput[(pixNum + _rainbowLocation) % (_rainbowWheelInputSize - 2)]));
    }
    strip.show();
    _rainbowLocation++;
  }
  else
  {
    _rainbowLocation = 0;
  }

}
void Lights::rainbowAllOutside(uint8_t r, uint8_t g, uint8_t b) //All colors present at once. Inside is set to custom color
{
  setInside(r, g, b);
  if (_rainbowLocation < (_rainbowWheelInputSize - 1))//rainbow offset
  {
    for (int pixNum = 0; pixNum < sizeof(_outsideC); pixNum++) //Set color for each pix
    {
      strip.setPixelColor(_outsideC[pixNum], Wheel(_rainbowWheelInput[((pixNum * 391 / STRIPLENGTH + _rainbowLocation) % (_rainbowWheelInputSize - 2))]));
    }
    strip.show();
    _rainbowLocation++;
  }
  else
  {
    _rainbowLocation = 0;
  }
}
uint32_t Lights::Wheel(byte WheelPos)
{
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
Lights lights;


void HeartBeat::recievedHeartbeat()
{
  _HeartBeatRecieved = true;
}
void HeartBeat::processHeartbeat()
{
  if (!_HeartBeatRecieved && !_LastHeartBeatRecieved)
  {
    lights.setNextState(1, 1, lights.valueBrightness(), 0, 0, 0, 1000);
    lights.newCommand = true;
  }

  _LastHeartBeatRecieved = _HeartBeatRecieved;
  _HeartBeatRecieved = false;
}
int HeartBeat::setup()
{
  return 1;
}
int HeartBeat::loop()
{
  if ((millis() - _LastRun) >= _Delay)
  {
    processHeartbeat();
    _LastRun = millis();
  }
  return 1;
}



HeartBeat heartBeat;

void startOTA() 
{ // Start the OTA service
  delay(5000); //Delay before connecting to wifi to start ota only if STA
  //ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(OTAName);
  //ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() 
  {
    lights.setNextState(0);
  });

  ArduinoOTA.begin();
}

void setup() 
{

  //********** CHANGE PIN FUNCTION  TO GPIO **********
  //GPIO 1 (TX) swap the pin to a GPIO.
  pinMode(1, FUNCTION_3); 
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(3, FUNCTION_3); 
  //**************************************************
  lights.setup();

/*
  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet, dns);
  WiFi.begin(APSSID,APPSK); //Connect to WiFi
*/
  //WiFi.begin("W04-006OAKPL","A246801234567890#"); //Connect to WiFi
  
  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password); //Start AP
  

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

// Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/set", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    String onoff = "0";
    String modee = "0";
    String brightness = "0";
    String red = "0";
    String green = "0";
    String blue = "0";
    String delayy = "0";

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_ONOFF) && request->hasParam(PARAM_MODE) && request->hasParam(PARAM_BRIGHTNESS) && request->hasParam(PARAM_RED) && request->hasParam(PARAM_GREEN) && request->hasParam(PARAM_BLUE) && request->hasParam(PARAM_DELAY)) 
    {
      onoff = request->getParam(PARAM_ONOFF)->value();
      if (onoff.toInt() == 3)
      {
        heartBeat.recievedHeartbeat();
      }
      else
      {     
        modee = request->getParam(PARAM_MODE)->value();
        brightness = request->getParam(PARAM_BRIGHTNESS)->value();
        red = request->getParam(PARAM_RED)->value();
        green = request->getParam(PARAM_GREEN)->value();
        blue = request->getParam(PARAM_BLUE)->value();
        delayy = request->getParam(PARAM_DELAY)->value();
        
        
        lights.setNextState(onoff.toInt(),modee.toInt(),brightness.toInt(),red.toInt(),green.toInt(),blue.toInt(),delayy.toInt());
        lights.newCommand = true;
      }


      
      request->send(200, "text/plain", "OK");
    }
    else
    {
      request->send(200, "text/plain", "ERROR");
    }

    
  });

  server.on("/value", HTTP_GET, [](AsyncWebServerRequest *request)
  {

    if (request->hasParam(PARAM_ALL))
    {
      String All = String(lights.valueOnOff()) + "&" + String(lights.valueMode()) + "&" + String(lights.valueBrightness()) + "&" + String(lights.valueRed()) + "&" + String(lights.valueGreen()) + "&" + String(lights.valueBlue()) + "&" + String(lights.valueDelay());
      request->send_P(200, "text/plain", All.c_str());
    }

    else if (request->hasParam(PARAM_ONOFF))
    {
      request->send_P(200, "text/plain", String(lights.valueOnOff()).c_str());
    }
    else if (request->hasParam(PARAM_MODE))
    {
      request->send_P(200, "text/plain", String(lights.valueMode()).c_str());
    }
    else if (request->hasParam(PARAM_BRIGHTNESS))
    {
      request->send_P(200, "text/plain", String(lights.valueBrightness()).c_str());
    }
    else if (request->hasParam(PARAM_RED))
    {
      request->send_P(200, "text/plain", String(lights.valueRed()).c_str());
    }
    else if (request->hasParam(PARAM_GREEN))
    {
      request->send_P(200, "text/plain", String(lights.valueGreen()).c_str());
    }
    else if (request->hasParam(PARAM_BLUE))
    {
      request->send_P(200, "text/plain", String(lights.valueBlue()).c_str());
    }
    else if (request->hasParam(PARAM_DELAY))
    {
      request->send_P(200, "text/plain", String(lights.valueDelay()).c_str());
    }
    else
    {
      request->send(200, "text/plain", "ERROR");
    }
    
  });


  
  // Start server
  server.begin();
  startOTA();
  //lights.setNextState(0);
  heartBeat.setup();
  
}

void loop() 
{
  heartBeat.loop();//nned to be first for bootup
  lights.loop();
  ArduinoOTA.handle();                        // listen for OTA events
}
