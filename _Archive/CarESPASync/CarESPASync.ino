#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <ArduinoOTA.h>
#include "index.h"




//Web socket: https://tttapa.github.io/ESP8266/Chap14%20-%20WebSocket.html

//#ifndef APSSID
#define APSSID "Im a Router Morty! Im WiFi RICK"
#define APPSK  "thereisnopassword"
//#endif


IPAddress local_IP(172,20,48,24);
IPAddress gateway(172,20,48,1);
IPAddress subnet(255,255,255,0);


//Define Pin Usage
#define PIN_RASPI_MOSFT_POWER 0 //High during upload. Ground to load bootloader. 
#define PIN_CAR_IGNITION 3 //RX Dosent go high on boot.
#define PIN_RASPI_SHUTDOWN_DETECT 1 //TX Dosent go high on boot.
#define PIN_RASPI_TRIGGER_SHUTDOWN 2 //LED //High during upload

bool Status_RaspiPowered = false;
bool Status_Ignition = false;
bool Status_ShutdownTriggered = false;
bool Status_ShutdownDetected = false;

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

const char *OTAName = "caresp";           // A name and a password for the OTA service
const char *OTAPassword = "esp8266";

AsyncWebServer server(80);


String ConvertToPlaneVaue(bool value, String trueVal, String falseVal)
{
  if (value)
  {
    return trueVal;
  }
  else
  {
    return falseVal;
  }

  
  return "Error";
}

void startOTA() 
{ // Start the OTA service
  //delay(5000); //Delay before connecting to wifi to start ota
  //ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname(OTAName);
  //ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.begin();
}

void notFound(AsyncWebServerRequest *request) 
{
  request->send(404, "text/plain", "404 Not found");
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
  digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, HIGH); //High is off/ Low is on

  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_AP);

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid, password); //Start AP
  //WiFi.begin("W04-006OAKPL","A246801234567890#"); //Connect to WiFi

  

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  
  server.on("/ignitionStatus", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(ConvertToPlaneVaue(Status_Ignition,"On", "Off")).c_str());
  });
  server.on("/raspiStatus", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(ConvertToPlaneVaue(Status_RaspiPowered,"On", "Off")).c_str());
  });
  server.on("/raspiShutdown", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(ConvertToPlaneVaue(Status_ShutdownTriggered,"Triggered", "Not Triggered")).c_str());
  });
  server.on("/raspiShutdownDetected", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(ConvertToPlaneVaue(Status_ShutdownDetected,"Detected", "Not Detected")).c_str());
  });

  /*
  server.on("/clientlist", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(clientStatus()).c_str());
  });
  */
  
  // Start server
  server.begin();
  startOTA();
}

String clientStatus() //Testing
{
  struct station_info *stat_info;
  struct ip4_addr *IPaddress;
  stat_info = wifi_softap_get_station_info();
  
  int ipsize = sizeof(stat_info);

  IPAddress address;
  String listOfIpAddress = "hiro";

  for (int i = 0; i<=ipsize; i++)
  {
    IPaddress = &stat_info->ip;    
    address = IPaddress->addr;
    listOfIpAddress += address.toString();
    stat_info = STAILQ_NEXT(stat_info, next);
  }
  

  return listOfIpAddress;

}

unsigned long _LastRun_ReadStatus = 0;
uint16_t _Delay_ReadStatus = 500;
void Loop_ReadStatus() //Read ShutdownDetected and IgnitionStatus every 500ms.
{
  if ((millis() - _LastRun_ReadStatus) >= _Delay_ReadStatus)
  {
  
    Status_ShutdownDetected = digitalRead(PIN_RASPI_SHUTDOWN_DETECT);
    Status_Ignition = digitalRead(PIN_CAR_IGNITION);

    _LastRun_ReadStatus = millis();
  }
}

bool RequestShutdown = false;
unsigned long _LastRun_TurnOffPi = 0;
uint16_t _Delay_TurnOffPi = 10000;
void TurnOffPi()
{
  if ((millis() - _LastRun_TurnOffPi) >= _Delay_TurnOffPi)
  { 
    Status_ShutdownTriggered = true;
  
    if (Status_ShutdownDetected)
    {
      digitalWrite(PIN_RASPI_MOSFT_POWER, LOW);
      digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, HIGH);
      RequestShutdown = false;
      Status_ShutdownTriggered = false;
      Status_RaspiPowered = false;
      _LastRun_TurnOffPi = 0;
    }
    else
    {
      digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, HIGH); //Request pi shutdown
      delay(2000);
      digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, LOW); //Request pi shutdown 
    }

    _LastRun_TurnOffPi = millis();
  }
}


unsigned long _LastRun_TurnOffRaspiPower = 0;
uint16_t _Delay_TurnOffRaspiPower = 500;
void Loop_TurnOffRaspiPower()
{
  if ((millis() - _LastRun_TurnOffRaspiPower) >= _Delay_TurnOffRaspiPower)
  {
    if (RequestShutdown)
    {

      TurnOffPi();

    }
  
  
  _LastRun_TurnOffRaspiPower = millis();
  }
}


bool RequestTurnOn = false;
void TurnOnPi()
{
  digitalWrite(PIN_RASPI_MOSFT_POWER, HIGH);
  digitalWrite(PIN_RASPI_TRIGGER_SHUTDOWN, HIGH);
  Status_RaspiPowered = true;  
  RequestTurnOn = false;
}


unsigned long _LastRun_TurnOnRaspiPower = 0;
uint16_t _Delay_TurnOnRaspiPower = 500;
void Loop_TurnOnRaspiPower()
{
  if ((millis() - _LastRun_TurnOnRaspiPower) >= _Delay_TurnOnRaspiPower)
  {
    if (RequestTurnOn && !RequestShutdown)
    {
  
      TurnOnPi();
    
    }
   
   
   _LastRun_TurnOnRaspiPower = millis();
  }
}

unsigned long _LastRun_LogicControl = 0;
uint16_t _Delay_LogicControl = 500;
void Loop_LogicControl()
{
  if ((millis() - _LastRun_LogicControl) >= _Delay_LogicControl)
  {
    if (Status_Ignition && !Status_RaspiPowered)  //If ignition on and raspi not powered. turn on
    {
      RequestTurnOn = true;
    }
    else if (!Status_Ignition && Status_RaspiPowered) //If ignition on and raspi powered. turn off
    {
      RequestShutdown = true;
    }


    _LastRun_LogicControl = millis();
  }
}




void loop() 
{
  Loop_ReadStatus();
  Loop_LogicControl();
  Loop_TurnOffRaspiPower();
  Loop_TurnOnRaspiPower();
  ArduinoOTA.handle();                        // listen for OTA events
}
