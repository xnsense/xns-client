// 
// 
// 

#include "xnsclientconfig.h"


bool xnsClientConfig::Save(int pAddress)
{
	int vAddress = pAddress;

	EEPROM.begin(EEPROM_SIZE);
	EEPROM.put(vAddress, EEPROM_CHECK_SUM);
	vAddress++;

	vAddress += SaveString(vAddress, ssid);
	vAddress += SaveString(vAddress, ssidPassword);
	vAddress += SaveString(vAddress, mqtt);
	vAddress += SaveInt(vAddress, mqttPort);
	vAddress += SaveString(vAddress, mqttClientID);
	vAddress += SaveString(vAddress, mqttPublishTopic);
	vAddress += SaveString(vAddress, mqttSubscribeTopic);
	
	if (IsSecure()) {
		vAddress += SaveBool(vAddress, true);
		vAddress += SaveString(vAddress, mqttUser);
		vAddress += SaveString(vAddress, mqttPass);
	}
	else
		vAddress += SaveBool(vAddress, false);

	bool vRet = EEPROM.commit();
	EEPROM.end();
	
	return vRet;
}


bool xnsClientConfig::Load(int pAddress)
{
	int vAddress = pAddress;
	bool vSuccess = false;

	EEPROM.begin(EEPROM_SIZE);
	if (EEPROM.read(vAddress) == EEPROM_CHECK_SUM)
	{
		vAddress++;

		vAddress += ReadString(vAddress, &ssid);
		vAddress += ReadString(vAddress, &ssidPassword);
		vAddress += ReadString(vAddress, &mqtt);
		vAddress += ReadInt(vAddress, &mqttPort);
		vAddress += ReadString(vAddress, &mqttClientID);
		vAddress += ReadString(vAddress, &mqttPublishTopic);
		vAddress += ReadString(vAddress, &mqttSubscribeTopic);
		
		bool secure = false;
		vAddress += ReadBool(vAddress, &secure);

		if (secure)
		{
			vAddress += ReadString(vAddress, &mqttUser);
			vAddress += ReadString(vAddress, &mqttPass);
		}
		else
		{
			mqttUser = 0;
			mqttPass = 0;
		}

		vSuccess = true;
	}
	else
	{
		ssid = (char*)String("").c_str();
		ssidPassword = (char*)String("").c_str();
		mqtt = (char*)String("").c_str();
		mqttClientID = (char*)String("").c_str();
		mqttPublishTopic = (char*)String("").c_str();
		mqttSubscribeTopic = (char*)String("").c_str();
		mqttUser = 0; 
		mqttPass = 0; 
		mqttPort = 1883;
	}
	EEPROM.end();
	return vSuccess;
}

bool xnsClientConfig::IsSecure()
{
	return (mqttUser != 0) && (String(mqttUser).length() > 0);
}

int xnsClientConfig::ReadInt(int pAddress, int *pValue) 
{
	int lower = EEPROM.read(pAddress);
	int higher = EEPROM.read(pAddress + 1);
	*pValue = lower + (higher << 8);
	return 2;
}
int xnsClientConfig::SaveInt(int pAddress, int pValue)
{
	byte lowByte = pValue & 0xFF;
	byte highByte = ((pValue >> 8) & 0xFF);

	EEPROM.write(pAddress, lowByte);
	EEPROM.write(pAddress + 1, highByte);

	return 2;
}

int xnsClientConfig::ReadBool(int pAddress, bool *pValue)
{
	byte y = EEPROM.read(pAddress);
	*pValue = (bool)y;
	//Serial.printf("Read bool as %#x [%s]\r\n", y, (*pValue ? "true" : "false"));
	return 1;
}

int xnsClientConfig::SaveBool(int pAddress, bool pValue)
{
	byte y = (byte)pValue;
	//Serial.printf("Writing bool as %#x [%s]\r\n", y, (pValue ? "true" : "false"));
	EEPROM.write(pAddress, y);
	return 1;
}
void xnsClientConfig::Print(Stream& serial)
{

	/*
	char* ssid;
	char* ssidPassword;
	char* mqtt;
	int mqttPort;
	char* mqttClientID;
	char* mqttPublishTopic;
	char* mqttSubscribeTopic;
	bool secure;
	char* mqttUser;
	char* mqttPass;
	*/

	serial.println("Configuration:");
	serial.println("-----------------------------------------------");
	serial.printf("ssid:                 %s\r\n", this->ssid);
	serial.printf("ssidPassword:         %s\r\n", this->ssidPassword);
	serial.printf("mqtt:                 %s\r\n", this->mqtt);
	serial.printf("mqttPort:             %i\r\n", this->mqttPort);
	serial.printf("mqttClientID:         %s\r\n", this->mqttClientID);
	serial.printf("mqttPublishTopic:     %s\r\n", this->mqttPublishTopic);
	serial.printf("mqttSubscribeTopic:   %s\r\n", this->mqttSubscribeTopic);
	
	if (this->IsSecure())
	{
		serial.printf("SECURE MQTT CONNECTION:\r\n");
		serial.printf("mqttUser:             %s\r\n", this->mqttUser);
		serial.printf("mqttPass:             %s\r\n", this->mqttPass);
	}
	serial.println("-----------------------------------------------");
}



template <class T> int xnsClientConfig::EEPROM_writeAnything(int ee, const T& value)
{
	const byte* p = (const byte*)(const void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		EEPROM.write(ee++, *p++);
	return i;
}

template <class T> int xnsClientConfig::EEPROM_readAnything(int ee, T& value)
{
	byte* p = (byte*)(void*)&value;
	unsigned int i;
	for (i = 0; i < sizeof(value); i++)
		*p++ = EEPROM.read(ee++);
	return i;
}

int xnsClientConfig::ReadString(int pAddress, char* pString[])
{
	int vAddress = 0;

	byte vLength = EEPROM.read(pAddress + vAddress);
	vAddress++;
	
	//Serial.print("Found length of string: ");
	//Serial.println(vLength);
	
	char* buffer = new char[vLength];
	for (int i = 0; i<vLength; i++)
	{
		buffer[i] = EEPROM.read(pAddress + vAddress++);
	}
	*pString = buffer;
	
	//Serial.print("Read string from EEPROM: [");
	//Serial.print(*pString);
	//Serial.println("]");
	
	return vAddress;
}
int xnsClientConfig::SaveString(int pAddress, char* pString)
{
	int vAddress = 0;
	int vLength = strlen(pString) + 1;
	//Serial.print("Storing length of string: ");
	//Serial.println(vLength);
	EEPROM.put(pAddress + vAddress, vLength);
	vAddress++;

	//Serial.print("Storing string: ");
	//Serial.println(pString);
	for (int i = 0; i<vLength; i++)
	{
		EEPROM.put(pAddress + vAddress, pString[i]);
		vAddress++;
	}

	return vAddress;
}
