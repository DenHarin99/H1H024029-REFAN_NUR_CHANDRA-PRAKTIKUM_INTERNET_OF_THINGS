#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// Konfigurasi WiFi dan Broker MQTT
const char* ssid          = "hammed";
const char* password      = "kudalari";
const char* mqttServer    = "broker.hivemq.com";
const int mqttPort        = 1883;
const char* topicData     = "unsoed/tk245004/kelompokrefan/data";
const char* topicPerintah = "unsoed/tk245004/kelompokrefan/perintah";

// Konfigurasi Sensor DHT dan LED
#define DHTPIN 14
#define DHTTYPE DHT11
const int ledPin = 5;

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// Variabel Waktu untuk Non-Blocking Delay (millis)
unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000; // Publish data setiap 5 detik

// Fungsi Callback untuk Memproses Perintah Masuk
void callback(char* topic, byte* payload, unsigned int length) {
  String pesan;
  for (unsigned int i = 0; i < length; i++) pesan += (char)payload[i];

  JsonDocument doc;
  if (deserializeJson(doc, pesan)) return; // Abaikan jika parsing gagal

  const char* perintah = doc["perintah"];
  
  // Kendali aktuator menggunakan Ternary Operator
  digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
  
  Serial.print("Perintah diterima -> Aktuator: ");
  Serial.println(perintah);
}

// Fungsi Koneksi WiFi
void hubungkanWiFi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Serial.println("WiFi berhasil terhubung!");
}

// Fungsi Koneksi MQTT dan Auto-Subscribe
void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP8266Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      client.subscribe(topicPerintah);
      Serial.println("Terhubung dan subscribe topic perintah");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  dht.begin();

  hubungkanWiFi();
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    hubungkanMQTT();
  }
  
  // Memproses komunikasi MQTT dan pesan masuk secara terus-menerus
  client.loop();

  // Task non-blocking untuk mempublikasikan data sensor secara berkala
  if (millis() - waktuTerakhirPublish > intervalPublish) {
    waktuTerakhirPublish = millis();
    
    float suhu = dht.readTemperature();
    
    if (!isnan(suhu)) {
      JsonDocument doc;
      doc["suhu"] = suhu;
      
      char buffer[128];
      serializeJson(doc, buffer);
      
      client.publish(topicData, buffer);
      
      Serial.print("Data terkirim: ");
      Serial.println(buffer);
    }
  }
}