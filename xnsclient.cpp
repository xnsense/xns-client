// 
// 
// 


#include "xnsclient.h"
#include "xnsclientconfig.h"

void xnsClientClass_mqttMessageReceived(char* topic, unsigned char* payload, unsigned int length);

xnsClientClass::xnsClientClass()
{

}
void xnsClientClass::setAliveMessageInterval(unsigned long interval)
{
  aliveMessageInterval = interval * 1000 * 60;
}

void xnsClientClass::begin(const char* hwName, const char* hwRevision, const int fwRevision[], int accessPointButtonPin, Stream& debugger)
{
  this->hwName = hwName;
  this->hwRevision = hwRevision;
  this->fwRevision = fwRevision;
  
  this->debugger = &debugger;
  this->accessPointButtonPin = accessPointButtonPin;

  if (accessPointButtonPin)
    pinMode(accessPointButtonPin, INPUT);

  xnsClientConfig config = xnsClientConfig();
  if (config.Load(0) && !this->isAPButtonPressed())
  {
    if (this->debugger) config.Print(debugger);

    if (config.IsSecure())
      this->beginSecure(hwName, hwRevision, fwRevision, config.ssid, config.ssidPassword, config.mqtt, config.mqttPort, config.mqttClientID, config.mqttUser, config.mqttPass, config.mqttPublishTopic, config.mqttSubscribeTopic, debugger);
    else
    {
      this->mqttClientID = config.mqttClientID;
      this->mqttPublishTopic = config.mqttPublishTopic;
      this->mqttSubscribeTopic = config.mqttSubscribeTopic;
      this->begin(hwName, hwRevision, fwRevision, config.ssid, config.ssidPassword, config.mqtt, config.mqttPort, config.mqttUser, config.mqttPass, debugger);
    }
  }
  else
  {
    xnsClientWebServerClass::Setup(&config);
  }
}

void xnsClientClass::beginSecure(const char* hwName, const char* hwRevision, const int fwRevision[], const char* ssid, const char* ssidPassword, const char* mqtt, int mqttPort, const char* mqttClientID, const char* mqttUser, const char* mqttPass, const char* mqttPublishTopic, const char* mqttSubscribeTopic, Stream& debugger)
{
  this->mqttClientID = mqttClientID;
  this->mqttPublishTopic = mqttPublishTopic;
  this->mqttSubscribeTopic = mqttSubscribeTopic;

  this->secure = true;
  this->debugger = &debugger;
  begin(hwName, hwRevision, fwRevision, ssid, ssidPassword, mqtt, mqttPort, mqttUser, mqttPass);
}
void xnsClientClass::begin(const char* hwName, const char* hwRevision, const int fwRevision[], const char* ssid, const char* ssidPassword, const char* mqtt, int mqttPort, const char* mqttUser, const char* mqttPass, Stream& debugger)
{
  this->debugger = &debugger;
  begin(hwName, hwRevision, fwRevision, ssid, ssidPassword, mqtt, mqttPort, mqttUser, mqttPass);
}
void xnsClientClass::begin(const char* hwName, const char* hwRevision, const int fwRevision[], const char* ssid, const char* ssidPassword, const char* mqtt, int mqttPort, const char* mqttUser, const char* mqttPass)
{
  this->mqttUser = mqttUser;
  this->mqttPass = mqttPass;
  begin(hwName, hwRevision, fwRevision, ssid, ssidPassword, mqtt, mqttPort);
}
void xnsClientClass::begin(const char* hwName, const char* hwRevision, const int fwRevision[], const char* ssid, const char* ssidPassword, const char* mqtt, int mqttPort, Stream& debugger)
{
  this->debugger = &debugger;
  begin(hwName, hwRevision, fwRevision, ssid, ssidPassword, mqtt, mqttPort);
}

