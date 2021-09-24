#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

#include <ArduinoOTA.h>

//#ifndef APSSID
#define APSSID "Im a Car Morty! Im Car RICK!"
#define APPSK  "thereisnopassword"
//#endif

/* Set these to your desired credentials. */
const char *OTAName = "cabincontrolleresp";           // A name and a password for the OTA servic

boolean USE_OTA = false;

IPAddress local_IP(1,2,3,2);
IPAddress gateway(1,2,3,0);
IPAddress subnet(255,255,255,0);


#define PIN_ONOFFBUTTON                2
#define PIN_MODESELECTBUTTON          0
#define PIN_BRIGHTNESSSWITCH          1 //TX

#define PIN_NEOPIXEL                  3 //RX
#define STRIPLENGTH                   2
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);


#define NUMOFSTATES                   4

#define CONTROLLERLIGHTMINBRIGHTNESS  20
#define CONTROLLERLIGHTMAXBRIGHTNESS  150

#define EEPROMADDRESS                 0 //EEPROM.length() get eeprom address range
                                        //Address usage log:
                                        //0 - 11/28/19

const String NODEIPADDRESS = "1.2.3.1"; //Light Node ip address //Has to be str const


#define PARAM_ONOFF         "of"
#define PARAM_MODE          "m"
#define PARAM_BRIGHTNESS    "br"
#define PARAM_RED           "r"
#define PARAM_GREEN         "g"
#define PARAM_BLUE          "b"
#define PARAM_DELAY         "d"
const char* PARAM_OTA = "enable";



int stripBrightness = 255; //force set in light.setBrightness()

void SetStripBrightness(int bright)
{
  stripBrightness = bright;
}



void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "404 Not found");
}



String sendHttp(uint8_t onOff, uint8_t m, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int d)
{
    
  HTTPClient http;  //Declare an object of class HTTPClient    
  //"http://"+
  String sendIpAddress = NODEIPADDRESS;
  String urlRequest = "http://"+sendIpAddress+"/set?"+PARAM_ONOFF+"="+String(onOff)+"&"+PARAM_MODE+"="+String(m)+"&"+PARAM_BRIGHTNESS+"="+String(brightness)+"&"+PARAM_RED+"="+String(red)+"&"+PARAM_GREEN+"="+String(green)+"&"+PARAM_BLUE+"="+String(blue)+"&"+PARAM_DELAY+"="+String(d);
  
  if (WiFi.status() == WL_CONNECTED) 
  { //Check WiFi connection status
    http.begin(urlRequest);  //Specify request destination
    //int httpCode = http.GET(); //Send the request
    http.GET(); //Send the request
    http.end();   //Close connection
  }
    
  return urlRequest;
}




class HeartBeat
{
public:
  int setup();
  int loop();
  void recievedHeartbeat();

  //Varables

private:
  void sendHeartbeat();
  
  void processHeartbeat();

  boolean _HeartBeatRecieved = true;
  boolean _LastHeartBeatRecieved = false;

  //Varables
  const uint16_t _Delay = 5 * 1000;  //Read sensors
  uint8_t _CurrentValue = 0;
  uint8_t _LastValue = 0;
  unsigned long _LastRun = 0;
};
class OnOffButton
{
public:
  int setup();
  int loop();
  int getOnOff();

  //Varables

private:
  void Read();

  //Varables
  const uint16_t _Delay = 400;  //Read sensors
  uint8_t _CurrentValue = 0;
  uint8_t _LastValue = 0;
  unsigned long _LastRun = 0;
};
class ModeButton
{
public:
  int setup();
  int loop();
  int getMode();

private:
  void Read();

  //Varables
  //uint8_t _CurrentValue = 1;
  uint8_t _LastValue = 0;
  uint8_t _ModeCounter = 0;
  unsigned long _LastRun = 0;
  const uint16_t _Delay = 200; //Read sensors
};
class BrightnessSwitch
{
public:
  int setup();
  int loop();
  int getOnOff();

  //Varables

private:
  void Read();

  //Varables
  const uint16_t _Delay = 400;
  uint8_t _CurrentValue = 1;
  uint8_t _LastValue = 1;
  unsigned long _LastRun = 0;
};
class Lights //Not identical to node.ino light class
{
public:
  int setup();
  int loop();
  Lights(uint8_t pixNum);
  void setNextState(uint8_t onOff, uint8_t m, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int d);
  void setModeLight(boolean value, int r, int g, int b);
private:
  void setLights();
  void setAll(uint8_t r, uint8_t g, uint8_t b);
  void setBrightness(uint8_t brightness);
  void rainbow();


  uint32_t Wheel(byte WheelPos);

  //Varables
  uint16_t _color = 200;

  uint8_t _OnOff = 0;
  uint8_t _Mode = 0;
  uint8_t _Brightness = 1;
  uint8_t _Red = 0;
  uint8_t _Green = 0;
  uint8_t _Blue = 0;


