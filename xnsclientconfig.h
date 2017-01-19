// xnsClientConfig.h

#ifndef _xnsClientConfig_h
#define _xnsClientConfig_h

#include <EEPROM.h>
#include <arduino.h>

class xnsClientConfig
{
public:
	char* ssid;
	char* ssidPassword;
	char* mqtt;
	int mqttPort;
	char* mqttClientID;
	char* mqttPublishTopic;
	char* mqttSubscribeTopic;
	char* mqttUser;
	char* mqttPass;

	bool IsSecure();
	bool Save(int pAddress);
	bool Load(int pAddress);

	void Print(Stream& serial);
protected:
	
private:
	const int EEPROM_SIZE = 512;
	const byte EEPROM_CHECK_SUM = 124; // Used to check if xnsClientConfig is stored. Change if structure changes

	int SaveString(int pAddress, char* pString);
	int ReadString(int pAddress, char* pString[]);
	int SaveInt(int pAddress, int pValue);
	int ReadInt(int pAddress, int *pValue);
	int SaveBool(int pAddress, bool pValue);
	int ReadBool(int pAddress, bool *pValue);


	template <class T> int EEPROM_writeAnything(int ee, const T& value);
	template <class T> int EEPROM_readAnything(int ee, T& value);
};

#endif

