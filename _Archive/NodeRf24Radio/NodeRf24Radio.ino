///
//Brown wire from front control unit is GROUND
//Connect programmer board appropriately
///
///ATmega328P 5V Arduino Pro/Mini

//***************************************************************
// UPDATE LOG | Date - Comment
//
// 12/23/19 - Inital file created. 
// 
//***************************************************************


//----------------------------------------------------------------------------------
// Radio
//----------------------------------------------------------------------------------
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define RF24_CHANNEL					50
//#define MY_DEBUG


#include <Adafruit_NeoPixel.h>
//----------------------------------------------------------------------------------
// PIN USAGE HERE SO WE DONT DUPLICATE USAGE
//----------------------------------------------------------------------------------
#define RF24_CHANNEL					50
#define PIN_RF24_CE                      9  // Chip Enable             (5 on Pro, 9  on UNO/Mini,  8 on MEGA
#define PIN_SPI_SCK_CS                  13  // Serial Clock  

#define RADIO_TYPE						1	//1 = Node address, 0 = Controller adddress

#define PIN_NEOPIXEL					8
#define STRIPLENGTH						39

#define BUFFERSIZE						8




RF24 rf24radio(PIN_RF24_CE, PIN_SPI_SCK_CS); // CE, CSN
class RF24Radio
{
public:
	int setup();
	int loop();

	RF24Radio(int type);

	void send(char* pMessage, int iMaxSize);

private:
	char recieved[BUFFERSIZE];

	boolean read(char* pData, int iMaxSize);
	const uint64_t address[2] = { 0xF0F0F0F0AA, 0xF0F0F0F066 }; //Controller, Node
};
RF24Radio::RF24Radio(int type)
{
	// Setup and configure rf radio

	rf24radio.begin();

	rf24radio.setChannel(RF24_CHANNEL);

	rf24radio.setAutoAck(1);                    // Ensure autoACK is enabled
	//radio.enableAckPayload();               // Allow optional ack payloads
	rf24radio.setRetries(0, 5);                 // Smallest time between retries, max no. of retries
	rf24radio.setPayloadSize(8);                // Here we are sending 8-byte payloads

	if (type == 1)
	{
		rf24radio.openWritingPipe(address[1]);        // Both radios listen on the same pipes by default, and switch when writing
		rf24radio.openReadingPipe(1, address[0]);
	}
	else if (type == 0)
	{
		rf24radio.openWritingPipe(address[0]);        // Both radios listen on the same pipes by default, and switch when writing
		rf24radio.openReadingPipe(1, address[1]);
	}

	rf24radio.startListening();                 // Start listening
	//radio.printDetails();                 // Dump the configuration of the rf unit for debugging
}
int RF24Radio::setup()
{
	return 1;
}
int RF24Radio::loop()
{
	boolean recievedMessage = false;
	recievedMessage = read(recieved, BUFFERSIZE); //pass address
	if (recievedMessage)
	{
		receiveMsg(recieved);
	}
	return 1;
}
void RF24Radio::send(char* pMessage, int iMaxSize) //*pMessage = 1st value of message, pMessage = address
{
	rf24radio.stopListening();                                  // First, stop listening so we can talk.
	rf24radio.write(pMessage, iMaxSize);
	rf24radio.startListening();
}
boolean RF24Radio::read(char* pData, int iMaxSize)
{
	byte pipeNo; //Pipe recieved msg from
	//char recieved[BUFFERSIZE];                                       // Dump the payloads until we've gotten everything
	if (rf24radio.available(&pipeNo))
	{
		rf24radio.read(pData, iMaxSize);
		//radio.writeAckPayload(pipeNo, &recieved, 1);
		return true;
	}
	return false;
}


class Lights
{
public:
	int setup();
	int loop();



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

	uint8_t _OnOff = 0;
	uint8_t _Mode = 0;
	uint8_t _Brightness = 0;
	uint8_t _Red = 0;
	uint8_t _Green = 0;
	uint8_t _Blue = 0;

	uint8_t _outOne[8] = { 13, 14, 15, 16, 17, 18, 19, 0 };
	uint8_t _outTwo[8] = { 26, 27, 28, 29, 30, 31, 32, 0 };
	uint8_t _outThree[8] = { 6, 5, 4, 3, 2, 1, 0, 0 };
	uint8_t _outsideC[22] = { 13, 14, 15, 16, 17, 18, 19, 26, 27, 28, 29, 30, 31, 32, 6, 5, 4, 3, 2, 1, 0, 0 };

