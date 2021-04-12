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

IPAddress local_IP(1,2,3,2);
IPAddress gateway(1,2,3,0);
IPAddress subnet(255,255,255,0);


#define PIN_ONOFFBUTTON				        2
#define PIN_MODESELECTBUTTON	        0
#define PIN_BRIGHTNESSSWITCH	        1 //TX

#define PIN_NEOPIXEL					        3 //RX
#define STRIPLENGTH						        2
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);


#define NUMOFSTATES						        4

#define CONTROLLERLIGHTMINBRIGHTNESS	20
#define CONTROLLERLIGHTMAXBRIGHTNESS	150

#define EEPROMADDRESS				         	0 //EEPROM.length() get eeprom address range
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



uint8_t stripBrightness = 255;



void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "404 Not found");
}


HTTPClient http;  //Declare an object of class HTTPClient
String sendHttp(uint8_t onOff, uint8_t m, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int d)
{
  //"http://"+
  String sendIpAddress = NODEIPADDRESS;
  String urlRequest = "http://"+sendIpAddress+"/set?"+PARAM_ONOFF+"="+String(onOff)+"&"+PARAM_MODE+"="+String(m)+"&"+PARAM_BRIGHTNESS+"="+String(brightness)+"&"+PARAM_RED+"="+String(red)+"&"+PARAM_GREEN+"="+String(green)+"&"+PARAM_BLUE+"="+String(blue)+"&"+PARAM_DELAY+"="+String(d);
  http.begin(urlRequest);  //Specify request destination
  
  int httpCode = http.GET();                                                                  //Send the request
  http.end();   //Close connection
  return urlRequest;
}




class HeartBeat
{
public:
  int setup();
  int loop();

  //Varables

private:
  void sendHeartbeat();

  //Varables
  const uint16_t _Delay = 10 * 1000;  //Read sensors
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

private:
  void setNextState();
  void writeCurrentMode(byte mode);

  boolean newCommand = true;

  uint8_t _Brightness = 1;

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
int HeartBeat::setup()
{
  return 1;
}
int HeartBeat::loop()
{
  //Serial.println(millis() - _LastRun);
  if ((millis() - _LastRun) >= _Delay)
  {
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
		if (_Brightness)  //Set brightness
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
	  controllerLights.setNextState(onOff, m, stripBrightness, r, g, b, d);
    sendHttp(onOff, m, brightness, r, g, b, d);
    if ((byte)readLastMode() != (byte)modeButtonMode)  //If last mode and current mode differnt, save to EEProm
    {
      writeCurrentMode((byte)modeButtonMode);
    }
    //writeCurrentMode((byte)modeButtonMode);
  }
 


  
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
  strip.setBrightness(brightness);
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
  delay(5 *1000); //Delay before connecting to wifi to start ota only if STA
  //ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(OTAName);
  //ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.begin();
}

AsyncWebServer server(80);
void setup()
{
  EEPROM.begin(4);
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
    Serial.println("Connection Failed! Rebooting...");
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
  request->send_P(200, "text/html", String(brightnessSwitch.getOnOff()).c_str());
});
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", String(modeButton.getMode()).c_str());
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
      
      stripBrightness = brightness.toInt();
      
      nodeLights.setNextState(onoff.toInt(),modee.toInt(),stripBrightness,red.toInt(),green.toInt(),blue.toInt(),delayy.toInt());
      
      
      request->send_P(200, "text/html", "OK");
      //nodeLights.newCommand = true;
      //heartBeat.recievedHeartbeat();
    }
  });


  // Send a GET request to <ESP_IP>/update?output=<inputMessage1>&state=<inputMessage2>
  server.on("/rtc", HTTP_GET, [] (AsyncWebServerRequest *request) 
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

  server.on("/reset", HTTP_GET, [] (AsyncWebServerRequest *request) 
  {
    request->send(200, "text/plain", "TRIGGER RESET OK");
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
  startOTA();
}

void loop()
{
	brightnessSwitch.loop();
	modeButton.loop();
	onOffButton.loop();
  controllerLights.loop();
  nodeLights.loop();
	process.loop();
  heartBeat.loop();
  ArduinoOTA.handle();                        // listen for OTA events
}