  uint8_t _PixNum = 0;

  uint16_t _rainbowWheelInput[392];
  uint16_t _rainbowWheelInputSize;
  uint16_t _rainbowLocation = 0;

  
  unsigned long _LastRun = 0;
  uint16_t _Delay = 200;
};
class Process
{
public:
  int setup();
  int loop();

  int getMaxBrightness();
  int getMinBrightness();

  int readLastMode();

  void setNewCommand();
  void setBrightness(uint8_t bright);

  
  uint8_t onOff = 0;
  uint8_t m = 0;
  uint8_t brightness = 0;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  int d = 0;


  uint8_t _Brightness = 1;

private:
  void setNextState();
  void writeCurrentMode(byte mode);

  boolean newCommand = true;

  

  String nodeIpAddress = NODEIPADDRESS;
  const uint8_t _MaxBrightness = 255;
  const uint8_t _MinBrightness = 50;

  uint8_t _CurrentValue = 0;
  uint8_t _LastValue = 0;
  uint8_t _ModeCounter = 0;
  unsigned long _LastRun = 0;
  const uint16_t _Delay = 400;  //Read sensors
};

HeartBeat heartBeat;
OnOffButton onOffButton;
ModeButton modeButton;
BrightnessSwitch brightnessSwitch;
Lights controllerLights(0);
Lights nodeLights(1);
Process process;

