///ATmega328P Old Bootloader Nano
//***************************************************************
// UPDATE LOG | Date - Comment
//
// 12/3/19 - Inital file created. Seperated from developing and stable
// 
// 
//***************************************************************

//MySensors
#define MY_RADIO_NRF24
#define MY_RF24_PA_LEVEL				RF24_PA_LOW
//#define MY_DEBUG
#define MY_RF24_CHANNEL					50
// Enable serial gateway
#define MY_GATEWAY_SERIAL
//#define MY_TRANSPORT_WAIT_READY_MS	1	//Ignore attempt to find parent

#include <EEPROM.h>

//----------------------------------------------------------------------------------
// MY SENSORS INCLUDES
//----------------------------------------------------------------------------------
#include <Adafruit_NeoPixel.h>
#include <MyConfig.h>
#include <MySensors.h>
//----------------------------------------------------------------------------------
// PIN USAGE HERE SO WE DONT DUPLICATE USAGE
//----------------------------------------------------------------------------------
// ARDUINO PINS FOR MYSENSORS
#define PIN_RF24_IRQ                     2  // radio Interrupt Request (DO NOT CHANGE))
#define PIN_RF24_CE                      9  // Chip Enable             (5 on Pro, 9  on UNO/Mini,  8 on MEGA
#define PIN_RF24_CSN_SS                 10  // Serial Select (Slave)   (6 on Pro, 10 on UNO/Mini, 53 on MEGA)
#define PIN_SPI_SDI_MOSI                11  // Master Out / Slave In   
#define PIN_SPI_SDO_MISO                12  // Master In / Slave Out   
#define PIN_SPI_SCK_CS                  13  // Serial Clock            

#define PIN_ONOFFBUTTON					5
#define PIN_MODESELECTBUTTON			8
#define PIN_LEDONOFF					6
#define PIN_LEDMODE						7
#define PIN_BRIGHTNESSSWITCH			4

#define PIN_NEOPIXEL					3
#define STRIPLENGTH						1
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

private:
	//void confirmMessage();
	void sendMsgInBuffer();
	void resendMsg();

	char _SendBuffer[BUFFERSIZE];
	char _SentMessage[BUFFERSIZE];

	boolean _NewMessageToSend = true; //Send start msg
	boolean _MsgRecieveSameAsMsgSent = false; //Defualt to send initally

	unsigned long _LastRun = 0;
	int _Delay = 300;

	unsigned long _SameLastRun = 0;
	int _SameMsgDelay = 10 * (1000); //10 sec
	unsigned long _DiffLastRun = 0;
	int _DiffMsgDelay = 2 * (1000);  //2 sec
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
	uint16_t _Delay = 800;  //Read sensors
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

	int storageAddress = 0; //0 - 1024 addresses for arduino Nano. use EEPROM.length() to find length

	//Varables
	//uint8_t _CurrentValue = 1;
	uint8_t _LastValue = 0;
	uint8_t _ModeCounter = 0;
	unsigned long _LastRun = 0;
	uint16_t _Delay = 200; //Read sensors
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
	uint16_t _Delay = 800;
	uint8_t _CurrentValue = 0;
	uint8_t _LastValue = 99;
	unsigned long _LastRun = 0;
};
class Lights //Identical to node.ino light class
{
public:
	int setup();
	int loop();
	void setNextState(int onOff, int mode, int brightness, int red, int green, int blue, int delay);
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


	uint8_t _MaxBrightness = 254;
	uint8_t _MinBrightness = 50;

	uint8_t _CurrentValue = 0;
	uint8_t _LastValue = 0;
	uint8_t _ModeCounter = 0;
	unsigned long _LastRun = 0;
	uint16_t _Delay = 800;  //Read sensors
};

MyMessage msg;
Radio radio;
OnOffButton onOffButton;
ModeButton modeButton;
BrightnessSwitch brightnessSwitch;
Lights lights;
Process process;

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

	msg.set(_SendBuffer);
	msg.setDestination(1);
	send(msg, true);
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

		switch (modeButtonMode) //State switch depending on mode //Change num of states varable if num of states is changed
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

	if (readLastMode() != modeButtonMode)  //If last mode and current mode differnt, save to EEProm
	{
		writeCurrentMode((byte)modeButtonMode);
	}
	
	//Serial.println(brightnessSwitch.getOnOff());
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
		case 1: //Rainbow Cycle
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
void Lights::setBrightness(uint8_t brightness)
{
	strip.setBrightness(brightness);
	//strip.show();
}
void Lights::rainbow()
{
	if (_rainbowLocation < (_rainbowWheelInputSize - 1))//rainbow offset
	{
		for (int pixNum = 0; pixNum < STRIPLENGTH; pixNum++) //Set color for each pix
		{
			strip.setPixelColor(pixNum, Wheel(_rainbowWheelInput[((pixNum * 391 / STRIPLENGTH + _rainbowLocation) % (_rainbowWheelInputSize - 2))]));
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
	pinMode(PIN_LEDONOFF, OUTPUT);
	pinMode(PIN_LEDMODE, OUTPUT);
	onOffButton.setup();
	modeButton.setup();
	brightnessSwitch.setup();
	radio.setup();
	lights.setup();
	process.setup();
	Serial.begin(115000);
}

void loop()
{
	brightnessSwitch.loop();
	modeButton.loop();
	onOffButton.loop();
	radio.loop();
	lights.loop();
	process.loop();
}

void receive(const MyMessage& message)
{
	Serial.println("****RECIEVED****");
	char recievedData[BUFFERSIZE];
	strncpy(recievedData, message.data, BUFFERSIZE); //Copy msg into buffer
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


	if (brightness == process.getMinBrightness())
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
	lights.setNextState(onOff, mode, brightness, red, green, blue, delay);



	//Serial.println("****RECIEVED***");
	//Serial.println(onOff);
	//Serial.println(mode);
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