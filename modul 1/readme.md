
Readme · MD
# Modifikasi Percobaan 1A
 
Program ini membaca data suhu dan kelembaban dari sensor **DHT11** menggunakan mikrokontroler (ESP32/Arduino-compatible), melakukan **5 kali pembacaan berturut-turut**, lalu menghitung **rata-rata** dari data yang valid sebelum ditampilkan ke Serial Monitor.
 
---
 
## 1. Library / Dependencies yang Diperlukan
 
| Library | Fungsi |
|---|---|
| **DHT sensor library** (by Adafruit) | Menyediakan fungsi untuk membaca data suhu dan kelembaban dari sensor DHT11/DHT22 |
| **Adafruit Unified Sensor** | Library pendukung (dependency) yang dibutuhkan oleh DHT sensor library agar dapat berjalan |
 
Kedua library ini bisa diinstal melalui **Library Manager** di Arduino IDE dengan mencari `DHT sensor library` (Adafruit).
 
---
 
## 2. Penjelasan Kode Baris per Baris
 
```cpp
#include <DHT.h>              // Mengimpor library DHT agar Serial dapat berkomunikasi dengan sensor DHT11
#define DHTPIN 4              // Menentukan pin GPIO 4 (D2) sebagai pin data sensor DHT11
#define DHTTYPE DHT11         // Memberi tahu library bahwa tipe sensor yang dipakai adalah DHT11
DHT dht(DHTPIN, DHTTYPE);     // Membuat objek "dht" dari class DHT dengan pin & tipe yang sudah ditentukan
```
 
### Fungsi `setup()`
Fungsi ini hanya dijalankan **sekali** saat mikrokontroler dinyalakan atau di-reset. Berisi inisialisasi awal.
 
```cpp
void setup() {
  Serial.begin(115200);   // Mengaktifkan komunikasi serial dengan baudrate 115200 bps
  dht.begin();             // Menginisialisasi sensor DHT11 agar siap dibaca
  Serial.println("Memulai akuisisi data sensor DHT11..."); // Menampilkan pesan awal sebagai penanda program mulai berjalan
}
```
 
### Fungsi `loop()`
Fungsi ini dijalankan **berulang-ulang secara terus-menerus** selama perangkat menyala. Di sinilah proses pembacaan 5x dan penghitungan rata-rata dilakukan.
 
```cpp
void loop() {
  float totalKelembaban = 0;   // Menyimpan akumulasi (jumlah) seluruh nilai kelembaban dari 5 pembacaan
  float totalSuhu = 0;         // Menyimpan akumulasi (jumlah) seluruh nilai suhu dari 5 pembacaan
  int jumlahDataValid = 0;     // Menghitung berapa banyak pembacaan yang BERHASIL (tidak gagal/NaN)
```
 
**Perulangan (loop) 5 kali pembacaan:**
```cpp
  for (int i = 0; i < 5; i++) {              // Perulangan akan berjalan 5 kali (i = 0,1,2,3,4)
    float kelembaban = dht.readHumidity();   // Membaca nilai kelembaban dari sensor
    float suhu = dht.readTemperature();      // Membaca nilai suhu dari sensor
```
 
**Percabangan (conditional) untuk validasi data:**
```cpp
    if (isnan(kelembaban) || isnan(suhu)) {
      // isnan() = "is not a number", mengecek apakah hasil pembacaan gagal (nilai tidak valid)
      // Operator || (OR) berarti: jika SALAH SATU dari suhu ATAU kelembaban gagal dibaca,
      // maka blok ini akan dijalankan
      Serial.println("Gagal membaca data dari sensor DHT11!"); // Menampilkan pesan error, data ini TIDAK dihitung
    } else {
      // Jika kedua data (suhu & kelembaban) berhasil dibaca (valid), maka:
      totalKelembaban += kelembaban;  // Menjumlahkan nilai kelembaban ke total (akumulasi)
      totalSuhu += suhu;              // Menjumlahkan nilai suhu ke total (akumulasi)
      jumlahDataValid++;              // Menambah counter data valid sebanyak 1
    }
    delay(2000); // Memberi jeda 2 detik sebelum pembacaan berikutnya (sesuai batas minimal DHT11)
  }
```
 
> **Kenapa perlu validasi `isnan()`?**
> Sensor DHT11 kadang gagal membaca (karena noise, timing, atau kabel kurang stabil) dan mengembalikan nilai `NaN`. Jika nilai ini ikut dijumlahkan tanpa filter, hasil rata-rata akan salah. Maka setiap data yang gagal **tidak dihitung**, dan hanya data valid yang dipakai untuk rata-rata.
 
**Percabangan untuk menghitung dan menampilkan hasil rata-rata:**
```cpp
  if (jumlahDataValid > 0) {
    // Blok ini hanya dijalankan JIKA minimal ada 1 data valid dari 5 pembacaan
    // Mencegah pembagian dengan nol (division by zero) yang bisa menyebabkan error/nilai tak terdefinisi
 
    float rataSuhu = totalSuhu / jumlahDataValid;
    // Menghitung rata-rata suhu = total suhu yang berhasil dibaca / jumlah data valid
 
    float rataKelembaban = totalKelembaban / jumlahDataValid;
    // Menghitung rata-rata kelembaban = total kelembaban yang berhasil dibaca / jumlah data valid
 
    Serial.println("HASIL RATA-RATA 5 PEMBACAAN ");   // Judul/header hasil di Serial Monitor
    Serial.print("Rata-rata Suhu: ");
    Serial.print(rataSuhu);                            // Menampilkan angka rata-rata suhu
    Serial.println(" °C");                             // Menampilkan satuan suhu
 
    Serial.print("Rata-rata Kelembaban: ");
    Serial.print(rataKelembaban);                      // Menampilkan angka rata-rata kelembaban
    Serial.println(" %");                              // Menampilkan satuan kelembaban
  }
 
  delay(2000); // Jeda sebelum siklus 5 pembacaan berikutnya dimulai kembali
}
```
 