void HeartBeat::sendHeartbeat()
{
  sendHttp(process.onOff, process.m, process.brightness, process.r, process.g, process.b, process.d);
}
void HeartBeat::recievedHeartbeat()
{
  _HeartBeatRecieved = true;
}
void HeartBeat::processHeartbeat()
{
  if (!_HeartBeatRecieved && !_LastHeartBeatRecieved) //Missed 2 heartbeat messages turn off
  {
    nodeLights.setNextState(0, 1, stripBrightness, 0, 0, 0, 5000);
    //lights.newCommand = true;
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
  //Serial.println(millis() - _LastRun);
  if ((millis() - _LastRun) >= _Delay)
  {
    processHeartbeat();
    sendHeartbeat();
    _LastRun = millis();
  }
  
  return 1;
}



int OnOffButton::setup()
{
  pinMode(PIN_ONOFFBUTTON, INPUT_PULLUP);
  _CurrentValue = digitalRead(PIN_ONOFFBUTTON);
  return 1;
}
int OnOffButton::loop()
{
  if ((millis() - _LastRun) >= _Delay)
  {
    Read();
    _LastRun = millis();
  }

  return 1;
}
void OnOffButton::Read()
{
  int OnOffValue = digitalRead(PIN_ONOFFBUTTON);

  if (OnOffValue == _LastValue && _CurrentValue != OnOffValue)
  {
    //Do Work
    _CurrentValue = OnOffValue; // Set Values before read
    process.setNewCommand(); //Say new command recieved
  }

  _LastValue = OnOffValue;
}
int OnOffButton::getOnOff()
{
  return  _CurrentValue;
}

int ModeButton::setup()
{
  _ModeCounter = process.readLastMode();
  //_ModeCounter = 2;
  pinMode(PIN_MODESELECTBUTTON, INPUT_PULLUP);

  return 1;
}
int ModeButton::loop()
{
  //Serial.println(millis() - _LastRun);
  if ((millis() - _LastRun) >= _Delay)
  {
    Read();
    _LastRun = millis();
  }

  return 1;
}
void ModeButton::Read()
{
  int ModeValue = !digitalRead(PIN_MODESELECTBUTTON); //Default is on

  if (_LastValue == 1 && ModeValue == 0) //Only trigger when switch is relased
  {

    //Do Work
    _ModeCounter++;
    process.setNewCommand(); //Say new command recieved

  }
  //Serial.println(_ModeCounter);
  _LastValue = ModeValue;
}
int ModeButton::getMode()
{
  /*
  if (EEPROM.read(storageAddress) != modenumber)
  {
    EEPROM.write(storageAddress, modenumber);
  }
  */
  return  _ModeCounter % NUMOFSTATES; //Number of States in State machine in Lights Class //NUMOFSTATES Var defined at top
}

int BrightnessSwitch::setup()
{
  pinMode(PIN_BRIGHTNESSSWITCH, INPUT_PULLUP);
  _CurrentValue = digitalRead(PIN_BRIGHTNESSSWITCH);
  return 1;
}
int BrightnessSwitch::loop()
{
  if ((millis() - _LastRun) >= _Delay)
  {
    Read();
    _LastRun = millis();
  }

  return 1;
}
void BrightnessSwitch::Read()
{
  int BrightnessValue = digitalRead(PIN_BRIGHTNESSSWITCH);


  
  if (BrightnessValue == _LastValue && _CurrentValue != BrightnessValue)
  {

    //Do Work
    _CurrentValue = BrightnessValue; // Set Values before read
    //Serial.println("BRIGHTNESS");
    //Serial.println(_CurrentValue);
    process.setBrightness(_CurrentValue);
    process.setNewCommand(); //Say new command recieved

  }
  //Serial.println(OnOffValue);
  _LastValue = BrightnessValue;
}
int BrightnessSwitch::getOnOff()
{
  return  _CurrentValue;
}



int Process::setup()
{
  return 1;
}
int Process::loop()
{

  if ((millis() - _LastRun) >= _Delay) //check/poll sensor and create msg
  {

    setNextState();
    _LastRun = millis();

  }
  return 1;
}
int Process::readLastMode()
{
  //Serial.println("Read EEPROM");
  //Serial.println((int)EEPROM.read(EEPROMADDRESS));
  int m = int(EEPROM.read(EEPROMADDRESS)); //EEprom read returns byte. Cast into int so that others can use
  //yield(500);
  return m;

}
void Process::writeCurrentMode(byte m)
{
  //Serial.println("Write EEPROM");
  //Serial.println((int)mode);
  EEPROM.write(EEPROMADDRESS, m); //Eeprom writes bytes
  EEPROM.commit();
  //yield(100);
}
int Process::getMaxBrightness()
{
  return _MaxBrightness;
}
int Process::getMinBrightness()
{
  return _MinBrightness;
}
void Process::setNewCommand()
{
  newCommand = true;
}
void Process::setBrightness(uint8_t bright)
{
  
  _Brightness = bright;
}
void Process::setNextState()
{

  int modeButtonMode = modeButton.getMode();

  onOff = onOffButton.getOnOff();


  if (onOff) //Turn on lights
  {
    if (_Brightness == 1)  //Set brightness
    {
      brightness = _MaxBrightness;
    }
    else
    {
      brightness = _MinBrightness;
    }

    switch (modeButtonMode) //State switch depending on mode 
                //Change num of states varable if num of states is changed
    {
      //OnOff, Mode, Brightness, Red, Green, Blue, delay

    case 0: //Rainbow All - delay = 75, Rainbow Cycle - delay = 200
      m = 2;
      r = 0;
      g = 0;
      b = 0;
      d = 150;
      break;
    case 1: //White
      //Serial.println("White");
      m = 1;
      r = 254;
      g = 254;
      b = 150;
      d = 200;
      break;
    case 2: //Purple
      m = 1;
      r = 180;
      g = 0;
      b = 254;
      d = 200;
      break;
    case 3: //Blue
      m = 1;
      r = 0;
      g = 254;
      b = 0;
      d = 200;
      break;
    }
  }
  else // Turn off lights
  {
    //Technically if onOff var is 0 all other vals are ignores. Below is not necessary
    m = 0;
    r = 0;
    g = 0;
    b = 0;
    d = 8000;
  }

  if (newCommand)
  {
    
    sendHttp(onOff, m, brightness, r, g, b, d);
    if ((byte)readLastMode() != (byte)modeButtonMode)  //If last mode and current mode differnt, save to EEProm
    {
      writeCurrentMode((byte)modeButtonMode);
    }
    //writeCurrentMode((byte)modeButtonMode);
  }
 
  controllerLights.setNextState(onOff, m, stripBrightness, r, g, b, d);

  
  newCommand = false;

  //Serial.println(brightnessSwitch.getOnOff());
}


Lights::Lights(uint8_t pixNum)
{

  _PixNum = pixNum;
  
}
int Lights::setup()
{
  pinMode(PIN_NEOPIXEL, OUTPUT);
  setAll(0, 0, 0);
  //setBrightness(30);
  _rainbowWheelInputSize = (sizeof(_rainbowWheelInput) / sizeof(uint16_t));
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


  return 1;
}
int Lights::loop()
{
  if ((millis() - _LastRun) >= _Delay)
  {
    setLights();
    _LastRun = millis();

  }

  return 1;
}

void Lights::rainbow()
{
  int numPix = 1;
  if (_rainbowLocation < (_rainbowWheelInputSize - 1))//rainbow offset
  {

    strip.setPixelColor(_PixNum, Wheel(_rainbowWheelInput[((numPix * 391 / (STRIPLENGTH-1) + _rainbowLocation) % (_rainbowWheelInputSize - 2))]));
    
    strip.show();
    _rainbowLocation++;
  }
  else
  {
    _rainbowLocation = 0;
  }
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
      setAll(_Red, _Green, _Blue);
      break;
    case 1: //Solid Color
      setAll(_Red, _Green, _Blue);
      break;
    case 2: //Rainbow Cycle
      rainbow();
      break;
    case 3: //Rainbow Cycle
      rainbow();
      break;
    }

  }
  else //Off
  {
    setAll(0, 0, 0);
    setBrightness(0);
  }
}
void Lights::setNextState(uint8_t onOff, uint8_t m, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int d)
{
  _OnOff = onOff;
  _Mode = m;
  _Brightness = brightness;
  _Red = red;
  _Green = green;
  _Blue = blue;
  _Delay = d;

  setLights();
  _LastRun = millis();
}
void Lights::setAll(uint8_t r, uint8_t g, uint8_t b)
{
  uint32_t color = strip.Color(r, g, b);

  strip.setPixelColor(_PixNum, color);
  strip.show();

}
void Lights::setBrightness(uint8_t brightness)
{
  strip.setBrightness(stripBrightness); //Dont Care about waht it should be.
   
  //strip.show();
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





void startOTA() 
{ // Start the OTA service
  //delay(5000); //Delay before connecting to wifi to start ota only if STA
  //ArduinoOTA.setPort(8266);

  controllerLights.setNextState(0, 1, stripBrightness, 0, 0, 0, 5000);
  controllerLights.loop();
  nodeLights.setNextState(0, 1, stripBrightness, 0, 0, 0, 5000);
  nodeLights.loop();
  
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

AsyncWebServer server(80);
void setup()
{
  EEPROM.begin(4); //4 = size. min is  bytes
  //********** CHANGE PIN FUNCTION  TO GPIO **********
  //GPIO 1 (TX) swap the pin to a GPIO.
  pinMode(1, FUNCTION_3); 
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(3, FUNCTION_3); 
  //**************************************************

  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(APSSID,APPSK); //Connect to WiFi
  while (WiFi.waitForConnectResult() != WL_CONNECTED) 
  {
    //Serial.println("Connection Failed! Rebooting...");
    delay(10*1000);
    ESP.restart();
  }
  
  //WiFi.begin("W04-006OAKPL","A246801234567890#"); //Connect to WiFi

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", "This is not the website ur looking for");
  });
  server.on("/onoff", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", String(onOffButton.getOnOff()).c_str());
  });
  server.on("/bright", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send_P(200, "text/html", String(stripBrightness).c_str());
  });
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", String(modeButton.getMode()).c_str());
  });
  server.on("/test", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", String(stripBrightness).c_str());
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
      modee = request->getParam(PARAM_MODE)->value();
      brightness = request->getParam(PARAM_BRIGHTNESS)->value();
      red = request->getParam(PARAM_RED)->value();
      green = request->getParam(PARAM_GREEN)->value();
      blue = request->getParam(PARAM_BLUE)->value();
      delayy = request->getParam(PARAM_DELAY)->value();
      
      SetStripBrightness(brightness.toInt());
      //process.setNewCommand();
      
      heartBeat.recievedHeartbeat();
      nodeLights.setNextState(onoff.toInt(),modee.toInt(),stripBrightness,red.toInt(),green.toInt(),blue.toInt(),delayy.toInt());
      
      
      request->send_P(200, "text/html", "OK");
    }
  });


  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/time", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    String Bright;
    String Time;
    
    #define PARAM_TIME "TIME"
    #define PARAM_BRIGHT "BRIGHT"

    // GET input1 value on <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
    if (request->hasParam(PARAM_TIME) | request->hasParam(PARAM_BRIGHT))
    {

      if (request->hasParam(PARAM_TIME))
      {
        Time = request->getParam(PARAM_TIME)->value(); 
      }

      if (request->hasParam(PARAM_BRIGHT))
      {
        Bright = request->getParam(PARAM_BRIGHT)->value();

        process.setBrightness(Bright.toInt());
        process.setNewCommand(); //Say new command recieved
      }

      
      request->send_P(200, "text/html", "OK");
    }
    else
    {
      request->send_P(200, "text/html", "ERROR");
    }
  });

  //1.2.3.2/ota?enable=1
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

  server.on("/reset", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    request->send(200, "text/plain", "TRIGGER RESET OK");
    yield();
    ESP.restart();
  });



  server.begin();
  
  onOffButton.setup();
  modeButton.setup();
  brightnessSwitch.setup();
  controllerLights.setup();
  nodeLights.setup();
  process.setup();
  heartBeat.setup();
}



unsigned long previousWiFiMillis = 0;
int intervalWiFi = 10 * 1000;

void loop()
{
  if (USE_OTA)
  {
    ArduinoOTA.handle();                        // listen for OTA events
  }
  else
  {
    brightnessSwitch.loop();
    modeButton.loop();
    onOffButton.loop();
    controllerLights.loop();
    nodeLights.loop();
    process.loop();
    heartBeat.loop();
  }

  /*
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousWiFiMillis >=intervalWiFi)) 
  {
    ////ESP.restart();
    previousWiFiMillis = millis();
  }
  */
}