void xnsClientClass::begin(const char* hwName, const char* hwRevision, const int fwRevision[], const char* ssid, const char* ssidPassword, const char* mqtt, int mqttPort)
{
  this->hwName = hwName;
  this->hwRevision = hwRevision;
  this->fwRevision = fwRevision;
  this->ssid = ssid;
  this->ssidPassword = ssidPassword;
  
  if (secure)
    client = new WiFiClientSecure();
  else
    client = new WiFiClient();

  this->mqttName = mqtt;
  this->mqttPort = mqttPort;

  String vMqttName = String(this->hwName) + "[" + WiFi.macAddress() + "]";
  mqttClientName = mqttClientID == 0 ? vMqttName.c_str() : mqttClientID;
  
  this->mqtt = PubSubClient(*client);

  this->printDeviceInfo();
  WiFi.enableAP(false);
  WiFi.begin(this->ssid, this->ssidPassword);
  this->mqtt.setServer(mqttName, this->mqttPort);

  //std::function<void(char*, unsigned char*, unsigned int)> vCallback = MakeDelegate(this, xnsClientClass::mqttMessageReceived);
  this->mqtt.setCallback(xnsClientClass_mqttMessageReceived);
  this->MQTT_connect();

  sendMqttMessage("Connected");
  lastAliveMessage = 0;
}

void xnsClientClass::loop()
{
  mqtt.loop();
  delay(10); // <- fixes some issues with WiFi stability

  if (!client->connected() || !mqtt.connected()) {
    MQTT_connect();
  }
  else
  {
    if (aliveMessageInterval > 0 && (lastAliveMessage + aliveMessageInterval < millis()))
    {
      sendMqttMessage("I'm alive");
      lastAliveMessage = millis();
    }
  }
}

bool xnsClientClass::connected()
{
  return client->connected() && mqtt.connected();
}

void xnsClientClass::sendMqttData(JsonObject& data)
{
  String vTopic = mqttPublishTopic == 0 ? String("sensors/out/") + WiFi.macAddress() : String(mqttPublishTopic);
  if (!client->connected() || !mqtt.connected()) {
    MQTT_connect();
  }
  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["id"] = WiFi.macAddress();
  json["up"] = millis() / 1000;
  json["data"] = data;

  String msg;
  json.printTo(msg);

  mqtt.publish(vTopic.c_str(), msg.c_str());

}
void xnsClientClass::sendMqttData(String data)
{
  String vTopic = mqttPublishTopic == 0 ? String("sensors/out/") + WiFi.macAddress() : String(mqttPublishTopic);
  
  if (!client->connected() || !mqtt.connected()) {
    MQTT_connect();
  }

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["id"] = WiFi.macAddress();
  json["up"] = millis() / 1000;
  json["data"] = data;

  String msg;
  json.printTo(msg);

  mqtt.publish(vTopic.c_str(), msg.c_str());

}

void xnsClientClass::sendMqttMessage(JsonObject& message)
{
  String vTopic = mqttPublishTopic == 0 ? String("sensors/out/") + WiFi.macAddress() : String(mqttPublishTopic);
  if (!client->connected() || !mqtt.connected()) {
    MQTT_connect();
  }
  message["id"] = WiFi.macAddress();
  message["up"] = millis() / 1000;
  message["hwName"] = hwName;
  message["hwRevision"] = hwRevision;
  message["fwRevision"] = GetVersionString();

  String msg;
  message.printTo(msg);

  mqtt.publish(vTopic.c_str(), msg.c_str());
}

void xnsClientClass::sendMqttMessage(String message)
{
  String vTopic = mqttPublishTopic == 0 ? String("sensors/out/") + WiFi.macAddress() : String(mqttPublishTopic);
  
  if (!client->connected() || !mqtt.connected()) {
    MQTT_connect();
  }

  StaticJsonBuffer<500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  json["id"] = WiFi.macAddress();
  json["up"] = millis() / 1000;
  json["hwName"] = hwName;
  json["hwRevision"] = hwRevision;
  json["fwRevision"] = GetVersionString();
  json["message"] = message;

  String msg;
  json.printTo(msg);

  mqtt.publish(vTopic.c_str(), msg.c_str());
}

