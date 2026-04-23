#ifndef JimHelper_h
#define JimHelper_h


#include <Arduino.h>
#include <WiFiManager.h>
#include <ThingSpeak.h>
#include <PubSubClient.h>


class JimHelper 
{
  private:
    int _currentVersion = 2; // 目前的版本號
    const char* _versionUrl = "http://10.12.125.144/version.txt";
    const char* _binUrl = "http://10.12.125.144/ConnectWifi.ino.bin";
	
  public:
    JimHelper(); // 建構子
    void ConnectWiFi2(); //開啟AP模式，WIFI輸入帳密
	void Post2ThingSpeak(); //資料拋傳至thingspeak
	void reconnectMQTT(PubSubClient& _client); //MQTT重連
	void checkUpdate(); //OAT更新
};

#endif