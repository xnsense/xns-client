// xnsClientWebServer.h

#ifndef _xnsClientWebServer_h
#define _xnsClientWebServer_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "xnsclientconfig.h"

class xnsClientWebServerClass
{
public:
	xnsClientWebServerClass()
	{
		
	}
	static void Setup(xnsClientConfig *pConfig);
	static void Handle();
	static xnsClientConfig *config;

protected:
	static void SetupWebServer();
	static void WebServer_GetConfig();
	static void WebServer_SaveConfig();

private:
	static const char* AP_SSID;
	static ESP8266WebServer gWebServer;
	static String getConfigPage();
	static String saveConfigResponsePage();
};


#endif
