#include "JimHelper.h"
#include <ESP8266httpUpdate.h>
#include <Adafruit_GFX.h>

JimHelper::JimHelper() {}

void JimHelper::ConnectWiFi2() 
{
    WiFiManager wm;

    // 如果 ESP8266 找不到之前存過的 Wi-Fi，就會自動開啟 AP
    // 熱點名稱叫 "ESP_Setup_Portal"，密碼是 "88888888"
    if (!wm.autoConnect("WMS", "88888888")) {
        Serial.println("設定失敗，重啟中...");
        delay(3000);
        ESP.restart();
    }

    // 只要跑到這行，代表已經成功連上某個 Wi-Fi 了
    Serial.println("已成功連網！");

}

// ThingSpeak 設定
unsigned long _channelNumber = 3328532;
const char* _writeAPIKey = "TVHHNYVEN9M8C07S";


void JimHelper::Post2ThingSpeak() 
{
  float mockTemp = random(200, 301) / 10.0;

  ThingSpeak.setField(1, mockTemp);
  int x = ThingSpeak.writeFields(_channelNumber, _writeAPIKey);
  if (x == 200) {
    Serial.println("ThingSpeak 上傳成功！");
  } else {
    Serial.println("ThingSpeak 上傳失敗，代碼: " + String(x));
  }
}

void JimHelper::reconnectMQTT(PubSubClient& mqttClient) 
{
  String clientId = "ESP8266-" + String(ESP.getChipId(), HEX);
	
  while (!mqttClient.connected()) {
    Serial.print("嘗試連接 Mosquitto Broker...");
    // 建立唯一的 Client ID
    //String clientId = "ESP8266Client-Jim";
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("已連線！");
	  
	  String topic = "device/" + clientId;
      // 連線成功後訂閱主題
      mqttClient.subscribe(topic.c_str());
    } else {
      Serial.print("失敗, 錯誤碼 rc=");
      Serial.print(mqttClient.state());
      Serial.println("，5秒後重試...");
      delay(5000);
    }
  }
}

void JimHelper::checkUpdate() 
{
	WiFiClient client;
	
	// 給網路堆疊一點反應時間
	delay(500);
	
	Serial.print("ESP8266 IP: ");
	
	Serial.println(WiFi.localIP()); // 確認 ESP8266 拿到的 IP 是不是也是 10.12.125.X
	
    HTTPClient http;
    Serial.println("正在檢查版本更新...");

    if (http.begin(client, _versionUrl)) {
		
		http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
		
        int httpCode = http.GET();
		
		Serial.println(httpCode);	
		
		
        if (httpCode == HTTP_CODE_OK) {
            String newVersion = http.getString();
            if (newVersion.toInt() > _currentVersion) {
                Serial.printf("發現新版本: %s, 準備更新...\n", newVersion.c_str());
                
                // 開始更新
                t_httpUpdate_return ret = ESPhttpUpdate.update(client, _binUrl);

                switch (ret) {
                    case HTTP_UPDATE_FAILED:
                        Serial.printf("更新失敗: %s\n", ESPhttpUpdate.getLastErrorString().c_str());
                        break;
                    case HTTP_UPDATE_NO_UPDATES:
                        Serial.println("伺服器端無更新檔");
                        break;
                    case HTTP_UPDATE_OK:
                        Serial.println("更新成功！重啟中...");
                        // 設備會自動重啟，這裡之後的程式碼不會執行
                        break;
                }
            } else {
                Serial.println("目前已是最新版本。");
            }
        }
        http.end();
    }
}

void JimHelper::LedDraw(String _wo, String _text, int _stock, Adafruit_SSD1306& display, int screenWidth, int screenHeight) 
{
  display.clearDisplay(); // 務必先清除緩衝區

  //畫一個外框
  display.drawRect(0, 0, screenWidth, screenHeight, WHITE);

  //設定文字屬性
  display.setTextSize(1);      
  display.setTextColor(WHITE); 
  
  //第一行文字
  display.setCursor(10, 4);    
  display.println("WO:" + _wo);
  //第二行文字
  display.setTextSize(1);
  display.setCursor(10, 24);
  display.println("Stock:" + String(_stock));
  //第三行文字
  display.setTextSize(1);
  display.setCursor(10, 44);
  display.println(String("PU:") + _text);


  // 真正將以上指令推送到螢幕
  display.display(); 
}