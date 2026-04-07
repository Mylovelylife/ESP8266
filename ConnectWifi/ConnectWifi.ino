#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <PubSubClient.h> // 確保已安裝此程式庫

// 1. 基本資訊設定
const char* ssid = "Jim Sung";
const char* password = "19860903";
const char* mqtt_server = "10.12.125.144";

// ThingSpeak 設定
unsigned long channelNumber = 3328532;
const char* writeAPIKey = "TVHHNYVEN9M8C07S";

// 2. 物件定義 (修正變數名稱衝突)
WiFiClient espClient;           // 用於 WiFi
PubSubClient mqttClient(espClient); // 用於 MQTT，傳入 espClient

// 3. MQTT 收到訊息的回呼函式 (當電腦發布訊息時，這裡會觸發)
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
}

void setup() {
  Serial.begin(115200);
  
  // 連接 WiFi
  connectWiFi();

  // 設定 MQTT
  mqttClient.setServer(mqtt_server, 1883);
  mqttClient.setCallback(mqttCallback);

  // 初始化 ThingSpeak
  ThingSpeak.begin(espClient);
}

void loop() 
{
  // 檢查 MQTT 連線狀態
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  
  // 維持 MQTT 背景運作 (關鍵！)
  mqttClient.loop();

  // 範例：每 20 秒上傳一次資料到 ThingSpeak (不要用 delay 擋住 loop)
  /*
  static unsigned long lastUploadTime = 0;
  if (millis() - lastUploadTime > 20000) {
    Post2ThingSpeak();
    lastUploadTime = millis();
  }
  */
  
}

// --- 以下為自定義功能函式 ---

void connectWiFi() 
{
  Serial.print("連線至 WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi 連線成功！ IP: " + WiFi.localIP().toString());
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("嘗試連接 Mosquitto Broker...");
    // 建立唯一的 Client ID
    String clientId = "ESP8266Client-Jim";
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("已連線！");
      // 連線成功後訂閱主題
      mqttClient.subscribe("test/mytopic"); 
    } else {
      Serial.print("失敗, 錯誤碼 rc=");
      Serial.print(mqttClient.state());
      Serial.println("，5秒後重試...");
      delay(5000);
    }
  }
}

void Post2ThingSpeak() 
{
  ThingSpeak.setField(1, (float)25.5); // 模擬溫度
  int x = ThingSpeak.writeFields(channelNumber, writeAPIKey);
  if (x == 200) {
    Serial.println("ThingSpeak 上傳成功！");
  } else {
    Serial.println("ThingSpeak 上傳失敗，代碼: " + String(x));
  }
}