	uint8_t _inOne[7] = { 25, 24, 23, 22, 21, 20, 0 };
	uint8_t _inTwo[7] = { 38, 37, 36, 35, 34, 33, 0 };
	uint8_t _inThree[7] = { 7, 8, 9, 10, 11, 12, 0 };
	uint8_t _insideC[19] = { 25, 24, 23, 22, 21, 20, 38, 37, 36, 35, 34, 33, 7, 8, 9, 10, 11, 12, 0 };

	uint16_t _rainbowWheelInput[392];
	uint16_t _rainbowWheelInputSize;
	uint16_t _rainbowLocation = 0;

	Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
	unsigned long _LastRun = 0;
	uint16_t _Delay = 200;
};
int Lights::setup()
{

	_rainbowWheelInputSize = (sizeof(_rainbowWheelInput) / sizeof(uint16_t));

	pinMode(PIN_NEOPIXEL, OUTPUT);
	setAll(0, 0, 0);
	//setBrightness(30);

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
		case 1: //Rainbow Cycle
			rainbowCycle();
			break;
		case 2: //Rainbow All
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
}
void Lights::setNextState(int onOff, int mode, int brightness, int red, int green, int blue, int delay)
{
	_OnOff = onOff;
	_Mode = mode;
	_Brightness = brightness;
	_Red = red;
	_Green = green;
	_Blue = blue;
	_Delay = delay;
	setLights();
	_LastRun = millis();
}
void Lights::setAll(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = strip.Color(r, g, b);
	for (uint16_t i = 0; i < strip.numPixels(); i++)
	{
		strip.setPixelColor(i, color);
		strip.show();
		delay(50);
	}
}
void Lights::setOutside(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = strip.Color(r, g, b);
	for (uint16_t i = 0; i <= 20; i++)
	{
		strip.setPixelColor(_outsideC[i], color);
	}
	//strip.show();
}
void Lights::setInside(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = strip.Color(r, g, b);
	for (uint16_t i = 0; i <= 17; i++)
	{
		strip.setPixelColor(_insideC[i], color);
		//delay(50);
	}
	//strip.show();
}
void Lights::setBrightness(uint8_t brightness)
{
	strip.setBrightness(brightness);
	//strip.show();
}

void Lights::setAllone(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = strip.Color(r, g, b);
	for (uint16_t i = 0; i < 7; i++)
	{
		strip.setPixelColor(_outOne[i], color);
		strip.show();
		delay(75);
		//Serial.println(outOne[i]);
	}
	for (uint16_t i = 0; i < 7; i++)
	{
		strip.setPixelColor(_outTwo[i], color);
		strip.show();
		delay(75);
	}
	for (uint16_t i = 0; i < 7; i++)
	{
		strip.setPixelColor(_outThree[i], color);
		strip.show();
		delay(75);
	}

	int i = 2;
	strip.setPixelColor(_inOne[i], color);
	strip.setPixelColor(_inTwo[i], color);
	strip.setPixelColor(_inThree[i], color);
	i = 3;
	strip.setPixelColor(_inOne[i], color);
	strip.setPixelColor(_inTwo[i], color);
	strip.setPixelColor(_inThree[i], color);
	strip.show();

	delay(200);

	i = 4;
	strip.setPixelColor(_inOne[i], color);
	strip.setPixelColor(_inTwo[i], color);
	strip.setPixelColor(_inThree[i], color);
	i = 1;
	strip.setPixelColor(_inOne[i], color);
	strip.setPixelColor(_inTwo[i], color);
	strip.setPixelColor(_inThree[i], color);
	strip.show();

	delay(200);

	i = 0;
	strip.setPixelColor(_inOne[i], color);
	strip.setPixelColor(_inTwo[i], color);
	strip.setPixelColor(_inThree[i], color);
	i = 5;
	strip.setPixelColor(_inOne[i], color);
	strip.setPixelColor(_inTwo[i], color);
	strip.setPixelColor(_inThree[i], color);
	strip.show();
}
void Lights::setAlltwo(uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = strip.Color(r, g, b);
	for (uint16_t i = 0; i < ((sizeof(_outOne) / sizeof(int)) - 1); i++)
	{
		strip.setPixelColor(_outOne[i], color);
		strip.setPixelColor(_outTwo[i], color);
		strip.setPixelColor(_outThree[i], color);
		strip.show();
		delay(50);
	}
	for (uint16_t i = ((sizeof(_inOne) / sizeof(int)) - 1); i < 0; i--)
	{
		strip.setPixelColor(_inOne[i], color);
		strip.setPixelColor(_inTwo[i], color);
		strip.setPixelColor(_inThree[i], color);
		strip.show();
		delay(50);
	}
}