void xnsClientClass::printDeviceInfo()
{
  if (debugger)
  {
    debugger->println("\r\n+++++++++++++++++++++++++++++++++++++\r\n");
    debugger->print("Hardware name : ");
    debugger->println(hwName);
    debugger->print("Hardware revision : ");
    debugger->println(hwRevision);
    debugger->print("Firmware revision : ");
    debugger->println(GetVersionString());
    debugger->print("MAC address: ");
    debugger->println(WiFi.macAddress());
    debugger->println("\r\n-------------------------------------\r\n");
    debugger->println("Flash info:\r\n");
    printFlashInfo();
  }
}
void xnsClientClass::printFlashInfo()
{
  if (debugger)
  {
    uint32_t realSize = ESP.getFlashChipRealSize();
    uint32_t ideSize = ESP.getFlashChipSize();
    FlashMode_t ideMode = ESP.getFlashChipMode();

    debugger->printf("Flash real id:   %08X\n", ESP.getFlashChipId());
    debugger->printf("Flash real size: %u\n\n", realSize);

    debugger->printf("Flash ide  size: %u\n", ideSize);
    debugger->printf("Flash ide speed: %u\n", ESP.getFlashChipSpeed());
    debugger->printf("Flash ide mode:  %s\n", (ideMode == FM_QIO ? "QIO" : ideMode == FM_QOUT ? "QOUT" : ideMode == FM_DIO ? "DIO" : ideMode == FM_DOUT ? "DOUT" : "UNKNOWN"));

    if (ideSize != realSize) {
      debugger->println("Flash Chip configuration wrong!\n");
    }
    else {
      debugger->println("Flash Chip configuration ok.\n");
    }
  }
}




String xnsClientClass::GetVersionString()
{
  String vVersion;
  for (int i = 0; i < 4; i++)
  {
    vVersion += fwRevision[i];
    if (i < 3)
      vVersion += ".";
  }
  return vVersion;
}



// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void xnsClientClass::MQTT_connect() {
  // Connect to WiFi access point.
  if (debugger) debugger->println(); if (debugger) debugger->println();
  if (debugger) debugger->print("Connecting to ");
  if (debugger) debugger->println(this->ssid);

  long vTimeout = millis() + WIFI_CONNECTION_TIMEOUT;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (debugger) debugger->print(".");
    if (vTimeout < millis())
    {
      if (debugger)
      {
        debugger->print("Timout during connect. WiFi status is: ");
        debugger->println(WiFi.status());
      }
      WiFi.disconnect();
      WiFi.begin(this->ssid, this->ssidPassword);
      vTimeout = millis() + WIFI_CONNECTION_TIMEOUT;
    }
    yield();
  }

  if (debugger) {
    debugger->println();
    debugger->println("WiFi connected");
    debugger->println("IP address: ");
    debugger->println(WiFi.localIP());
    debugger->print("\nconnecting to MQTT: ");
    debugger->print(mqttName);
    debugger->print(", port: ");
    debugger->print(mqttPort);
    debugger->println();
  }
  while (!mqtt.connected()) { // (!mqtt.connect(mqttClientName)) { //, "user", "pass")) {
    if ((mqttUser == 0 && mqtt.connect(mqttClientName)) || (mqttUser != 0 && mqtt.connect(mqttClientName, mqttUser, mqttPass)))
    {
      if (debugger) debugger->println("\nSuccessfully connected to MQTT!");
      if (mqttSubscribeTopic != 0)
      {
        mqtt.subscribe(mqttSubscribeTopic);
        if (debugger) debugger->printf("  Subscribing to [%s]\r\n", mqttSubscribeTopic);
      }
      else
      {
        String vTopic = String("sensors/in/") + WiFi.macAddress();
        char buffer[vTopic.length() + 1];
        vTopic.toCharArray(buffer, vTopic.length() + 1, 0);
        mqtt.subscribe(buffer);
        if (debugger) debugger->printf("  Subscribing to [%s]\r\n", buffer);
      }
    }
    else
    {
      if (debugger) debugger->print(".");
      if (debugger) debugger->print("failed, rc=");
      if (debugger) debugger->print(mqtt.state());
      if (debugger) debugger->println(" trying again in 5 seconds");
      // Wait 2 seconds before retrying
      mqtt.disconnect();
      delay(2000);
    }
    yield();
  }

  
}