---
# Modifikasi Percobaan 1B
 
Program ini membaca suhu dari sensor **DHT11** dan mengendalikan sebuah **aktuator (relay)** menggunakan logika **histerisis dua ambang batas**: aktuator **menyala** saat suhu di atas **30°C**, dan baru **mati** saat suhu turun di bawah **28°C**. Di antara kedua ambang batas tersebut (28°C–30°C), kondisi aktuator dipertahankan seperti sebelumnya.
 
---
 
## 1. Library / Dependencies yang Diperlukan
 
| Library | Fungsi |
|---|---|
| **DHT sensor library** (by Adafruit) | Menyediakan fungsi untuk membaca data suhu dari sensor DHT11/DHT22 |
| **Adafruit Unified Sensor** | Library pendukung (dependency) yang dibutuhkan oleh DHT sensor library agar dapat berjalan |
 
Kedua library ini bisa diinstal melalui **Library Manager** di Arduino IDE dengan mencari `DHT sensor library` (Adafruit).
 
---
 
## 2. Penjelasan Kode Baris per Baris
 
### Header, Pin, dan Objek Sensor
```cpp
#include <DHT.h>              // Mengimpor library DHT agar dapat berkomunikasi dengan sensor DHT11
#define DHTPIN 4              // Menentukan pin GPIO 4 (D2) sebagai pin data sensor DHT11
#define DHTTYPE DHT11         // Memberi tahu library bahwa tipe sensor yang dipakai adalah DHT11
#define RELAYPIN 14           // Menentukan pin GPIO 14 sebagai pin output untuk mengendalikan relay/aktuator
 
DHT dht(DHTPIN, DHTTYPE);     // Membuat objek "dht" dari class DHT dengan pin & tipe sensor yang sudah ditentukan
```
 
### Deklarasi Ambang Batas (Histerisis)
```cpp
const float batasAtas = 30.0;   // Ambang batas ATAS: aktuator akan MENYALA jika suhu > 30°C
const float batasBawah = 28.0;  // Ambang batas BAWAH: aktuator akan MATI jika suhu < 28°C
```
> Kata kunci `const` berarti nilai ini **tetap** dan tidak boleh diubah selama program berjalan, sehingga aman digunakan sebagai acuan pembanding suhu.
 
### Fungsi `setup()`
Dijalankan **sekali** saat mikrokontroler dinyalakan atau di-reset. Berisi inisialisasi awal.
 
```cpp
void setup() {
  Serial.begin(115200);        // Mengaktifkan komunikasi serial dengan baudrate 115200 bps
  dht.begin();                  // Menginisialisasi sensor DHT11 agar siap dibaca
  pinMode(RELAYPIN, OUTPUT);    // Mengatur pin relay sebagai output digital
  digitalWrite(RELAYPIN, LOW);  // Memastikan relay dalam kondisi MATI saat program pertama kali berjalan (aman saat start-up)
}
```
 
### Fungsi `loop()`
Dijalankan **berulang-ulang secara terus-menerus** selama perangkat menyala. Di sinilah proses pembacaan suhu dan pengambilan keputusan histerisis dilakukan.
 
```cpp
void loop() {
  float suhu = dht.readTemperature();   // Membaca nilai suhu terbaru dari sensor DHT11
```
 
**Percabangan (conditional) untuk validasi data:**
```cpp
  if (isnan(suhu)) {
    // isnan() = "is not a number", mengecek apakah hasil pembacaan gagal (nilai tidak valid)
    Serial.println("Gagal membaca sensor!");   // Menampilkan pesan error jika pembacaan gagal
  } else {
    // Jika suhu berhasil dibaca (valid), tampilkan nilainya:
    Serial.print("Suhu: ");
    Serial.print(suhu);                        // Menampilkan angka suhu hasil pembacaan
    Serial.println(" °C");                     // Menampilkan satuan suhu
```
 
**Percabangan inti — logika histerisis dua ambang batas:**
```cpp
    if (suhu > batasAtas) {
      // Jika suhu MELEBIHI 30°C
      digitalWrite(RELAYPIN, HIGH);    // Menyalakan relay (mengirim sinyal HIGH ke pin relay)
      Serial.println("Aktuator: ON");  // Menampilkan status aktuator menyala
 
    } else if (suhu < batasBawah) {
      // Jika suhu TURUN di bawah 28°C
      digitalWrite(RELAYPIN, LOW);     // Mematikan relay (mengirim sinyal LOW ke pin relay)
      Serial.println("Aktuator: OFF"); // Menampilkan status aktuator mati
    }
    // Tidak ada blok "else" terakhir di sini secara sengaja:
    // Jika suhu berada di ANTARA 28°C dan 30°C, program TIDAK mengubah status relay,
    // sehingga kondisi aktuator tetap seperti sebelumnya (inilah efek "histerisis")
  }
 
  delay(2000); // Menunggu 2 detik sebelum melakukan pembacaan suhu berikutnya
}
```
 
---
