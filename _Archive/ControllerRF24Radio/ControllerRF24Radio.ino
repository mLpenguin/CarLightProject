///ATmega328P Old Bootloader Nano
//***************************************************************
// UPDATE LOG | Date - Comment
//
// 12/23/19 - Inital log created. 
// 
//***************************************************************
#define RF24_CHANNEL					50
#define PIN_RF24_CE                      9  // Chip Enable             (5 on Pro, 9  on UNO/Mini,  8 on MEGA
#define PIN_SPI_SCK_CS                  13  // Serial Clock  

#define RADIO_TYPE						0	//1 = Node address, 0 = Controller adddress

#include <EEPROM.h>
#include <Adafruit_NeoPixel.h>

//----------------------------------------------------------------------------------
// Radio
//----------------------------------------------------------------------------------
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//----------------------------------------------------------------------------------
// PIN USAGE HERE SO WE DONT DUPLICATE USAGE
//----------------------------------------------------------------------------------
// ARDUINO PINS FOR MYSENSORS
/*
#define PIN_RF24_IRQ                     2  // radio Interrupt Request (DO NOT CHANGE))
#define PIN_RF24_CE                      9  // Chip Enable             (5 on Pro, 9  on UNO/Mini,  8 on MEGA
#define PIN_RF24_CSN_SS                 10  // Serial Select (Slave)   (6 on Pro, 10 on UNO/Mini, 53 on MEGA)
#define PIN_SPI_SDI_MOSI                11  // Master Out / Slave In
#define PIN_SPI_SDO_MISO                12  // Master In / Slave Out
#define PIN_SPI_SCK_CS                  13  // Serial Clock
*/

#define PIN_ONOFFBUTTON					5
#define PIN_MODESELECTBUTTON			8
#define PIN_LEDONOFF					6
#define PIN_LEDMODE						7
#define PIN_BRIGHTNESSSWITCH			4

#define PIN_NEOPIXEL					3
#define STRIPLENGTH						2
#define NUMOFSTATES						4

#define CONTROLLERLIGHTMINBRIGHTNESS	20
#define CONTROLLERLIGHTMAXBRIGHTNESS	150

#define BUFFERSIZE						8

#define EEPROMADDRESS					0 //EEPROM.length() get eeprom address range
//Address usage log:
//0 - 11/28/19

class Radio
{
public:
	int setup();
	int loop();
	void createMsg(uint8_t onOff, uint8_t mode, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int delay);
	boolean isSameMsgAsSent(char* check);
	void receiveMsg(char* message);


private:
	//void confirmMessage();
	void sendMsgInBuffer();
	void resendMsg();

	char _SendBuffer[BUFFERSIZE];
	char _SentMessage[BUFFERSIZE];

	boolean _NewMessageToSend = true; //Send start msg
	boolean _MsgRecieveSameAsMsgSent = false; //Defualt to send initally

	unsigned long _LastRun = 0;
	const unsigned int _Delay = 800;

	unsigned long _SameLastRun = 0;
	const unsigned int _SameMsgDelay = 10 * (1000); //10 sec
	unsigned long _DiffLastRun = 0;
	const unsigned int _DiffMsgDelay = 2 * (1000);  //2 sec
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
	const uint16_t _Delay = 800;  //Read sensors
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

	//Varable
	boolean _pendingMessage = true;

private:
	void Read();

	const int storageAddress = 0; //0 - 1024 addresses for arduino Nano. use EEPROM.length() to find length

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
	const uint16_t _Delay = 800;
	uint8_t _CurrentValue = 0;
	uint8_t _LastValue = 99;
	unsigned long _LastRun = 0;
};
class LightParent //Sets up neopixel strip
{
public:

	void setup();


