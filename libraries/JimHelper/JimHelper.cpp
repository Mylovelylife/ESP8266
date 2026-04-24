#include "JimHelper.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

JimHelper::JimHelper() {}

void JimHelper::ConnectWiFi2() 
{
    WiFiManager wm;
    if (!wm.autoConnect("WMS", "88888888")) {
        Serial.println("[WiFi] 連線失敗，重啟中...");
        delay(3000);
        ESP.restart();
    }
    Serial.println("[WiFi] 已連線: " + WiFi.localIP().toString());
}

// ThingSpeak 設定
unsigned long _channelNumber = 3328532;
const char* _writeAPIKey = "TVHHNYVEN9M8C07S";

void JimHelper::Post2ThingSpeak() 
{
    float mockTemp = random(200, 301) / 10.0;
    ThingSpeak.setField(1, mockTemp);
    int x = ThingSpeak.writeFields(_channelNumber, _writeAPIKey);
    Serial.println(x == 200 ? "[ThingSpeak] 上傳成功" : "[ThingSpeak] 上傳失敗");
}

void JimHelper::reconnectMQTT(PubSubClient& mqttClient, const char* topic) 
{
    String clientId = "ESP8266-" + String(ESP.getChipId(), HEX);
    
    while (!mqttClient.connected()) {
        Serial.print("[MQTT] 連線中...");
        if (mqttClient.connect(clientId.c_str())) {
            Serial.println("OK");
            mqttClient.subscribe(topic);
        } else {
            Serial.print("失敗 rc=");
            Serial.print(mqttClient.state());
            Serial.println("，5秒後重試...");
            delay(5000);
        }
    }
}

void JimHelper::checkUpdate() 
{
    WiFiClient client;
    delay(500);
    
    HTTPClient http;
    Serial.println("[OTA] 檢查更新...");

    if (http.begin(client, _versionUrl)) {
        http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        int httpCode = http.GET();
        
        if (httpCode == HTTP_CODE_OK) {
            String newVersion = http.getString();
            if (newVersion.toInt() > _currentVersion) {
                Serial.println("[OTA] 發現新版本: " + newVersion);
                t_httpUpdate_return ret = ESPhttpUpdate.update(client, _binUrl);
                
                switch (ret) {
                    case HTTP_UPDATE_FAILED:
                        Serial.println("[OTA] 更新失敗: " + String(ESPhttpUpdate.getLastErrorString()));
                        break;
                    case HTTP_UPDATE_NO_UPDATES:
                        Serial.println("[OTA] 無更新檔");
                        break;
                    case HTTP_UPDATE_OK:
                        Serial.println("[OTA] 更新成功！重啟中...");
                        break;
                }
            } else {
                Serial.println("[OTA] 已是最新版本");
            }
        }
        http.end();
    }
}

void JimHelper::LedDraw(String _wo, String _text, int _stock, Adafruit_SSD1306& display, int screenWidth, int screenHeight) 
{
    display.clearDisplay();
    display.drawRect(0, 0, screenWidth, screenHeight, WHITE);
    
    display.setTextSize(1);
    display.setTextColor(WHITE);
    
    display.setCursor(10, 4);
    display.println("WO:" + _wo);
    
    display.setCursor(10, 24);
    display.println("Stock:" + String(_stock));
    
    display.setCursor(10, 44);
    display.println("PU:" + _text);
    
    display.display();
}