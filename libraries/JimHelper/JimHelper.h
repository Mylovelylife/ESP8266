#ifndef JimHelper_h
#define JimHelper_h

#include <Arduino.h>
#include <WiFiManager.h>
#include <ThingSpeak.h>
#include <PubSubClient.h>
#include <Adafruit_SSD1306.h>

class JimHelper 
{
  private:
    int _currentVersion = 2;
    const char* _versionUrl = "http://10.12.125.144/version.txt";
    const char* _binUrl = "http://10.12.125.144/ConnectWifi.ino.bin";

  public:
    JimHelper();
    void ConnectWiFi2();
    void Post2ThingSpeak();
    void reconnectMQTT(PubSubClient& mqttClient, const char* topic);
    void checkUpdate();
    void LedDraw(String _wo, String _text, int _stock, Adafruit_SSD1306& display, int screenWidth = 128, int screenHeight = 64);
};

#endif