	void setLights(uint8_t startpix, uint8_t striplength, uint8_t onOff, uint8_t mode, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue);
	void setAll(uint8_t startpix, uint8_t striplength, uint8_t r, uint8_t g, uint8_t b);
	void setBrightness(uint8_t brightness);
	void rainbow(uint8_t startpix, uint8_t striplength);
	uint32_t Wheel(byte WheelPos);

	Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLENGTH, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

private:


	uint16_t _rainbowWheelInput[392];
	uint16_t _rainbowWheelInputSize;
	uint16_t _rainbowLocation = 0;

};
class Lights //Not identical to node.ino light class
{
public:
	Lights(int start = 0, int length = STRIPLENGTH);
	int setup();
	int loop();
	void setNextState(int onOff, int mode, int brightness, int red, int green, int blue, int delay);
private:
	uint8_t _startPix;
	uint8_t _stripLength;

	//Varables
	uint8_t _OnOff = 0;
	uint8_t _Mode = 0;
	uint8_t _Brightness = 0;
	uint8_t _Red = 0;
	uint8_t _Green = 0;
	uint8_t _Blue = 0;

	unsigned long _LastRun = 0;
	uint16_t _Delay = 300;
};
class Process
{
public:
	int setup();
	int loop();

	int getMaxBrightness();
	int getMinBrightness();

	int readLastMode();
private:
	void setNextState();
	void writeCurrentMode(byte mode);

	const uint8_t _MaxBrightness = 254;
	const uint8_t _MinBrightness = 50;

	uint8_t _CurrentValue = 0;
	uint8_t _LastValue = 0;
	uint8_t _ModeCounter = 0;
	unsigned long _LastRun = 0;
	const uint16_t _Delay = 800;  //Read sensors
};
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

RF24Radio msg(RADIO_TYPE);
Radio radio;
OnOffButton onOffButton;
ModeButton modeButton;
BrightnessSwitch brightnessSwitch;
LightParent lightParent;
Lights controllerLights(0, 1); //Start at neopix 0, length 1
Lights nodeLights(1, 1); //Start at neopix 1, lenght 1
Process process;

