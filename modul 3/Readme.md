# Modifikasi Percobaan 3A Menambahkan Data Waktu (millis()) ke JSON
 
Program ini mengirim data sensor (suhu dan kelembaban) dari ESP8266 ke server melalui **HTTP POST** dalam format **JSON**. Modifikasi pada bagian ini menambahkan satu field baru, yaitu **`waktu`**, yang berisi jumlah milidetik sejak ESP8266 menyala (`millis()`), sehingga setiap data yang dikirim memiliki penanda waktu relatif (timestamp lokal perangkat).
 
---
 
## 1. Library / Dependencies yang Diperlukan
 
| Library | Fungsi |
|---|---|
| **ESP8266WiFi.h** | Library bawaan board package ESP8266, mengatur koneksi WiFi (mode Station, status koneksi, dsb.) |
| **ESP8266HTTPClient.h** | Menyediakan class `HTTPClient` untuk melakukan request HTTP (GET/POST) dari ESP8266 ke server |
| **WiFiClient.h** | Menyediakan class `WiFiClient` sebagai "jalur" koneksi TCP dasar yang dipakai oleh `HTTPClient` |
| **ArduinoJson.h** | Library pihak ketiga (by Benoit Blanchon) untuk membuat, mengisi, dan mengubah data menjadi format JSON (`JsonDocument`, `serializeJson()`) |
 
 
---
 
## 2. Kode  Modifikasi
 
 
```cpp
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
 
const char* ssid     = "hammed";
const char* password = "kudalari";
const char* serverUrl = "http://httpbin.org/post"; // Digunakan HTTP biasa untuk menghindari verifikasi SSL/TLS pada ESP8266
 
void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Menghubungkan ke WiFi");
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println();
  Serial.println("WiFi berhasil terhubung!");
}
 
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
 
    // Inisialisasi HTTP client dengan WiFiClient
    http.begin(client, serverUrl);
    http.addHeader("Content-Type", "application/json");
 
    // Membuat objek data sensor dalam format JSON
    JsonDocument doc;
    doc["suhu"] = 28.5;              // data suhu (°C)
    doc["kelembaban"] = 65.0;        // data kelembaban (%)
    doc["waktu"] = millis();         // <-- BARIS BARU: menambahkan data waktu (dalam milidetik sejak ESP8266 menyala)
 
    String requestBody;
    serializeJson(doc, requestBody);
 
    Serial.print("Mengirim data: ");
    Serial.println(requestBody);
 
    // Mengirim data melalui HTTP POST
    int httpResponseCode = http.POST(requestBody);
 
    if (httpResponseCode > 0) {
      Serial.print("Kode Response HTTP: ");
      Serial.println(httpResponseCode);
      Serial.println("Isi Response:");
      Serial.println(http.getString());
    } else {
      Serial.print("Pengiriman gagal, kode error: ");
      Serial.println(httpResponseCode);
    }
 
    http.end();
  }
 
  delay(10000); // kirim data setiap 10 detik
}
```
 
Penambahan hanya terjadi pada **satu baris** di dalam fungsi `loop()`, tepat setelah data suhu dan kelembaban dimasukkan ke dalam `JsonDocument`:
 
```cpp
doc["waktu"] = millis();   // BARIS BARU
```
 
---
 
## 3. Penjelasan Baris yang Ditambahkan
 
```cpp
doc["waktu"] = millis();
```
 
- **`millis()`** — Fungsi bawaan Arduino/ESP8266 yang mengembalikan jumlah **milidetik** yang telah berlalu sejak papan (board) terakhir kali dinyalakan atau di-*reset*. Nilainya bertipe `unsigned long` dan akan terus bertambah selama perangkat menyala (akan "berputar" kembali ke 0 setelah ± 49 hari, kasus yang jarang terjadi pada perangkat sensor biasa).
- **`doc["waktu"] = ...`** — Menambahkan **key (kunci) baru bernama `"waktu"`** ke dalam objek JSON (`doc`) yang sedang dibangun, dengan nilai berupa hasil dari `millis()`. Setelah baris ini dijalankan, objek JSON akan memiliki tiga field: `suhu`, `kelembaban`, dan `waktu`.
- Baris ini diletakkan **sebelum** `serializeJson(doc, requestBody);` sehingga field `waktu` ikut terkonversi menjadi teks JSON dan ikut terkirim ke server bersama data suhu dan kelembaban.
### Contoh hasil JSON yang dikirim setelah modifikasi
```json
{
  "suhu": 28.5,
  "kelembaban": 65.0,
  "waktu": 123456
}
```
Angka `123456` di atas berarti data ini dikirim pada saat ESP8266 sudah menyala selama 123.456 detik (≈ 2 menit 3 detik).
 
---