void xnsClientClass::UpgradeFirmware(String pUrl)
{
  String initMessage = String("OTA upgrade from [" + pUrl + "]");
  sendMqttMessage(initMessage);

  char vUrlArray[255];
  pUrl.toCharArray(vUrlArray, pUrl.length() + 1);

  String message("");
  t_httpUpdate_return ret = ESPhttpUpdate.update(vUrlArray);

  switch (ret) {
  case HTTP_UPDATE_FAILED:
    message += "HTTP_UPDATE_FAILD Error (";
    message += ESPhttpUpdate.getLastError();
    message += "): ";
    message += ESPhttpUpdate.getLastErrorString().c_str();
    if (debugger) debugger->println(message);

    sendMqttMessage(String("ERROR: " + message));
    break;

  case HTTP_UPDATE_NO_UPDATES:
    if (debugger) debugger->println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    ESP.restart();
    if (debugger) debugger->println("HTTP_UPDATE_OK");
    break;
  }
}

void xnsClientClass::mqttMessageReceived(char* topic, unsigned char* payload, unsigned int length) {
  
  char message[MQTT_MAX_PACKET_SIZE];
  for (int i = 0; i < length; i++)
    message[i] = payload[i];
  message[length] = 0;
  
  if (debugger)
  {
    debugger->print("incoming: ");
    debugger->print(topic);
    debugger->print(" - ");
    debugger->print(message);
    debugger->println();
  }

  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(message);

  if (getJsonValue(json, "id").equals(WiFi.macAddress()))
  {
    if (getJsonValue(json, "command").equals("UpgradeFromHttp"))
    {
      String vUrl = getJsonValue(json, "url");
      if (vUrl.length() > 4)
      {
        if (debugger) debugger->print("Upgrade requested from url:");
        if (debugger) debugger->println(vUrl);
        UpgradeFirmware(vUrl);
      }
    }
    else if (getJsonValue(json, "command").equals("Echo"))
    {
      String vMessage = String("Echo: ") + getJsonValue(json, "message");
      sendMqttMessage(vMessage);
    }
    else if (getJsonValue(json, "command").equals("GetConfig"))
    {
      sendConfigMessage();
    }
    else if (getJsonValue(json, "command").equals("SetConfig"))
    {
      setConfig(json["config"].asObject());
    }
    else
    {
      if (this->mqttMessageCallback)
      {
        if (debugger) debugger->println("Passing message on to client");
        this->mqttMessageCallback(json);
      }
      else
      {
        if (debugger) debugger->println("Client is not subscribing to messages");
      }
    }
  }
  else
  {
    if (debugger) debugger->println("Message was not addressed for this device");
  }
}
void xnsClientClass::setConfig(JsonObject& json)
{
  xnsClientConfig config;
  config.Load(0);

  if (hasValue(json, "ssid")) 
  {
    String value = getJsonValue(json, "ssid");
    config.ssid = new char[value.length() + 1];
    value.toCharArray(config.ssid, value.length() + 1, 0);
  }
  if (hasValue(json, "ssidPassword"))
  {
    String value = getJsonValue(json, "ssidPassword");
    config.ssidPassword = new char[value.length() + 1];
    value.toCharArray(config.ssidPassword, value.length() + 1, 0);
  }
  if (hasValue(json, "mqtt"))
  {
    String value = getJsonValue(json, "mqtt");
    config.mqtt = new char[value.length() + 1];
    value.toCharArray(config.mqtt, value.length() + 1, 0);
  }
  if (hasValue(json, "mqttPort"))
  {
    String value = getJsonValue(json, "mqttPort");
    config.mqttPort = value.toInt();
  }
  if (hasValue(json, "mqttClientID"))
  {
    String value = getJsonValue(json, "mqttClientID");
    config.mqttClientID = new char[value.length() + 1];
    value.toCharArray(config.mqttClientID, value.length() + 1, 0);
  }
  if (hasValue(json, "mqttPublishTopic"))
  {
    String value = getJsonValue(json, "mqttPublishTopic");
    config.mqttPublishTopic = new char[value.length() + 1];
    value.toCharArray(config.mqttPublishTopic, value.length() + 1, 0);
  }
  if (hasValue(json, "mqttSubscribeTopic"))
  {
    String value = getJsonValue(json, "mqttSubscribeTopic");
    config.mqttSubscribeTopic = new char[value.length() + 1];
    value.toCharArray(config.mqttSubscribeTopic, value.length() + 1, 0);
  }
  if (hasValue(json, "mqttUser"))
  {
    String value = getJsonValue(json, "mqttUser");
    config.mqttUser = new char[value.length() + 1];
    value.toCharArray(config.mqttUser, value.length() + 1, 0);
  }
  if (hasValue(json, "mqttPass"))
  {
    String value = getJsonValue(json, "mqttPass");
    config.mqttPass = new char[value.length() + 1];
    value.toCharArray(config.mqttPass, value.length() + 1, 0);
  }

  config.Print(Serial);
  config.Save(0);

  sendMqttMessage("New config received and stored in EEPROM. Rebooting now");
  ESP.reset();
}