RF24Radio::RF24Radio(int type)
{
	// Setup and configure rf radio

	rf24radio.begin();
	rf24radio.setChannel(RF24_CHANNEL);
	rf24radio.setAutoAck(1);                    // Ensure autoACK is enabled
	//radio.enableAckPayload();               // Allow optional ack payloads
	rf24radio.setRetries(0, 5);                 // Smallest time between retries, max no. of retries
	rf24radio.setPayloadSize(8);                // Here we are sending 8-byte payloads

	  // Set the PA Level low to prevent power supply related issues since this is a
 // getting_started sketch, and the likelihood of close proximity of the devices. RF24_PA_MAX is default.
	rf24radio.setPALevel(RF24_PA_LOW);


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
		radio.receiveMsg(recieved);
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

void LightParent::setup()
{
	pinMode(PIN_NEOPIXEL, OUTPUT);


	setAll(0, STRIPLENGTH, 0, 0, 0);
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

}
void LightParent::setAll(uint8_t startpix, uint8_t striplength, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t color = lightParent.strip.Color(r, g, b);
	for (uint16_t i = startpix; i < striplength; i++)
	{
		lightParent.strip.setPixelColor(i, color);
		lightParent.strip.show();
		delay(50);
	}
}
void LightParent::setBrightness(uint8_t brightness)
{
	lightParent.strip.setBrightness(brightness);
	//strip.show();
}
void LightParent::rainbow(uint8_t startpix, uint8_t striplength)
{
	if (_rainbowLocation < (_rainbowWheelInputSize - 1))//rainbow offset
	{
		for (int pixNum = startpix; pixNum < striplength; pixNum++) //Set color for each pix
		{
			lightParent.strip.setPixelColor(pixNum, Wheel(_rainbowWheelInput[((pixNum * 391 / striplength + _rainbowLocation) % (_rainbowWheelInputSize - 2))]));
		}
		lightParent.strip.show();
		_rainbowLocation++;
	}
	else
	{
		_rainbowLocation = 0;
	}
}
void LightParent::setLights(uint8_t startpix, uint8_t striplength, uint8_t onOff, uint8_t mode, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue)
{
	if (onOff == 1)
	{//Power on
		setBrightness(brightness);
		switch (mode)
		{
			//OnOff, Mode, Brightness, Red, Green, Blue, Delay
		case 0: //Solid Color
			setAll(startpix, striplength, red, green, blue);
			break;
		case 1: //Rainbow Cycle
			rainbow(startpix, striplength);
			break;
		}

	}
	else //Off
	{
		setAll(startpix, striplength, 0, 0, 0);
		setBrightness(0);
	}
}
uint32_t LightParent::Wheel(byte WheelPos) //Needed for rainbow
{
	WheelPos = 255 - WheelPos;
	if (WheelPos < 85) {
		return lightParent.strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	}
	if (WheelPos < 170) {
		WheelPos -= 85;
		return lightParent.strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
	WheelPos -= 170;
	return lightParent.strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}



int Radio::setup()
{
	//setNextState();
	return 1;
}
int Radio::loop()
{
	if ((millis() - _LastRun) >= _Delay)
	{
		if (_NewMessageToSend) //Var set in create msg
		{
			//Serial.println(_NewMessageToSend);
			sendMsgInBuffer(); //Sends msg in _SendBuffer when _NewMessageToSend is true
		}
		else
		{
			resendMsg(); //Resend prev msg
		}
		_LastRun = millis();
	}

	//Serial.println((millis() - _TimeMsgSent));
	//confirmMessage();
	/*
	if ((millis() - _LastMsgRecieved) > _LastMsgTimeout)
	{
		if (_TimeoutLastTime)
		{
			lights.setNextState(0, 0, 0, 0, 0, 0, 0); //Turn off controller light if no data has been recieved in 2 sec
		}
		else
		{
			_TimeoutLastTime = true;
		}
	}
	else
	{
		_TimeoutLastTime = false;
	}
	Serial.println(_TimeoutLastTime);
	*/
	//setNextState();
	//_TimeMsgSent = millis();

	return 1;
}
void Radio::createMsg(uint8_t onOff, uint8_t mode, uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue, int delay)
{
	_SendBuffer[0] = onOff + 1;
	_SendBuffer[1] = mode + 1;
	_SendBuffer[2] = brightness + 1;
	_SendBuffer[3] = red + 1;
	_SendBuffer[4] = green + 1;
	_SendBuffer[5] = blue + 1;
	delay++;
	memcpy(_SendBuffer + 6, &delay, sizeof(delay));
	_SendBuffer[8] = 0;

	/*
	for (int n = 0; n <= 9; n++)
	{
		Serial.print(_SentMessage[n], HEX);
	}
	Serial.println();
	for (int n = 0; n <= 9; n++)
	{
		Serial.print(_SendBuffer[n], HEX);
	}
	Serial.println();
	*/

	if (!strncmp(_SentMessage, _SendBuffer, BUFFERSIZE))
	{
		_NewMessageToSend = true;
		//Serial.println("NewMsg true");
	}
	else
	{
		_NewMessageToSend = false;
		//Serial.println("NewMsg False");
	}
}
void Radio::sendMsgInBuffer()
{

	Serial.println("***SENT****");

	//Serial.println(onOff);
	//Serial.println(mode);
	//Serial.println(brightness);
	//Serial.println(red);
	//Serial.println(green);
	//Serial.println(blue);
	//Serial.println(delay);
	//int val;
	//memcpy(&val, SendBuffer + 6, sizeof(val));
	//Serial.println(val);

	strncpy(_SentMessage, _SendBuffer, BUFFERSIZE); //Copy into _SentMessage to confirm message was sent

	msg.send(_SendBuffer, BUFFERSIZE);
	_NewMessageToSend = false;

}
boolean Radio::isSameMsgAsSent(char* check)
{
	if (strncmp(_SentMessage, check, BUFFERSIZE) == 0)
	{
		Serial.println("***RECEIVE SAME****");
		_MsgRecieveSameAsMsgSent = true; //Tell checker that the message was recieved as teh same
	}
	else
	{
		Serial.println("***RECEIVE NOT SAME****");
		_MsgRecieveSameAsMsgSent = false;
	}

	return _MsgRecieveSameAsMsgSent;
}
void Radio::resendMsg()
{
	if (_MsgRecieveSameAsMsgSent)//If msg recieved and msg sent the same, msg every 10 sec
	{
		if ((millis() - _SameLastRun) >= _SameMsgDelay)
		{

			sendMsgInBuffer();
			_SameLastRun = millis();

		}
	}
	else //else send every 2 sec. Msg differnt
	{
		if ((millis() - _DiffLastRun) >= _DiffMsgDelay)
		{

			sendMsgInBuffer();
			_DiffLastRun = millis();

		}
	}

}
void Radio::receiveMsg(char* message)
{
	Serial.println("****RECIEVED****");
	char recievedData[BUFFERSIZE];
	strncpy(recievedData, message, BUFFERSIZE); //Copy msg into buffer
	//Serial.println("*******");
	//Serial.println((int)recievedData[0]);
	int onOff = (byte)recievedData[0] - 1;
	int mode = (byte)recievedData[1] - 1;
	int brightness = (byte)recievedData[2] - 1;

	//Set power led and status led
	if (onOff == 1) { digitalWrite(PIN_LEDONOFF, HIGH); }
	if (onOff == 0) { digitalWrite(PIN_LEDONOFF, LOW); }

	if (brightness >= process.getMaxBrightness()) { digitalWrite(PIN_LEDMODE, HIGH); }
	else { digitalWrite(PIN_LEDMODE, LOW); }


	if (brightness == process.getMinBrightness()) //Set Controller neopixel brightness
	{
		brightness = CONTROLLERLIGHTMINBRIGHTNESS; //Hardcoded values for inside car
	}
	else if (brightness == process.getMaxBrightness())
	{
		brightness = CONTROLLERLIGHTMAXBRIGHTNESS; //Hardcoded values for inside car
	}

	int red = (byte)recievedData[3] - 1;
	int green = (byte)recievedData[4] - 1;
	int blue = (byte)recievedData[5] - 1;
	int delay;
	memcpy(&delay, recievedData + 6, sizeof(delay));
	delay--;

	radio.isSameMsgAsSent(recievedData);
	nodeLights.setNextState(onOff, mode, brightness, red, green, blue, delay);



	//Serial.println("****RECIEVED***");
	//Serial.println(onOff);
	//Serial.println(mode);
}

/*
void Radio::confirmMessage()
{
	if (!_SendConfirmed) //If sent message hasnt been confirmed
	{
		Serial.println("**TIMEOUT**");
		//int onOff = _SentMessage[0];
		//int mode = _SentMessage[1];
		//int brightness = _SentMessage[2];
		//int red = _SentMessage[3];
		//int green = _SentMessage[4];
		//int blue = _SentMessage[5];
		//int delay = _SentMessage[6];
		//sendMsg(onOff, mode, brightness, red, green, blue, delay);
		//msg.set(SendBuffer);
		//msg.setDestination(1);
		//send(msg, true);
		//_TimeMsgSent = millis();
		//lights.setLights()
	}
	//_TimeMsgSent = millis();
	setNextState();
	//Serial.println("**Resend**");
}
*/


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
		Serial.println("***ONOFF****");
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
	//_CurrentValue = EEPROM.read(storageAddress); //Read previous value
	//_LastValue = _CurrentValue = EEPROM.read(storageAddress); //Read previous value

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
		Serial.println("***MODE****");

	}
	//Serial.println(_ModeCounter);
	_LastValue = ModeValue;
}
int ModeButton::getMode()
{
	/*
	if (EEPROM.read(storageAddress) != modenumber)
	{
		Serial.println(modenumber);
		//EEPROM.write(storageAddress, modenumber);
		Serial.println("Wrote EEPROM");
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
		Serial.println("***BRIGHTNESS****");

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
	return (int)EEPROM.read(EEPROMADDRESS); //EEprom read returns byte. Cast into int so that others can use

}
void Process::writeCurrentMode(byte mode)
{
	//Serial.println("Write EEPROM");
	//Serial.println((int)mode);
	EEPROM.write(EEPROMADDRESS, mode); //Eeprom writes bytes
}
int Process::getMaxBrightness()
{
	return _MaxBrightness;
}
int Process::getMinBrightness()
{
	return _MinBrightness;
}

void Process::setNextState()
{

	//radio.setMessageRecieved(true);
	int modeButtonMode = modeButton.getMode();

	uint8_t onOff = onOffButton.getOnOff();
	uint8_t mode = 0;
	uint8_t brightness = 0;
	uint8_t r = 0;
	uint8_t g = 0;
	uint8_t b = 0;
	int delay = 0;

	if (onOff) //Turn on lights
	{
		if (brightnessSwitch.getOnOff())  //Set brightness
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
			mode = 1;
			r = 254;
			g = 254;
			b = 150;
			delay = 150;
			break;
		case 1: //White
			//Serial.println("White");
			mode = 0;
			r = 254;
			g = 254;
			b = 150;
			delay = 8000;
			break;
		case 2: //Purple
			mode = 0;
			r = 180;
			g = 0;
			b = 254;
			delay = 8000;
			break;
		case 3: //Blue
			mode = 0;
			r = 0;
			g = 0;
			b = 254;
			delay = 8000;
			break;
		}
	}
	else // Turn off lights
	{
		//Technically if onOff var is 0 all other vals are ignores. Below is not necessary
		mode = 0;
		r = 0;
		g = 0;
		b = 0;
		delay = 8000;
	}


	radio.createMsg(onOff, mode, brightness, r, g, b, delay);
	controllerLights.setNextState(onOff, mode, brightness, r, g, b, delay);

	if (readLastMode() != modeButtonMode)  //If last mode and current mode differnt, save to EEProm
	{
		writeCurrentMode((byte)modeButtonMode);
	}

	//Serial.println(brightnessSwitch.getOnOff());
}


Lights::Lights(int start = 0, int length = STRIPLENGTH)
{
	start = _startPix;
	length = _stripLength;
}
int Lights::setup()
{



	return 1;
}
int Lights::loop()
{
	if ((millis() - _LastRun) >= _Delay)
	{
		lightParent.setLights(_startPix, _stripLength, _OnOff, _Mode, _Brightness, _Red, _Green, _Blue);
		_LastRun = millis();

	}

	return 1;
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

	//setLights();
	_LastRun = millis();
}


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

void setup()
{
	Serial.begin(115000);
	Serial.println("Hi");
	pinMode(PIN_LEDONOFF, OUTPUT);
	pinMode(PIN_LEDMODE, OUTPUT);
	msg.setup();
	onOffButton.setup();
	modeButton.setup();
	brightnessSwitch.setup();
	radio.setup();
	lightParent.setup();
	controllerLights.setup();
	nodeLights.setup();
	process.setup();
	delay(random(100, 1001)); //Wait random time 100ms, 1s. This is done to prevent clocks of both from running at the same time.

}

void loop()
{
	msg.loop();
	brightnessSwitch.loop();
	modeButton.loop();
	onOffButton.loop();
	radio.loop();
	controllerLights.loop();
	nodeLights.loop();
	process.loop();
}

/*
if (onOffButton.CurrentValue == 1)
{
	_ModeCounter++;
	_SendBuffer[0] = onOffButton.CurrentValue;


	_msg.set(_SendBuffer);
	_msg.setDestination(1);
	send(_msg);
}*/