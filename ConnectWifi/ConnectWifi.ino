#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <PubSubClient.h> // MQTT

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>


#include "JimHelper.h"

JimHelper helper;

// 1. 基本資訊設定
//const char* ssid = "Jim Sung";
//const char* password = "xxxxxxxxxx";
const char* mqtt_server = "10.12.125.144";


// 2. 物件定義 (修正變數名稱衝突)
WiFiClient espClient;           // 用於 WiFi
PubSubClient mqttClient(espClient); // 用於 MQTT，傳入 espClient

// 0.96 吋 OLED 的標準解析度
#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 

// 你的腳位定義
#define OLED_SDA 12 // D6
#define OLED_SCL 14 // D5
// 初始化顯示器
// 宣告時指定寬、高、I2C 介面、以及 Reset 腳位 (無則填 -1)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define LED_PIN 2  // GPIO2 = D4 (大部分 ESP8266 內建 LED)

// 3. MQTT 收到訊息的回呼函式 (當電腦發布訊息時，這裡會觸發)

bool ledState = true; // true 代表燈熄滅, false 代表燈亮 (配合 Active Low)
int lastBtnState = LOW;

void setup() 
{
  Serial.begin(115200);
  delay(2000); // 延長到 2 秒，等電力穩定
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // 預設熄滅 (Active Low)
  pinMode(13, INPUT);

  // 連接 WiFi
  helper.ConnectWiFi2();
  // 設定 MQTT
  mqttClient.setServer(mqtt_server, 1883);

  mqttClient.setCallback(mqttCallback);

  // 初始化 ThingSpeak
  //ThingSpeak.begin(espClient);

  // 3. 顯示測試畫面
  Wire.begin(14, 12); // D6, D5  
  Wire.setClock(100000);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("ERROR");
    for(;;); 
  }


  //testDraw();
}

void loop() 
{
  // 檢查 MQTT 連線狀態
  helper.reconnectMQTT(mqttClient);

  
  // 維持 MQTT 背景運作 (關鍵！)
  mqttClient.loop();


// 1. 讀取按鈕狀態
  int currentBtnState = digitalRead(13); // D7

  // 2. 只有當狀態從「沒按」變成「按下」時才動作 (邊緣觸發)
  if (currentBtnState == HIGH && lastBtnState == LOW) 
  {
    delay(50); // 簡單去彈跳 (Debounce)
    if (digitalRead(13) == HIGH) // 再次確認不是雜訊
    {
       Serial.println(">>> 手動按鈕觸發：關閉 LED");
       digitalWrite(LED_PIN, HIGH); // 熄滅 LED


       String payload = "{\"action\":\"confirm\", \"status\":\"done\", \"msg\":\"Task Finished\"}";
      
      if (mqttClient.publish("device/ESP8266-7ac074", payload.c_str())) {
        Serial.println(">>> MQTT 回報成功！");
      } else {
        Serial.println(">>> MQTT 回報失敗，請檢查連線。");
      }
    }
  }

  lastBtnState = currentBtnState;
  //helper.checkUpdate();

  // 範例：每 20 秒上傳一次資料到 ThingSpeak (不要用 delay 擋住 loop)
  /*
  static unsigned long lastUploadTime = 0;
  if (millis() - lastUploadTime > 20000) {
    Post2ThingSpeak();
    lastUploadTime = millis();
  }
  */
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("收到 MQTT 主題 [");
  Serial.print(topic);
  Serial.print("] 訊息: ");
  
  String msg;
  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println(msg);

  // 可以在這裡根據 msg 內容做動作，例如控制 LED
  // ===== JSON 解析 =====
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, msg);

  if (error) 
  {
    Serial.print("JSON 解析失敗: ");
    Serial.println(error.c_str());
    return;
  }

  // 2. 檢查 action 是否存在
  if (!doc.containsKey("action")) 
  {
        Serial.println("錯誤: JSON 缺少 action 欄位");
        return;
  }


 String action = doc["action"];

  if (action == "update") 
  {
    Serial.println(">>> 指令確認：執行 OTA 更新檢查...");
    
    helper.checkUpdate(); // 執行你寫在 JimHelper 裡的更新功能
  } 
  else if (action == "display") 
  {
    String text = doc.containsKey("content") ? doc["content"].as<String>() : "N/A";
    String wo   = doc.containsKey("wo")      ? doc["wo"].as<String>()      : "N/A";
    int stock   = doc.containsKey("stock")   ? doc["stock"].as<int>()      : 0;

    LedDraw(wo,text,stock);

    digitalWrite(LED_PIN, LOW); // 亮燈
  }
  else 
  {
    Serial.println(">>> 收到未知指令，不執行任何動作。");
  }
}

void LedDraw(String _wo,String _text,int _stock) 
{
  display.clearDisplay(); // 務必先清除緩衝區

  //畫一個外框
  display.drawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);

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




