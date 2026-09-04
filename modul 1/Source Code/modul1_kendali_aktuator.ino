#include <DHT.h>
#define DHTPIN 4 // pin data DHT22 terhubung ke GPIO 4
#define DHTTYPE DHT11
#define RELAYPIN 13 // pin kendali relay/LED indikator

DHT dht(DHTPIN, DHTTYPE);

const float suhuThreshold = 30.0; // ambang batas suhu (°C)

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, LOW); // pastikan aktuator mati diawal
}
void loop() {
  float suhu = dht.readTemperature();
  if (isnan(suhu)) {
    Serial.println("Gagal membaca data sensor!");
  } else {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print(" °C -> ");
  // Kendali aktuator berdasarkan hasil akuisisi data sensor
    if (suhu > suhuThreshold) {
      digitalWrite(RELAYPIN, HIGH); // aktifkan relay/LED
      Serial.println("Aktuator: ON");
    } else {
      digitalWrite(RELAYPIN, LOW); // matikan relay/LED
      Serial.println("Aktuator: OFF");
    }
  }
}