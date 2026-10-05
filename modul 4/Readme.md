# Modifikasi Percobaan 4A Menambahkan Nilai Intensitas (PWM) pada JSON

Program ini menerima perintah dari **MQTT Broker** dalam format **JSON** pada ESP8266 untuk mengendalikan LED di GPIO5 (D1). Modifikasi pada bagian ini menambahkan satu field baru pada pesan JSON, yaitu **`intensitas`**, yang berisi nilai 0 sampai 255 untuk mengatur kecerahan LED menggunakan **PWM** (*Pulse Width Modulation*) melalui `analogWrite()`.

Contoh pesan yang dikirim lewat MQTT Explorer:

```json
{"perintah": "ON", "intensitas": 200}
```

---

## 1. Library / Dependencies yang Diperlukan

| Library | Fungsi |
|---|---|
| **ESP8266WiFi.h** | Library bawaan board package ESP8266, mengatur koneksi WiFi (mode Station, status koneksi, dsb.) |
| **PubSubClient.h** | Library pihak ketiga (by Nick O'Leary) untuk komunikasi MQTT: koneksi ke broker, `subscribe()`, dan menjalankan fungsi `callback()` saat pesan masuk |
| **ArduinoJson.h** | Library pihak ketiga (by Benoit Blanchon) untuk membaca (*deserialize*) teks JSON menjadi objek yang bisa diakses per key (`JsonDocument`, `deserializeJson()`). Kode ini memakai **ArduinoJson v7** |

---

## 2. Kode Modifikasi

```cpp
#include <ESP8266WiFi.h>       // Library untuk koneksi WiFi ESP8266
#include <PubSubClient.h>      // Library untuk komunikasi MQTT
#include <ArduinoJson.h>       // Library untuk membaca data JSON

// Nama jaringan WiFi
const char* ssid = "hammed";

// Password WiFi
const char* password = "kudalari";

// Alamat MQTT Broker
// Broker berfungsi sebagai perantara komunikasi MQTT
const char* mqttServer = "broker.hivemq.com";

// Port MQTT standar tanpa enkripsi
const int mqttPort = 1883;

// Topic yang digunakan untuk menerima perintah
// dari MQTT Broker
const char* topicPerintah =
    "unsoed/tk245004/kelompokrefan/perintah";

// Pin LED yang digunakan adalah GPIO5
// Pada NodeMCU ESP8266, GPIO5 biasanya merupakan pin D1
const int ledPin = 5;

// [BARU] Nilai PWM maksimum (resolusi 8-bit: 0 sampai 255)
const int PWM_MAKS = 255;

// Objek untuk koneksi jaringan menggunakan WiFi
WiFiClient espClient;

// Objek MQTT Client
// espClient digunakan sebagai koneksi dasar MQTT
PubSubClient client(espClient);

// Fungsi callback akan otomatis dijalankan ketika
// ESP8266 menerima pesan dari topic yang di-subscribe.
//
// Parameter:
// topic   = nama topic MQTT
// payload = isi pesan MQTT dalam bentuk byte
// length  = panjang pesan
void callback(
  char* topic,
  byte* payload,
  unsigned int length
) {
  // Variabel untuk menyimpan pesan MQTT
  String pesan = "";

  // Payload MQTT berupa array byte.
  // Setiap byte dikonversi menjadi karakter
  // kemudian digabungkan menjadi String.
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  // Menampilkan informasi pesan yang diterima
  // pada Serial Monitor
  Serial.print("Pesan diterima [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(pesan);

  // Membuat objek untuk menampung data JSON
  JsonDocument doc;

  // Mencoba melakukan parsing terhadap pesan JSON
  DeserializationError error =
      deserializeJson(doc, pesan);

  // Jika terjadi kesalahan saat parsing JSON
  if (error) {
    // Menampilkan jenis error
    Serial.print("Gagal parsing JSON: ");
    Serial.println(error.c_str());

    // Menghentikan callback
    // karena pesan tidak dapat diproses
    return;
  }

  // Mengambil nilai dari key "perintah"
  //
  // Contoh pesan:
  // {"perintah":"ON","intensitas":200}
  //
  // Maka nilai perintah = "ON"
  const char* perintah = doc["perintah"];

  // [BARU] Mengambil nilai key "intensitas".
  // Operator "|" memberi nilai default (PWM_MAKS) jika key
  // "intensitas" tidak ada atau bukan bilangan bulat.
  int intensitas = doc["intensitas"] | PWM_MAKS;

  // [BARU] Membatasi nilai intensitas agar selalu 0 sampai 255
  intensitas = constrain(intensitas, 0, PWM_MAKS);

  // Jika perintah yang diterima adalah "ON"
  if (String(perintah) == "ON") {
    // [UBAH] Mengatur kecerahan LED dengan PWM
    // (sebelumnya: digitalWrite(ledPin, HIGH))
    analogWrite(ledPin, intensitas);

    // Menampilkan status pada Serial Monitor
    // [UBAH] Ditambah informasi intensitas dan persentase
    Serial.print("Aktuator: ON, intensitas = ");
    Serial.print(intensitas);
    Serial.print(" (");
    Serial.print(map(intensitas, 0, PWM_MAKS, 0, 100));
    Serial.println("%)");

  // Jika perintah yang diterima adalah "OFF"
  } else if (String(perintah) == "OFF") {
    // [UBAH] Mematikan LED dengan PWM duty cycle 0
    // (sebelumnya: digitalWrite(ledPin, LOW))
    analogWrite(ledPin, 0);

    // Menampilkan status pada Serial Monitor
    Serial.println("Aktuator: OFF");
  }
}

void hubungkanWiFi() {
  // Memulai koneksi ke jaringan WiFi
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan ke WiFi");

  // Selama belum berhasil terhubung,
  // program akan terus menunggu
  while (WiFi.status() != WL_CONNECTED) {
    // Tunggu 500 milidetik
    delay(500);

    // Menampilkan titik sebagai indikator proses koneksi
    Serial.print(".");
  }

  // Jika koneksi berhasil
  Serial.println("\nWiFi berhasil terhubung!");
}

void hubungkanMQTT() {
  // Selama ESP8266 belum terhubung ke MQTT Broker
  while (!client.connected()) {
    Serial.print("Menghubungkan ke broker MQTT...");

    // Membuat Client ID secara acak
    //
    // Contoh:
    // ESP8266Client-a35f
    //
    // ID dibuat acak agar tidak mudah sama
    // dengan client MQTT lainnya.
    String clientId =
        "ESP8266Client-" +
        String(random(0xffff), HEX);

    // Mencoba melakukan koneksi ke MQTT Broker
    if (client.connect(clientId.c_str())) {
      Serial.println("berhasil terhubung!");

      // Setelah berhasil terhubung,
      // ESP8266 melakukan subscribe ke topic perintah
      client.subscribe(topicPerintah);

      // Menampilkan topic yang digunakan
      Serial.print("Subscribe ke topic: ");
      Serial.println(topicPerintah);

    } else {
      // Jika koneksi gagal,
      // tampilkan kode status/error MQTT
      Serial.print("gagal, rc=");
      Serial.print(client.state());

      // Memberikan informasi bahwa ESP8266
      // akan mencoba kembali
      Serial.println(" coba lagi dalam 2 detik");

      // Tunggu 2 detik sebelum mencoba kembali
      delay(2000);
    }
  }
}

// Fungsi setup hanya dijalankan satu kali
// ketika ESP8266 pertama kali dinyalakan
void setup() {
  // Memulai komunikasi Serial
  // Baud rate = 115200
  Serial.begin(115200);

  // Mengatur pin LED sebagai OUTPUT
  pinMode(ledPin, OUTPUT);

  // [BARU] Mengatur rentang PWM ESP8266 menjadi 0-255
  // (bawaan ESP8266 adalah 0-1023)
  analogWriteRange(PWM_MAKS);

  // [UBAH] Memastikan LED mati saat pertama dinyalakan
  // (sebelumnya: digitalWrite(ledPin, LOW))
  analogWrite(ledPin, 0);

  // Menghubungkan ESP8266 ke WiFi
  hubungkanWiFi();

  // Menentukan alamat MQTT Broker
  // dan port yang digunakan
  client.setServer(mqttServer, mqttPort);

  // Menentukan fungsi callback
  // untuk memproses pesan MQTT yang masuk
  client.setCallback(callback);
}

// Fungsi loop dijalankan terus-menerus
// selama ESP8266 masih menyala
void loop() {
  // Mengecek apakah koneksi MQTT masih aktif
  if (!client.connected()) {
    // Jika terputus, lakukan koneksi ulang
    hubungkanMQTT();
  }

  // Menjalankan proses komunikasi MQTT
  //
  // Fungsi ini penting karena digunakan untuk:
  // 1. Memeriksa pesan MQTT yang masuk
  // 2. Menjalankan callback()
  // 3. Menjaga komunikasi MQTT tetap berjalan
  client.loop();
}
```

Penambahan terjadi pada **tiga bagian** program, yang ditandai komentar `[BARU]` (baris baru) dan `[UBAH]` (baris lama yang diganti):

1. **Bagian global**, konstanta batas PWM:

```cpp
const int PWM_MAKS = 255;   // BARIS BARU
```

2. **Fungsi `setup()`**, konfigurasi rentang PWM dan kondisi awal LED:

```cpp
analogWriteRange(PWM_MAKS);   // BARIS BARU
analogWrite(ledPin, 0);       // DIUBAH dari digitalWrite(ledPin, LOW)
```

3. **Fungsi `callback()`**, membaca `intensitas` dan memakainya untuk PWM:

```cpp
int intensitas = doc["intensitas"] | PWM_MAKS;          // BARIS BARU
intensitas = constrain(intensitas, 0, PWM_MAKS);        // BARIS BARU
analogWrite(ledPin, intensitas);                        // DIUBAH dari digitalWrite(ledPin, HIGH)
analogWrite(ledPin, 0);                                 // DIUBAH dari digitalWrite(ledPin, LOW) pada perintah OFF
```

---

## 3. Penjelasan Baris yang Ditambahkan

### 3.1 Konstanta PWM maksimum

```cpp
const int PWM_MAKS = 255;
```

- **`const int PWM_MAKS = 255;`** — Membuat konstanta bernilai 255 sebagai batas atas intensitas (resolusi 8-bit, rentang 0 sampai 255). Dengan konstanta, angka 255 cukup ditulis di satu tempat dan dipakai di `setup()` maupun `callback()`.

### 3.2 Konfigurasi PWM di `setup()`

```cpp
analogWriteRange(PWM_MAKS);
analogWrite(ledPin, 0);
```

- **`analogWriteRange(PWM_MAKS);`** — Mengubah rentang nilai `analogWrite()` pada ESP8266 dari bawaan **0 sampai 1023** menjadi **0 sampai 255**. Tanpa baris ini, nilai `intensitas: 200` hanya sekitar 20% dari kecerahan penuh (200 dari 1023). Fungsi ini harus dipanggil sebelum `analogWrite()` pertama.
- **`analogWrite(ledPin, 0);`** — Menggantikan `digitalWrite(ledPin, LOW)`. Duty cycle 0% berarti LED mati saat ESP8266 pertama kali dinyalakan, dan pin sudah dikendalikan secara PWM sejak awal.

### 3.3 Membaca dan membatasi intensitas di `callback()`

```cpp
int intensitas = doc["intensitas"] | PWM_MAKS;
intensitas = constrain(intensitas, 0, PWM_MAKS);
```

- **`doc["intensitas"]`** — Mengambil nilai key `"intensitas"` dari objek JSON hasil `deserializeJson()`, dengan cara yang sama seperti `doc["perintah"]`.
- **`| PWM_MAKS`** — Operator `|` pada ArduinoJson berarti **nilai default**. Jika key `intensitas` tidak ada di pesan, atau isinya bukan bilangan bulat (misalnya string `"200"`), maka variabel `intensitas` bernilai `PWM_MAKS` (255). Akibatnya pesan lama `{"perintah":"ON"}` tetap berfungsi dengan LED menyala penuh.
- **`constrain(intensitas, 0, PWM_MAKS)`** — Fungsi bawaan Arduino yang membatasi nilai agar tetap di antara 0 dan 255. Nilai lebih dari 255 (misalnya 300) menjadi 255, dan nilai negatif (misalnya -10) menjadi 0, sehingga PWM tidak menerima nilai di luar rentang.

### 3.4 Perintah `ON` memakai PWM

```cpp
analogWrite(ledPin, intensitas);

Serial.print("Aktuator: ON, intensitas = ");
Serial.print(intensitas);
Serial.print(" (");
Serial.print(map(intensitas, 0, PWM_MAKS, 0, 100));
Serial.println("%)");
```

- **`analogWrite(ledPin, intensitas);`** — Menggantikan `digitalWrite(ledPin, HIGH)`. Menghasilkan sinyal PWM pada GPIO5 dengan **duty cycle = intensitas / 255**. Semakin besar nilainya, semakin lama pin berlogika HIGH dalam satu siklus, sehingga LED tampak lebih terang.
- **`Serial.print("Aktuator: ON, intensitas = ");`** — Mencetak teks status awal ke Serial Monitor.
- **`Serial.print(intensitas);`** — Mencetak nilai intensitas yang benar-benar dipakai (setelah melewati `constrain`), sehingga mudah dicek saat pengujian.
- **`Serial.print(" (");`** — Mencetak tanda kurung pembuka sebagai pembungkus persentase.
- **`map(intensitas, 0, PWM_MAKS, 0, 100)`** — Fungsi bawaan Arduino yang mengubah skala 0 sampai 255 menjadi 0 sampai 100 (persen). Hasilnya bilangan bulat sehingga bagian desimal dibuang (200 menjadi 78, bukan 78,4).
- **`Serial.println("%)");`** — Mencetak tanda persen dan kurung penutup, lalu pindah baris.

### 3.5 Perintah `OFF` memakai PWM

```cpp
analogWrite(ledPin, 0);
```

- **`analogWrite(ledPin, 0);`** — Menggantikan `digitalWrite(ledPin, LOW)`. Duty cycle 0% mematikan LED. Perintah `OFF` tidak membaca nilai intensitas, sehingga field `intensitas` pada pesan `OFF` diabaikan.

### Catatan: `analogWrite()` vs `ledcWrite()`

`ledcWrite()` hanya tersedia pada **ESP32**. Karena board yang dipakai adalah ESP8266, fungsi PWM yang digunakan adalah **`analogWrite()`**. GPIO5 (D1) mendukung PWM perangkat lunak pada ESP8266 dengan frekuensi bawaan 1 kHz.

### Contoh hasil setelah modifikasi

Pesan JSON yang dikirim dari MQTT Explorer:

```json
{
  "perintah": "ON",
  "intensitas": 200
}
```

Output pada Serial Monitor:

```
Pesan diterima [unsoed/tk245004/kelompokrefan/perintah]: {"perintah":"ON","intensitas":200}
Aktuator: ON, intensitas = 200 (78%)
```

Angka `200` berarti duty cycle = 200 / 255 ≈ 78%, sehingga LED menyala dengan kecerahan sekitar 78% dari maksimum.

---

# Modifikasi Percobaan 4B Menambahkan Topic Perintah Kedua (Buzzer)

Program ini mengirim data suhu dummy dari ESP8266 ke **MQTT Broker** dan menerima perintah dalam format **JSON**. Pada versi awal, ESP8266 hanya men-*subscribe* satu topic (`.../perintah`) untuk mengendalikan LED. Modifikasi pada bagian ini menambahkan **topic perintah baru** (`.../buzzer`) untuk mengendalikan **aktuator kedua berupa buzzer**, dan membuat fungsi `callback()` mampu **membedakan topic mana yang menerima pesan**.

Kedua topic memakai format pesan yang sama, tetapi aktuator yang bereaksi ditentukan oleh topic-nya:

| Topic | Aktuator | Contoh pesan |
|---|---|---|
| `unsoed/tk245004/kelompokrefan/perintah` | LED (GPIO5 / D1) | `{"perintah":"ON"}` |
| `unsoed/tk245004/kelompokrefan/buzzer` | Buzzer (GPIO4 / D2) | `{"perintah":"ON"}` |

---

## 1. Library / Dependencies yang Diperlukan

Tidak ada library baru. Modifikasi ini memakai library yang sama dengan program sebelumnya.

| Library | Fungsi |
|---|---|
| **ESP8266WiFi.h** | Library bawaan board package ESP8266, mengatur koneksi WiFi (mode Station, status koneksi, dsb.) |
| **PubSubClient.h** | Library pihak ketiga (by Nick O'Leary) untuk komunikasi MQTT: koneksi ke broker, `publish()`, `subscribe()`, dan menjalankan `callback()` saat pesan masuk |
| **ArduinoJson.h** | Library pihak ketiga (by Benoit Blanchon) untuk membuat dan membaca data JSON (`JsonDocument`, `serializeJson()`, `deserializeJson()`). Kode ini memakai **ArduinoJson v7** |

## Rangkaian Tambahan

| Komponen | Pin ESP8266 |
|---|---|
| Buzzer (kaki positif) | GPIO4 (D2) |
| Buzzer (kaki negatif) | GND |

---

## 2. Kode Modifikasi

```cpp
#include <ESP8266WiFi.h>       // Library untuk koneksi WiFi ESP8266
#include <PubSubClient.h>      // Library untuk komunikasi MQTT
#include <ArduinoJson.h>       // Library untuk membuat data dalam format JSON

const char* ssid = "hammed";                  // Nama jaringan WiFi
const char* password = "kudalari";            // Password WiFi
const char* mqttServer = "broker.hivemq.com"; // Alamat MQTT Broker
const int mqttPort = 1883;                    // Port 1883 merupakan MQTT tanpa enkripsi TLS

// Topic untuk mengirim data suhu
const char* topicData =
    "unsoed/tk245004/kelompokrefan/data";

// Topic untuk menerima perintah LED
const char* topicPerintah =
    "unsoed/tk245004/kelompokrefan/perintah";

// [BARU] Topic untuk menerima perintah buzzer (aktuator kedua)
const char* topicBuzzer =
    "unsoed/tk245004/kelompokrefan/buzzer";

const int ledPin = 5;       // LED terhubung ke GPIO5 (D1)

// [BARU] Buzzer terhubung ke GPIO4 (D2)
const int buzzerPin = 4;

// Objek koneksi jaringan menggunakan WiFi
WiFiClient espClient;

// Objek MQTT Client
// espClient digunakan sebagai koneksi jaringan MQTT
PubSubClient client(espClient);

// Menyimpan waktu terakhir data dikirim
unsigned long waktuTerakhirPublish = 0;
const long intervalPublish = 5000; // Interval pengiriman data = 5 detik

// Fungsi callback dipanggil secara otomatis ketika
// ESP8266 menerima pesan MQTT dari topic yang di-subscribe.
//
// topic   = nama topic yang mengirim pesan
// payload = isi pesan dalam bentuk byte
// length  = panjang pesan
void callback(
  char* topic,
  byte* payload,
  unsigned int length
) {
  // Variabel untuk menyimpan pesan MQTT
  String pesan;

  // Mengubah payload dari byte menjadi String
  for (unsigned int i = 0; i < length; i++) {
    pesan += (char)payload[i];
  }

  // [BARU] Menampilkan topic asal pesan pada Serial Monitor
  Serial.print("Pesan masuk dari topic [");
  Serial.print(topic);
  Serial.println("]");

  // Membuat objek JSON
  JsonDocument doc;

  // Membaca pesan sebagai JSON
  // Jika parsing gagal, pesan akan diabaikan
  if (deserializeJson(doc, pesan)) {
    return;
  }

  // Mengambil nilai dari key "perintah"
  // Contoh:
  // {"perintah":"ON"}
  // Maka:
  // perintah = "ON"
  const char* perintah = doc["perintah"];

  // [BARU] Membedakan topic yang menerima pesan.
  // strcmp() bernilai 0 jika kedua teks sama persis.
  if (strcmp(topic, topicPerintah) == 0) {
    // Pesan dari topic perintah -> kendalikan LED
    //
    // Jika perintah = "ON"  -> LED = HIGH
    // Jika bukan "ON"       -> LED = LOW
    digitalWrite(
      ledPin,
      String(perintah) == "ON" ? HIGH : LOW
    );

    // [UBAH] Menampilkan perintah beserta nama aktuatornya
    Serial.print("Perintah diterima -> LED: ");
    Serial.println(perintah);

  // [BARU] Pesan dari topic buzzer -> kendalikan buzzer
  } else if (strcmp(topic, topicBuzzer) == 0) {
    // Jika perintah = "ON"  -> buzzer berbunyi (HIGH)
    // Jika bukan "ON"       -> buzzer mati (LOW)
    digitalWrite(
      buzzerPin,
      String(perintah) == "ON" ? HIGH : LOW
    );

    // Menampilkan perintah beserta nama aktuatornya
    Serial.print("Perintah diterima -> Buzzer: ");
    Serial.println(perintah);
  }
}

void hubungkanWiFi() {
  WiFi.begin(ssid, password);   // Memulai koneksi ke WiFi

  // Selama belum terhubung,
  // program akan menunggu
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  Serial.println("WiFi berhasil terhubung!");   // Jika berhasil terhubung
}

void hubungkanMQTT() {
  // Selama belum terhubung ke MQTT Broker
  while (!client.connected()) {
    // Membuat Client ID secara acak
    //
    // Contoh:
    // ESP8266Client-a42f
    String clientId =
        "ESP8266Client-" +
        String(random(0xffff), HEX);

    // Mencoba terhubung ke MQTT Broker
    if (client.connect(clientId.c_str())) {
      // Jika berhasil terhubung,
      client.subscribe(topicPerintah);       // subscribe ke topic perintah (LED)

      // [BARU] subscribe juga ke topic buzzer
      client.subscribe(topicBuzzer);

      Serial.println(
        "Terhubung dan subscribe topic perintah & buzzer"   // [UBAH] teks diperbarui
      );
    } else {
      // Jika gagal, tunggu 2 detik lalu mencoba kembali
      delay(2000);
    }
  }
}

// setup() hanya dijalankan satu kali
// ketika ESP8266 pertama kali dinyalakan
void setup() {
  // Memulai Serial Monitor
  // dengan baud rate 115200
  Serial.begin(115200);

  // Menentukan LED sebagai OUTPUT
  pinMode(ledPin, OUTPUT);

  // [BARU] Menentukan buzzer sebagai OUTPUT
  pinMode(buzzerPin, OUTPUT);

  // [BARU] Memastikan buzzer mati saat pertama dinyalakan
  digitalWrite(buzzerPin, LOW);

  // Menginisialisasi generator angka acak.
  //
  // Nilai analog dari pin A0 digunakan sebagai
  // sumber nilai awal (seed), sehingga angka random
  // yang dihasilkan tidak selalu sama.
  randomSeed(analogRead(0));

  // Menghubungkan ESP8266 ke WiFi
  hubungkanWiFi();

  // Menentukan MQTT Broker dan port
  client.setServer(mqttServer, mqttPort);

  // Menentukan callback untuk pesan MQTT masuk
  client.setCallback(callback);
}

// loop() dijalankan terus-menerus selama ESP8266 menyala
void loop() {
  // Jika koneksi MQTT terputus
  if (!client.connected()) {
    hubungkanMQTT(); // Hubungkan kembali ke MQTT Broker
  }

  // Memproses komunikasi MQTT
  //
  // Fungsi ini memungkinkan:
  // - menerima pesan MQTT
  // - menjalankan callback()
  // - menjaga koneksi MQTT
  client.loop();

  // Mengecek apakah sudah lewat 5 detik
  // sejak pengiriman sebelumnya
  if (
    millis() - waktuTerakhirPublish >
    intervalPublish
  ) {
    // Menyimpan waktu pengiriman sekarang
    waktuTerakhirPublish = millis();

    // Membuat angka random antara 25.0 sampai 34.9°C
    //
    // random(250, 350):
    // menghasilkan angka 250 sampai 349
    //
    // kemudian dibagi 10.0:
    // 250 / 10 = 25.0
    // 349 / 10 = 34.9
    //
    // Jadi nilai suhu TIDAK berasal dari sensor.
    float suhuDummy =
        random(250, 350) / 10.0;

    // Membuat objek JSON
    JsonDocument doc;

    // Memasukkan suhu dummy ke JSON
    //
    // Contoh hasil:
    // {"suhu":27.4}
    doc["suhu"] = suhuDummy;

    // Menyiapkan buffer untuk menyimpan JSON
    char buffer[128];

    // Mengubah objek JSON menjadi teks JSON
    serializeJson(doc, buffer);

    // Mengirim data JSON ke MQTT Broker menggunakan topic topicData
    client.publish(topicData, buffer);

    // Menampilkan data yang dikirim pada Serial Monitor
    Serial.print("Data terkirim: ");
    Serial.println(buffer);
  }
}
```

Penambahan terjadi pada **empat bagian** program, yang ditandai komentar `[BARU]` (baris baru) dan `[UBAH]` (baris lama yang diganti):

1. **Bagian global**, topic dan pin buzzer:

```cpp
const char* topicBuzzer = "unsoed/tk245004/kelompokrefan/buzzer";   // BARIS BARU
const int buzzerPin = 4;                                            // BARIS BARU
```

2. **Fungsi `callback()`**, pembeda topic:

```cpp
if (strcmp(topic, topicPerintah) == 0) { ... }        // BARIS BARU (membungkus kode LED lama)
else if (strcmp(topic, topicBuzzer) == 0) { ... }     // BARIS BARU
```

3. **Fungsi `hubungkanMQTT()`**, subscribe topic kedua:

```cpp
client.subscribe(topicBuzzer);   // BARIS BARU
```

4. **Fungsi `setup()`**, inisialisasi pin buzzer:

```cpp
pinMode(buzzerPin, OUTPUT);       // BARIS BARU
digitalWrite(buzzerPin, LOW);     // BARIS BARU
```

---

## 3. Penjelasan Baris yang Ditambahkan

### 3.1 Topic dan pin buzzer (bagian global)

```cpp
const char* topicBuzzer =
    "unsoed/tk245004/kelompokrefan/buzzer";

const int buzzerPin = 4;
```

- **`const char* topicBuzzer = "...buzzer";`** — Menyimpan alamat topic MQTT baru khusus untuk perintah buzzer. Topic ini berbeda dari `topicPerintah` (LED), sehingga broker dapat mengirim perintah ke masing-masing aktuator secara terpisah.
- **`const int buzzerPin = 4;`** — Menentukan buzzer terhubung ke **GPIO4**, yang pada NodeMCU ESP8266 adalah pin **D2**. Pin ini dipilih karena aman dipakai sebagai output dan tidak bentrok dengan LED di GPIO5.

### 3.2 Inisialisasi buzzer di `setup()`

```cpp
pinMode(buzzerPin, OUTPUT);
digitalWrite(buzzerPin, LOW);
```

- **`pinMode(buzzerPin, OUTPUT);`** — Mengatur GPIO4 sebagai **output** agar ESP8266 bisa mengirim sinyal HIGH/LOW ke buzzer.
- **`digitalWrite(buzzerPin, LOW);`** — Memastikan buzzer dalam keadaan **mati** saat ESP8266 baru dinyalakan, sehingga tidak berbunyi sebelum ada perintah.

### 3.3 Subscribe ke topic kedua di `hubungkanMQTT()`

```cpp
client.subscribe(topicBuzzer);

Serial.println(
  "Terhubung dan subscribe topic perintah & buzzer"
);
```

- **`client.subscribe(topicBuzzer);`** — Mendaftarkan ESP8266 ke topic buzzer. **Tanpa baris ini, pesan yang dikirim ke topic buzzer tidak akan pernah sampai** ke `callback()`, karena broker hanya meneruskan pesan dari topic yang sudah di-subscribe. Satu `PubSubClient` dapat men-subscribe banyak topic sekaligus.
- **Teks `Serial.println(...)` yang diperbarui** — Pesan di Serial Monitor diubah agar menunjukkan bahwa kedua topic sudah di-subscribe.

### 3.4 Pembeda topic di `callback()`

```cpp
Serial.print("Pesan masuk dari topic [");
Serial.print(topic);
Serial.println("]");

if (strcmp(topic, topicPerintah) == 0) {
  digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);
  Serial.print("Perintah diterima -> LED: ");
  Serial.println(perintah);

} else if (strcmp(topic, topicBuzzer) == 0) {
  digitalWrite(buzzerPin, String(perintah) == "ON" ? HIGH : LOW);
  Serial.print("Perintah diterima -> Buzzer: ");
  Serial.println(perintah);
}
```

- **`Serial.print("Pesan masuk dari topic [")` ... `Serial.println("]")`** — Mencetak nama topic asal pesan. Parameter `topic` pada `callback()` berisi nama topic yang mengirim pesan, sehingga mudah dicek saat pengujian.
- **`strcmp(topic, topicPerintah) == 0`** — `strcmp()` membandingkan **isi dua teks** karakter demi karakter dan mengembalikan **0 jika sama persis**. Perbandingan ini tidak ditulis `topic == topicPerintah` karena pada tipe `char*` operator `==` hanya membandingkan **alamat memori**, bukan isi teksnya, sehingga hasilnya hampir selalu salah.
- **`digitalWrite(ledPin, String(perintah) == "ON" ? HIGH : LOW);`** — Kode kendali LED yang sama dengan sebelumnya, tetapi sekarang hanya dijalankan jika pesan datang dari `topicPerintah`. Operator ternary `kondisi ? A : B` memberi nilai HIGH jika perintah `"ON"`, selain itu LOW.
- **`Serial.print("Perintah diterima -> LED: ");`** — Teks lama `"Aktuator: "` diganti `"LED: "` agar jelas aktuator mana yang menerima perintah.
- **`else if (strcmp(topic, topicBuzzer) == 0)`** — Cabang baru. Jika pesan bukan dari topic perintah, program memeriksa apakah pesan berasal dari topic buzzer.
- **`digitalWrite(buzzerPin, String(perintah) == "ON" ? HIGH : LOW);`** — Buzzer berbunyi (HIGH) jika perintah `"ON"` dan mati (LOW) jika bukan. Logikanya sama dengan LED, tetapi pin yang dikendalikan berbeda.
- **`Serial.print("Perintah diterima -> Buzzer: ");`** — Menampilkan perintah beserta nama aktuatornya di Serial Monitor.

Karena pesan dari kedua topic memakai key JSON yang sama (`"perintah"`), **topic-lah satu-satunya pembeda** aktuator yang harus bereaksi. Pembacaan JSON (`deserializeJson`) tetap dilakukan satu kali sebelum percabangan, sehingga tidak ada kode yang diulang.

### Catatan Perangkat Keras

- Kode ini menganggap buzzer yang dipakai adalah **buzzer aktif** (berbunyi sendiri saat diberi tegangan HIGH). Untuk **buzzer pasif**, bunyi harus dibangkitkan dengan `tone(buzzerPin, frekuensi)` dan dihentikan dengan `noTone(buzzerPin)`.
- Pin GPIO ESP8266 hanya mampu memberi arus kecil (sekitar 12 mA). Jika buzzer memerlukan arus lebih besar, gunakan transistor sebagai penguat.

### Contoh hasil setelah modifikasi

Pesan dikirim dari MQTT Explorer ke topic buzzer:

```
Topic  : unsoed/tk245004/kelompokrefan/buzzer
Pesan  : {"perintah":"ON"}
```

Output pada Serial Monitor:

```
Pesan masuk dari topic [unsoed/tk245004/kelompokrefan/buzzer]
Perintah diterima -> Buzzer: ON
```

Hanya buzzer yang berbunyi, sedangkan status LED tidak berubah.

---

