#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

// Configuration WiFi Wokwi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// Configuration MQTT Broker
const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
const char* topicData = "unsoed/tk245004/kelompokrefan/perintah"; // Samakan topic ini dengan di Python

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 2000; // Kirim data setiap 2 detik sesuai Dapeng

// Fungsi Simulasi Suhu Mock dari Modul 5A
float bacaSuhuMock() {
  static unsigned long jumlahBaca = 0;
  jumlahBaca++;
  float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);
  float noise = random(-5, 6) * 0.1;
  float suhu = suhuDasar + noise;

  if (jumlahBaca % 25 == 0) {
    suhu += 8.0; // Lonjakan suhu / Anomali
  }
  return suhu;
}

void hubungkanWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
}

void hubungkanMQTT() {
  while (!client.connected()) {
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("Terhubung ke MQTT Broker!");
    } else {
      Serial.print("Gagal terhubung, rc=");
      Serial.print(client.state());
      Serial.println(" coba lagi dalam 2 detik");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  randomSeed(micros());
  hubungkanWiFi();
  client.setServer(mqttServer, mqttPort);
}

void loop() {
  if (!client.connected()) {
    hubungkanMQTT();
  }
  client.loop(); // Memproses tugas komunikasi MQTT

  // Non-blocking delay menggunakan millis() sesuai Modul 4
  if (millis() - waktuTerakhirPublish > intervalPublish) {
    waktuTerakhirPublish = millis();

    float suhu = bacaSuhuMock();

    // Format JSON menggunakan ArduinoJson sesuai Modul 4
    JsonDocument doc;
    doc["suhu"] = suhu;

    char buffer[128];
    serializeJson(doc, buffer);

    // Publish data JSON ke MQTT
    client.publish(topicData, buffer);

    Serial.print("Data terkirim ke MQTT: ");
    Serial.println(buffer);
  }
}