bool xnsClientClass::hasValue(JsonObject& json, String key)
{
  for (JsonObject::iterator it = json.begin(); it != json.end(); ++it) {
    if (key.equals(it->key))
      return true;
  }
  return false;
}

void xnsClientClass::sendConfigMessage()
{
  xnsClientConfig config;
  if (config.Load(0))
  {
    StaticJsonBuffer<500> jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    JsonObject& configJson = json.createNestedObject("Config");
    configJson["ssid"] = config.ssid;
    configJson["ssidPassword"] = config.ssidPassword;
    configJson["mqtt"] = config.mqtt;
    configJson["mqttPort"] = config.mqttPort;
    configJson["mqttClientID"] = config.mqttClientID;
    configJson["mqttPublishTopic"] = config.mqttPublishTopic;
    configJson["mqttSubscribeTopic"] = config.mqttSubscribeTopic;
    configJson["mqttUser"] = config.mqttUser;
    configJson["mqttPass"] = config.mqttPass;

    sendMqttMessage(json);
  }
  else
  {
    sendMqttMessage("No config in EEPROM");
  }
}

String xnsClientClass::getJsonValue(JsonObject& json, String key)
{
  for (JsonObject::iterator it = json.begin(); it != json.end(); ++it) {
    if (key.equals(it->key))
      return String(it->value.asString());
  }
  return String("");
}

JsonVariant xnsClientClass::getJsonObject(JsonObject& json, String key)
{
  for (JsonObject::iterator it = json.begin(); it != json.end(); ++it) {
    if (key.equals(it->key))
      return JsonVariant(it->value.asObject());
  }
  return JsonVariant(NULL);
}

void xnsClientClass::setMqttMessageCallback(void(*mqttMessageCallback)(JsonObject& json))
{
  this->mqttMessageCallback = mqttMessageCallback;
}

bool xnsClientClass::isAPButtonPressed()
{
  if (accessPointButtonPin)
  {
    return digitalRead(accessPointButtonPin);
  }
  else
    return false;
}




xnsClientClass xnsClient;



void xnsClientClass_mqttMessageReceived(char* topic, unsigned char* payload, unsigned int length) {
  xnsClient.mqttMessageReceived(topic, payload, length);
}