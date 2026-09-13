# Modifikasi Percobaan 2A Program ESP8266 dengan Auto-Reconnect WiFi
 

 
## 1. Library / Dependencies yang Diperlukan
 
| Library | Fungsi |
|---|---|
| **ESP8266WiFi.h** | Library bawaan (built-in) dari board package ESP8266 untuk Arduino IDE. Menyediakan fungsi-fungsi untuk mengatur mode WiFi, memulai koneksi, mengecek status, dan mengambil informasi jaringan (IP, MAC, RSSI). |
 
> Library ini otomatis tersedia setelah menginstal **ESP8266 board package** di Arduino IDE (melalui Boards Manager, mencari "esp8266"). Tidak perlu instalasi tambahan secara terpisah.
 
---
 
## 2. Penjelasan Kode Baris per Baris
 
### Deklarasi Variabel Global
```cpp
#include <ESP8266WiFi.h>        // Mengimpor library WiFi khusus untuk chip ESP8266
 
const char* ssid = "refan";      // Nama (SSID) jaringan WiFi yang akan disambungkan
const char* password = "anakpphmpp"; // Password/kata sandi jaringan WiFi tersebut
 
const int ledPin = 2;            // Menentukan pin GPIO 2 (biasanya LED bawaan board) sebagai indikator status koneksi
```
 
### Fungsi `setup()`
Dijalankan sekali saat ESP8266 dinyalakan atau di-reset. Bertugas menyiapkan koneksi WiFi awal.
 
```cpp
void setup() {
  Serial.begin(115200);          // Mengaktifkan komunikasi serial dengan baudrate 115200 bps
  pinMode(ledPin, OUTPUT);       // Mengatur pin LED sebagai output digital
  digitalWrite(ledPin, LOW);     // Memastikan LED dalam kondisi mati saat program pertama kali berjalan
 
  WiFi.mode(WIFI_STA);           // Mengatur ESP8266 dalam mode Station (STA), yaitu terhubung SEBAGAI CLIENT ke router/access point (bukan sebagai hotspot)
  WiFi.begin(ssid, password);    // Memulai proses koneksi ke WiFi menggunakan SSID dan password yang sudah didefinisikan
 
  Serial.print("Menghubungkan ke WiFi");
 
  while (WiFi.status() != WL_CONNECTED) {
    // Perulangan ini akan terus berjalan SELAMA status WiFi BELUM "WL_CONNECTED" (belum tersambung)
    delay(500);                  // Jeda 0.5 detik antar pengecekan, agar tidak membebani prosesor
    Serial.print(".");           // Menampilkan tanda titik sebagai indikator visual "sedang mencoba menyambung"
  }
 
  Serial.println();
  Serial.println("WiFi berhasil terhubung!");  // Ditampilkan setelah keluar dari while, artinya koneksi sudah berhasil
 
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());     // Menampilkan alamat IP yang didapat ESP8266 dari router (DHCP)
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());  // Menampilkan alamat MAC unik dari modul ESP8266
  Serial.print("RSSI (dBm) : ");
  Serial.println(WiFi.RSSI());        // Menampilkan kekuatan sinyal WiFi dalam satuan dBm (semakin mendekati 0, sinyal semakin kuat)
 
  digitalWrite(ledPin, HIGH);   // Menyalakan LED sebagai penanda bahwa koneksi WiFi berhasil
}
```
 
### Fungsi `loop()` — Bagian yang Dimodifikasi untuk Auto-Reconnect
Dijalankan **berulang-ulang terus-menerus**. Di sinilah logika pengecekan koneksi dan reconnect otomatis ditambahkan.
 
```cpp
void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // PERCABANGAN: mengecek apakah ESP8266 masih terhubung ke WiFi saat ini
    Serial.println("Status: Terhubung");
    digitalWrite(ledPin, HIGH);   // LED tetap menyala selama koneksi masih aktif
 
  } else {
    // Kondisi ini terjadi ketika koneksi WiFi TERPUTUS (misalnya router mati, sinyal hilang, dsb.)
    Serial.println("Status: Terputus");
    digitalWrite(ledPin, LOW);    // LED dimatikan sebagai penanda koneksi terputus
 
    Serial.println("Mencoba menghubungkan kembali...");
 
    WiFi.disconnect();            // BARIS BARU: memutuskan sesi WiFi yang lama secara eksplisit, membersihkan status koneksi sebelum mencoba ulang
    WiFi.begin(ssid, password);   // BARIS BARU: memulai kembali proses koneksi ke WiFi yang sama (proses reconnect)
 
    unsigned long waktuMulai = millis();
    // BARIS BARU: mencatat waktu (dalam milidetik sejak ESP8266 menyala) saat proses reconnect dimulai,
    // digunakan sebagai referensi untuk menghitung batas waktu percobaan (timeout)
 
    while (WiFi.status() != WL_CONNECTED && millis() - waktuMulai < 10000) {
      // BARIS BARU — PERULANGAN DENGAN TIMEOUT:
      // Akan terus mencoba SELAMA status WiFi belum "WL_CONNECTED"
      // DAN selama waktu yang telah berlalu (millis() - waktuMulai) belum mencapai 10000 ms (10 detik)
      // Kombinasi dua syarat dengan && (AND) ini mencegah program "macet" tak terbatas
      // jika WiFi ternyata tidak bisa tersambung sama sekali
      delay(500);                 // Jeda 0.5 detik antar percobaan
      Serial.print(".");          // Indikator visual proses reconnect sedang berlangsung
    }
    Serial.println();
 
    if (WiFi.status() == WL_CONNECTED) {
      // BARIS BARU: setelah keluar dari while, cek apakah reconnect BERHASIL sebelum waktu 10 detik habis
      Serial.println("Reconnect berhasil!");
      Serial.print("IP Address : ");
      Serial.println(WiFi.localIP());  // Menampilkan kembali IP address yang baru didapat
      digitalWrite(ledPin, HIGH);      // Menyalakan kembali LED sebagai penanda koneksi pulih
 
    } else {
      // BARIS BARU: jika 10 detik habis dan status masih belum WL_CONNECTED, berarti reconnect gagal
      Serial.println("Reconnect gagal.");
      // LED tetap dalam keadaan LOW (mati), dan program akan mencoba lagi pada iterasi loop() berikutnya
    }
  }
 
  delay(5000);   // Jeda 5 detik sebelum pengecekan status WiFi dilakukan kembali dari awal
}
```
 
