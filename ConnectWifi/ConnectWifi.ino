#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>
#include "JimHelper.h"

// ========== 系統設定 ==========
const char* MQTT_SERVER = "10.12.125.144";
const char* MQTT_TOPIC  = "device/ESP8266-7ac074";

// I2C 腳位
#define I2C_SDA 12  // D6
#define I2C_SCL 14  // D5

// OLED 解析度
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

// 按鍵腳位
#define BTN_PIN 13
#define LED_PIN 2

// ========== 全域變數 ==========
WiFiClient       g_WifiClient;
PubSubClient    g_MQTTClient(g_WifiClient);
Adafruit_SSD1306 g_Display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
JimHelper      g_Helper;

int lastBtnState = LOW;

// ==========  Callback ==========
void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.println("[收到 MQTT]: " + msg);

  // JSON 解析
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, msg);

  if (error) {
    Serial.println("[JSON 解析失敗]: " + String(error.c_str()));
    return;
  }

  if (!doc.containsKey("action")) {
    Serial.println("[錯誤] 缺少 action 欄位");
    return;
  }

  String action = doc["action"];

  if (action == "update") {
    Serial.println("[執行] OTA 更新");
    g_Helper.checkUpdate();
  } 
  else if (action == "display") {
    int _qty  = doc.containsKey("pu") ? doc["pu"].as<int>() : 0;
    String wo   = doc.containsKey("wo")      ? doc["wo"].as<String>()      : "N/A";
    int stock  = doc.containsKey("stock")    ? doc["stock"].as<int>()      : 0;

    g_Helper.LedDraw(wo, _qty, stock, g_Display, SCREEN_WIDTH, SCREEN_HEIGHT);
    digitalWrite(LED_PIN, LOW);  // 亮燈
    Serial.println("[顯示] WO: " + wo + ", Stock: " + String(stock));
  }
  else {
    Serial.println("[未知指令]: " + action);
  }
}

// ==========  發送 MQTT ==========
bool publishMQTT(const char* action, const char* status, const char* msg) {
  String payload = "{\"action\":\"" + String(action) + 
                  "\", \"status\":\"" + String(status) + 
                  "\", \"msg\":\"" + String(msg) + "\"}";
  
  bool success = g_MQTTClient.publish(MQTT_TOPIC, payload.c_str());
  Serial.println(success ? "[MQTT 發送成功]: " + payload : "[MQTT 發送失敗]");
  return success;
}

// ==========  Setup ==========
void setup() 
{
  Serial.begin(115200);
  delay(2000);

  // IO 初始化
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED 預設熄滅 (Active Low)
  pinMode(BTN_PIN, INPUT);

  // I2C 初始化
  Wire.begin(I2C_SCL, I2C_SDA);
  Wire.setClock(100000);

  // OLED 初始化
  if (!g_Display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) 
  {
    Serial.println("[錯誤] OLED 初始化失敗");
    for(;;);
  }
  g_Display.clearDisplay();
  g_Display.display();

  // WiFi 連線
  g_Helper.ConnectWiFi();

  // MQTT 初始化
  g_MQTTClient.setServer(MQTT_SERVER, 1883);
  g_MQTTClient.setCallback(MQTTCallback);

  Serial.println("[系統] 初始化完成");
}

// ==========  Loop ==========
void loop() {
  // 維持 MQTT 連線
  g_Helper.reconnectMQTT(g_MQTTClient, MQTT_TOPIC);
  g_MQTTClient.loop();

  // 按鈕偵測（邊緣觸發 + Debounce）
  int currentBtnState = digitalRead(BTN_PIN);
  
  if (currentBtnState == HIGH && lastBtnState == LOW) {
    delay(50);  // Debounce
    if (digitalRead(BTN_PIN) == HIGH) {
      digitalWrite(LED_PIN, HIGH);  // 熄滅 LED
      publishMQTT("confirm", "done", "Task Finished");
    }
  }
  lastBtnState = currentBtnState;
}