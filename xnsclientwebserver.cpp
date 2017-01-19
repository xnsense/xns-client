// 
// 
// 

#include "xnsclientwebserver.h"
#include "xnsclientconfig.h"

ESP8266WebServer xnsClientWebServerClass::gWebServer = ESP8266WebServer(80);
xnsClientConfig* xnsClientWebServerClass::config;

void xnsClientWebServerClass::Setup(xnsClientConfig *pConfig)
{
	config = pConfig;

	WiFi.disconnect(true);
	WiFi.softAPdisconnect(true);
	WiFi.mode(WIFI_OFF);
	delay(2000);

	WiFi.softAP(AP_SSID);
	WiFi.mode(WIFI_AP);

	Serial.print("Configuring access point as ");
	Serial.println(AP_SSID);

	IPAddress vIP = WiFi.softAPIP();
	Serial.print("AP IP Address: ");
	Serial.println(vIP);
	
	SetupWebServer();

	while (true)
		gWebServer.handleClient();
}

void xnsClientWebServerClass::SetupWebServer()
{
	
	Serial.println("Setting up web server on port 80");
	gWebServer.on("/", WebServer_GetConfig);
	gWebServer.on("/save", HTTP_POST, WebServer_SaveConfig);
	gWebServer.begin();
}

void xnsClientWebServerClass::WebServer_GetConfig()
{
	Serial.println("Responding to HTTP request at '/'");
	String html = getConfigPage();
	gWebServer.send(200, "text/html", html);
}

void xnsClientWebServerClass::WebServer_SaveConfig()
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

	xnsClientConfig *config = new xnsClientConfig();
	
	String temp;
	
	temp = gWebServer.arg("ssid");
	config->ssid = new char[temp.length() + 1];
	temp.toCharArray(config->ssid, temp.length() + 1, 0);

	temp = gWebServer.arg("ssidPassword");
	config->ssidPassword = new char[temp.length() + 1];
	temp.toCharArray(config->ssidPassword, temp.length() + 1, 0);

	temp = gWebServer.arg("mqtt");
	config->mqtt = new char[temp.length() + 1];
	temp.toCharArray(config->mqtt, temp.length() + 1, 0);

	config->mqttPort = (int)gWebServer.arg("mqttPort").toInt();

	temp = gWebServer.arg("mqttClientID");
	config->mqttClientID = new char[temp.length() + 1];
	temp.toCharArray(config->mqttClientID, temp.length() + 1, 0);

	temp = gWebServer.arg("mqttPublishTopic");
	config->mqttPublishTopic = new char[temp.length() + 1];
	temp.toCharArray(config->mqttPublishTopic, temp.length() + 1, 0);

	temp = gWebServer.arg("mqttSubscribeTopic");
	config->mqttSubscribeTopic = new char[temp.length() + 1];
	temp.toCharArray(config->mqttSubscribeTopic, temp.length() + 1, 0);

	temp = gWebServer.arg("mqttUser");
	config->mqttUser = new char[temp.length() + 1];
	temp.toCharArray(config->mqttUser, temp.length() + 1, 0);

	temp = gWebServer.arg("mqttPass");
	config->mqttPass = new char[temp.length() + 1];
	temp.toCharArray(config->mqttPass, temp.length() + 1, 0);

	Serial.println("Saving configuration now:");
	config->Print(Serial);
	
	config->Save(0);

	String html = saveConfigResponsePage();
	gWebServer.send(200, "text/html", html);

	ESP.reset();
}

const char* xnsClientWebServerClass::AP_SSID = "XNS_DEVICE";
/*
const char* WebConfig::AP_CONFIG_PAGE = "<html><body><form method='POST' action='/save'>SSID: <input name='ssid'/><br/>Password: <input name='pwd' type='password'/><br/><h3>MQTT</h3>Server: <input name='msrv'/><br/>Topic: <input name='mtop'/><br/>Sensor Name: <input name='mname'/><br/><input type='submit' value='submit'/></body></form>";
const char* WebConfig::AP_SUCCESS_PAGE = "<html><body><h1>Successflly Saved!</h1><h3>Please restart the device now</h3></body></form>";
*/

void xnsClientWebServerClass::Handle()
{
	gWebServer.handleClient();
}


String xnsClientWebServerClass::getConfigPage()
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

	String html = String(F("<html>"));
	html += F("<head>");
	html += F("<link rel = 'stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css' integrity='sha384-BVYiiSIFeK1dGmJRAkycuHAHRg32OmUcww7on3RYdg4Va+PmSTsz/K68vbdEjh4u' crossorigin='anonymous'>");
	html += F("</head>");
	html += F("<body>");
	html += F("<form method = 'POST' action = '/save'>");
	html += F("<h1>[");
	html += WiFi.macAddress();
	html += F("]</h1>");

	html += F("<h2>WiFi Connection</h2>");

	html += F("SSID: <input name='ssid' value='");
	html += config->ssid;
	html += F("'/><br/>");

	html += F("Password: <input name='ssidPassword' type='password' value='");
	html += config->ssidPassword;
	html += F("'/><br/>");

	html += "<h2>MQTT</h2>";

	html += F("Server: <input name='mqtt' value='");
	html += config->mqtt;
	html += F("'/>, port: <input name='mqttPort' value='");
	html += config->mqttPort;
	html += F("'/><br/>");
	html += F("Client ID: <input name='mqttClientID' value='");
	html += config->mqttClientID;
	html += F("'/><br/>");
	html += F("Publish Topic: <input name='mqttPublishTopic' value='");
	html += config->mqttPublishTopic;
	html += F("'/><br/>");
	html += F("Subscribe Topic: <input name='mqttSubscribeTopic' value='");
	html += config->mqttSubscribeTopic;
	html += F("'/><br/>");
	html += F("User: <input name='mqttUser' value='");
	html += config->mqttUser;
	html += F("'/><br/>");
	html += F("Password: <input name='mqttPass' type='password' value='");
	html += config->mqttPass;
	html += F("'/><br/>");
	html += F("<input type='submit' value='submit'/>");
	html += F("</form></body></html>");

	return html;
}
String xnsClientWebServerClass::saveConfigResponsePage()
{
	String html = String("<html><body>");
	html += "Configuration saved, device will now reboot...";
	html += "</body></html>";
	return html;
}
