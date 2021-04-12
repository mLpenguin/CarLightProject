#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>
#include "index.h"

//#ifndef APSSID
#define APSSID "Im a Router Morty! Im WiFi RICK"
#define APPSK  "1234567890"
//#endif

IPAddress local_IP(172,20,48,24);
IPAddress gateway(172,20,48,1);
IPAddress subnet(255,255,255,0);

//Define Pin Usage
#define PIN_RASPI_MOSFT_POWER 2
#define PIN_CAR_IGNITION 3 //RX
#define PIN_RASPI_SHUTDOWN_DETECT 0
#define PIN_RASPI_TRIGGER_SHUTDOWN 1 //TX

bool _RaspiPowered = false;
bool _IgnitionStatus = false;
bool _ShutdownTriggered = false;
bool _ShutdownDetected = false;

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

const char *OTAName = "CarESP";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";

ESP8266WebServer server(80);

void startOTA() 
{ // Start the OTA service
  //ArduinoOTA.setHostname(OTAName);
  //ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.begin();
}


void IndexWebpage(bool Ignition=false, bool RaspiStatus = false, index_html)
{

    // Replaces placeholder with DHT values
  String processor(const String& var)
  {
    //Serial.println(var);
    if(var == "TEMPERATURE")
    {
      return String(_IgnitionStatus);
    }
    else if(var == "HUMIDITY"){
      return String(_ShutdownTriggered);
    }
    return String();
  }
}


void handleRoot() 
{

 String s = index_html; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
 //server.send(200, "text/plain", "404: hehe");
}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void setup() 
{

  //********** CHANGE PIN FUNCTION  TO GPIO **********
  //GPIO 1 (TX) swap the pin to a GPIO.
  pinMode(1, FUNCTION_3); 
  //GPIO 3 (RX) swap the pin to a GPIO.
  pinMode(3, FUNCTION_3); 
  //**************************************************
  pinMode(PIN_RASPI_MOSFT_POWER, OUTPUT);
  pinMode(PIN_RASPI_TRIGGER_SHUTDOWN, OUTPUT);
  pinMode(PIN_RASPI_SHUTDOWN_DETECT, INPUT);
  pinMode(PIN_CAR_IGNITION, INPUT);

  digitalWrite(PIN_RASPI_MOSFT_POWER, LOW);
  digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, LOW);

  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);
  //WiFi.softAPConfig(local_IP, gateway, subnet);
  //WiFi.softAP(ssid, password);
  WiFi.begin("W04-006OAKPL","A246801234567890#"); 

  //IPAddress myIP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //server.on("/", handleRoot);

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server

  startOTA();

}

unsigned long _LastRun_ReadStatus = 0;
uint16_t _Delay_ReadStatus = 500;
void Loop_ReadStatus() //Read ShutdownDetected and IgnitionStatus every 500ms.
{
  if ((millis() - _LastRun_ReadStatus) >= _Delay_ReadStatus)
  {
  
  _ShutdownDetected = digitalRead(PIN_RASPI_SHUTDOWN_DETECT);
  _IgnitionStatus = digitalRead(PIN_CAR_IGNITION);

  _LastRun_ReadStatus = millis();
  }
}


bool _ShutdownRaspiPi = false;
unsigned long _LastRun_TurnOffRaspiPower = 0;
uint16_t _Delay_TurnOffRaspiPower = 200;
void Loop_TurnOffRaspiPower()
{
  if (_ShutdownRaspiPi)
  {
    if ((millis() - _LastRun_TurnOffRaspiPower) >= _Delay_TurnOffRaspiPower)
    {
      digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, HIGH); //Repeats
    
      if (_ShutdownDetected)
      {
        digitalWrite(PIN_RASPI_MOSFT_POWER, LOW);
        _ShutdownRaspiPi = false;
      }
      _LastRun_TurnOffRaspiPower = millis();
    }
  }
  
}


bool _TurnOnRaspiPi = false;
unsigned long _LastRun_TurnOnRaspiPower = 0;
uint16_t _Delay_TurnOnRaspiPower = 200;
void Loop_TurnOnRaspiPower()
{
  if (_TurnOnRaspiPi)
  {
    if ((millis() - _LastRun_TurnOnRaspiPower) >= _Delay_TurnOnRaspiPower)
    {
      digitalWrite(PIN_RASPI_MOSFT_POWER, HIGH);  
      _TurnOnRaspiPi = false;
      _LastRun_TurnOnRaspiPower = millis();
    }
  }
}


unsigned long _LastRun_LogicControl = 0;
uint16_t _Delay_LogicControl = 500;
void Loop_LogicControl()
{
  if ((millis() - _LastRun_LogicControl) >= _Delay_LogicControl)
  {
    if (_IgnitionStatus)
    {
      _TurnOnRaspiPi = true;
    }
    else
    {
      _ShutdownRaspiPi = true;
    }
  }



_LastRun_LogicControl = millis();
}




void loop() 
{
  //Loop_ReadStatus();
  //Loop_LogicControl();
  //Loop_TurnOffRaspiPower();
  //Loop_TurnOnRaspiPower();
  ArduinoOTA.handle();                        // listen for OTA events
  server.handleClient();
}
