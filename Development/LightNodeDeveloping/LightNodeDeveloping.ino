#include <Adafruit_NeoPixel.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ArduinoOTA.h>

#include "index.h"

const String CONTROLIPADDRESS = "1.2.3.2"; //Light controller ip address //Has to be str const

boolean USE_OTA = false;

//Web socket: https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html

//#ifndef APSSID
#define APSSID "Im a Car Morty! Im Car RICK!"
#define APPSK  "thereisnopassword"
//#endif

const char *ssid = APSSID;
const char *password = APPSK;

IPAddress local_IP(1,2,3,1);
IPAddress gateway(1,2,3,0);
IPAddress subnet(255,255,255,0);


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
const char* PARAM_OTA = "enable";


AsyncWebServer server(80);

void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "404 Not found");
}




class SendHttp
{
  public:
    int loop();
    void triggerSendHttp(uint8_t onOff, uint8_t m, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int d);

  private:

    HTTPClient http;  //Declare an object of class HTTPClient
    
    String sendHttp();

    boolean sendMsg = false;
    
    uint8_t _OnOff = 0;
    uint8_t _M = 0;
    uint8_t _Brightness = 0;
    uint8_t _Red = 0;
    uint8_t _Green = 0;
    uint8_t _Blue = 0;
    int _D = 1000;
  
};
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



  setNextState(1, 1, 255, 180, 0, 255, 1000);
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
  if (!_HeartBeatRecieved && !_LastHeartBeatRecieved) //Missed 2 heartbeat messages turn off
  {
    lights.setNextState(0, 1, lights.valueBrightness(), 0, 0, 0, 5000);
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



String SendHttp::sendHttp()
{
  //"http://"+
  String sendIpAddress = CONTROLIPADDRESS;
  String urlRequest = "http://"+sendIpAddress+"/set?"+PARAM_ONOFF+"="+String(_OnOff)+"&"+PARAM_MODE+"="+String(_M)+"&"+PARAM_BRIGHTNESS+"="+String(_Brightness)+"&"+PARAM_RED+"="+String(_Red)+"&"+PARAM_GREEN+"="+String(_Green)+"&"+PARAM_BLUE+"="+String(_Blue)+"&"+PARAM_DELAY+"="+String(_D);
  
  http.begin(urlRequest);  //Specify request destination
  //int httpCode = http.GET();  //Send the request
  http.GET(); //Send the request
  http.end();   //Close connection
  sendMsg = false;
  return urlRequest;
}

void SendHttp::triggerSendHttp(uint8_t onOff, uint8_t m, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int d)
{
  sendMsg = true;
  _OnOff = onOff;
  _M = m;
  _Brightness = brightness;
  _Red = red;
  _Green = green;
  _Blue = blue;
  _D = d;
  
}

int SendHttp::loop()
{
  if (sendMsg)
  {
    sendHttp();
  }
  return 1;
}

HeartBeat heartBeat;
SendHttp sendhttp;

void startOTA() 
{ // Start the OTA service
  //delay(5000); //Delay before connecting to wifi to start ota only if STA
  //ArduinoOTA.setPort(8266);

  lights.setNextState(0, 1, lights.valueBrightness(), 0, 0, 0, 5000);
  lights.newCommand = true;
  lights.loop();
  
  ArduinoOTA.setHostname(OTAName);
  //ArduinoOTA.setPassword(OTAPassword);

  /*
  ArduinoOTA.onStart([]() 
  {
    lights.setNextState(0);
  });
  */


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

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_ONOFF) && request->hasParam(PARAM_MODE) && request->hasParam(PARAM_BRIGHTNESS) && request->hasParam(PARAM_RED) && request->hasParam(PARAM_GREEN) && request->hasParam(PARAM_BLUE) && request->hasParam(PARAM_DELAY)) 
    {

      String sonoff = "0";
      String smodee = "0";
      String sbrightness = "0";
      String sred = "0";
      String sgreen = "0";
      String sblue = "0";
      String sdelayy = "0";
     
      sonoff = request->getParam(PARAM_ONOFF)->value();
      smodee = request->getParam(PARAM_MODE)->value();
      sbrightness = request->getParam(PARAM_BRIGHTNESS)->value();
      sred = request->getParam(PARAM_RED)->value();
      sgreen = request->getParam(PARAM_GREEN)->value();
      sblue = request->getParam(PARAM_BLUE)->value();
      sdelayy = request->getParam(PARAM_DELAY)->value();

      uint8_t onoff = sonoff.toInt();
      uint8_t modee = smodee.toInt();
      uint8_t brightness = sbrightness.toInt();
      uint8_t red = sred.toInt();
      uint8_t green = sgreen.toInt();
      uint8_t blue = sblue.toInt();
      uint8_t delayy = sdelayy.toInt();

      //sendHttp(onoff, modee, brightness, red, green, blue, delayy);
      //sendHttp(1, 1, 255, 255, 0, 180, 500);

      sendhttp.triggerSendHttp(onoff, modee, brightness, red, green, blue, delayy);

      if ((onoff == lights.valueOnOff()) && (modee == lights.valueMode()) && (brightness == lights.valueBrightness())&& (red == lights.valueRed()) && (green == lights.valueGreen()) && (blue == lights.valueBlue()) && (delayy == lights.valueDelay()))
      {
        heartBeat.recievedHeartbeat();
        request->send(200, "text/plain", "HEARTBEAT OK");
      }
      else
      { 
        lights.setNextState(onoff, modee, brightness, red, green, blue, delayy);
        lights.newCommand = true;
        request->send(200, "text/plain", "OK");
      }      
    }
    else
    {
      request->send(200, "text/plain", "ERROR");
    }
  });

  server.on("/reset", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    request->send(200, "text/plain", "TRIGGER RESET OK");
    yield();
    ESP.restart();
  });


  //1.2.3.1/ota?enable=1
  server.on("/ota", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    if (request->hasParam(PARAM_OTA))
    {
        USE_OTA=true;
        startOTA();
        request->send(200, "text/plain", "OTA ENABLED");
    }
    else
    {
      request->send(200, "text/plain", String(USE_OTA).c_str());
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
      request->send(200, "text/plain", "ERROR. REQUEST NOT FOUND");
    }
    
  });


  
  // Start server
  server.begin();
  //lights.setNextState(0);
  heartBeat.setup();
  
}




void loop() 
{
  if (USE_OTA)
  {
    ArduinoOTA.handle();  // listen for OTA events
  }
  else
  {
    heartBeat.loop();//nned to be first for bootup
    lights.loop();
    sendhttp.loop();
  }
}