void Lights::rainbowCycle()
{
	if (_rainbowLocation < (_rainbowWheelInputSize - 1))
	{
		for (int pixNum = 0; pixNum < STRIPLENGTH; pixNum++) //Set color and increment based on position
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
		for (int pixNum = 0; pixNum < 20; pixNum++) //Set color for each pix
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
RF24Radio msg(RADIO_TYPE);

/*
class RadioHeartbeat
{
public:
	int setup();
	int loop();
	void setLastTimeHeartbeat();
private:
	void sendHBMsg();
	boolean isConnected();

	unsigned long _LastHBMsgRecieved = 0;
	int _timeoutDelay = 11000; //How long to wait before next msg before shutdown

	unsigned long _LastRun = 0;
	int _SendDelay = 5000; //Delay to send heartbeat interval
	uint32_t _SleepDelay = 1000; //Delay for sleep
};
int RadioHeartbeat::setup()
{

}
int RadioHeartbeat::loop()
{
	if ((millis() - _LastRun) >= _SendDelay) //Send Delay Task
	{
		sendHBMsg();
		_LastRun = millis();
	}

	if (!isConnected())//Go to low power mode if isConnected() is false
	{
		lights.setOnOff(0);
		Serial.println("***********");
		Serial.println("SLEEP");
		sleep(_SleepDelay);
		sendHBMsg();
		_LastHBMsgRecieved = millis();
		_LastRun = millis();
	}
	return 1;
}
void RadioHeartbeat::sendHBMsg()
{
	char SendBuffer[1];
	SendBuffer[0] = 2;

	msg.set(SendBuffer);
	msg.setDestination(0);
	send(msg);
	Serial.println("***********");
	Serial.println("Ack sent");
	//Serial.println((int)SendBuffer[0]);
}
void RadioHeartbeat::setLastTimeHeartbeat()
{
	_LastHBMsgRecieved = millis();
	_LastRun = millis();
}
boolean RadioHeartbeat::isConnected()
{
	if ((millis() - _LastHBMsgRecieved) > _timeoutDelay) { return false; }
	else { return true; }
}
RadioHeartbeat radioHeartbeat;
*/

/*
void OnOffButton::ReadModeSelectButton()
{
int value = digitalRead(PIN_MODESELECTBUTTON);
static int lastValue = 0;

if (value == lastValue)
{
_CurrentValue = value;
}

lastValue = value;

ReadOnOffButtonLastRun = millis();
}
*/


void receiveMsg(char* message)
{
	char recievedData[BUFFERSIZE];
	strncpy(recievedData, message, BUFFERSIZE); //Copy msg into buffer

	int onOff = (byte)recievedData[0] - 1;
	int mode = (byte)recievedData[1] - 1;
	int brightness = (byte)recievedData[2] - 1;
	int red = (byte)recievedData[3] - 1;
	int green = (byte)recievedData[4] - 1;
	int blue = (byte)recievedData[5] - 1;
	int delay;
	memcpy(&delay, recievedData + 6, sizeof(delay));
	delay--;
	//Serial.println((int)recievedData[3]);
	//Serial.println("****Recieved*****");
	//Serial.println((byte)recievedData[3]);
	//Serial.println((byte)recievedData[4]);

	msg.send(recievedData, BUFFERSIZE); //Send same message back

	Serial.println("***RECIEVED***");
	//Serial.println("Set Sent");
	//Serial.println(onOff);
	//Serial.println(mode);
	//Serial.println(brightness);
	//Serial.println(red);
	//Serial.println(green);
	//Serial.println(blue);
	//Serial.println(delay);
	lights.setNextState(onOff, mode, brightness, red, green, blue, delay);
	//Serial.println((byte)recievedData[7]);
}

/*
										 //Mode, Preset, Delay, Brightness, R, G, B, PIXNUM
light.execute = false; //New command recieved. Stop running code

										//Check if same as queue msg

if (taskLightChilds[child].q_mode != (byte)copyData[0]) { recievedNewCommand = true; }
if (taskLightChilds[child].q_preset != (byte)copyData[1]) { recievedNewCommand = true; }
if (taskLightChilds[child].q_lightDelay != (byte)copyData[2]) { recievedNewCommand = true; }
if (taskLightChilds[child].q_stripBrightness != (byte)copyData[3]) { recievedNewCommand = true; }

//If not same, set vars
if (recievedNewCommand == true)
{
	taskLightChilds[child].q_mode = (byte)copyData[0];
	taskLightChilds[child].q_preset = (byte)copyData[1];
	taskLightChilds[child].q_lightDelay = (byte)copyData[2];
	taskLightChilds[child].q_stripBrightness = (byte)copyData[3];

	taskLightChilds[child].execute = true;
	Serial.println(child);
}



copyData[0] = 0; //Reset
*/

void setup()
{
	lights.setup();
	msg.setup();


	Serial.begin(115200);
	//Serial.println("ON");
}
void loop()
{
	msg.loop();
	lights.loop();
}