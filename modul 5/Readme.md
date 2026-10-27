# Modifikasi Percobaan 5A Menambahkan Perhitungan Simpangan Baku dari Buffer

Program ini mengolah data suhu simulasi (pengganti sensor DHT22) dengan **moving average** 5 sampel serta statistik **minimum** dan **maksimum**. Modifikasi pada bagian ini menambahkan **simpangan baku** (*standard deviation*) yang dihitung dari data pada `buffer`, sehingga selain nilai rata-rata, program juga menunjukkan **seberapa menyebar** 5 pembacaan terakhir. Nilai simpangan baku kecil berarti pembacaan stabil, nilai besar berarti ada fluktuasi atau lonjakan.

Kode ini tidak memakai fungsi khusus satu board, sehingga berjalan di **ESP32** maupun **ESP8266**. Teks banner pada `setup()` masih bertuliskan `ESP8266`, dan bisa diganti `ESP32` jika perlu.

---

## 1. Library / Dependencies yang Diperlukan

Tidak ada library eksternal baru.

| Library | Fungsi |
|---|---|
| **Arduino.h** | Header inti framework Arduino: `Serial`, `random()`, `randomSeed()`, `micros()`, `delay()` |
| **math.h** | Fungsi matematika standar C. Modifikasi ini memakai **`sqrt()`** untuk akar kuadrat (sebelumnya sudah dipakai `sin()`) |

---

## 2. Kode Modifikasi

```cpp
#include <Arduino.h>
#include <math.h>

// ==========================================
// 1. SIMULASI DATA SUHU PENGGANTI SENSOR DHT22
// ==========================================
float bacaSuhuMock() {
  static unsigned long jumlahBaca = 0;
  jumlahBaca++;

  // Suhu dasar berubah secara bertahap
  float suhuDasar = 28.0 + 2.0 * sin(jumlahBaca * 0.08);

  // Noise kecil untuk mensimulasikan fluktuasi sensor
  float noise = random(-5, 6) * 0.1;

  float suhu = suhuDasar + noise;

  // Lonjakan suhu setiap 25 pembacaan untuk simulasi anomali
  if (jumlahBaca % 25 == 0) {
    suhu += 8.0;
  }

  return suhu;
}

// ==========================================
// 2. VARIABEL MOVING AVERAGE
// ==========================================
const int N = 5;                 // Jumlah sampel moving average
float buffer[N];                 // Menyimpan 5 pembacaan terakhir
int indeks = 0;                  // Posisi penulisan berikutnya
int jumlah = 0;                  // Jumlah data yang sudah masuk

// ==========================================
// 3. VARIABEL STATISTIK
// ==========================================
float suhuMin = 1000.0;
float suhuMax = -1000.0;

// ==========================================
// 4. FUNGSI MOVING AVERAGE
// ==========================================
float hitungMovingAverage(float nilaiBaru) {
  buffer[indeks] = nilaiBaru;

  // Memutar indeks kembali ke 0 setelah mencapai N-1
  indeks = (indeks + 1) % N;

  // Menghitung jumlah sampel yang sudah tersedia
  if (jumlah < N) {
    jumlah++;
  }

  float total = 0;

  // Menjumlahkan seluruh data dalam buffer
  for (int i = 0; i < jumlah; i++) {
    total += buffer[i];
  }

  // Mengembalikan nilai rata-rata
  return total / jumlah;
}

// ==========================================
// [BARU] 4B. FUNGSI SIMPANGAN BAKU
// ==========================================
// Menghitung simpangan baku populasi dari data pada buffer.
// Parameter rataRata adalah hasil hitungMovingAverage().
// Fungsi ini harus dipanggil SETELAH hitungMovingAverage()
// agar buffer sudah memuat pembacaan terbaru.
float hitungSimpanganBaku(float rataRata) {
  // [BARU] Penampung jumlah kuadrat selisih tiap data terhadap rata-rata
  float totalKuadrat = 0;

  // [BARU] Hanya data yang sudah terisi (sebanyak jumlah) yang dihitung
  for (int i = 0; i < jumlah; i++) {
    // [BARU] Selisih data ke-i terhadap rata-rata
    float selisih = buffer[i] - rataRata;

    // [BARU] Kuadratkan selisih lalu akumulasikan
    totalKuadrat += selisih * selisih;
  }

  // [BARU] Variansi = rata-rata dari kuadrat selisih
  float variansi = totalKuadrat / jumlah;

  // [BARU] Simpangan baku = akar kuadrat dari variansi
  return sqrt(variansi);
}

// ==========================================
// 5. SETUP ESP8266
// ==========================================
void setup() {
  Serial.begin(115200);

  // Menginisialisasi bilangan acak untuk noise
  randomSeed(micros());

  Serial.println();
  Serial.println("Percobaan 5A - ESP8266");
  Serial.println("Pengolahan Data Suhu Simulasi");
  Serial.println("---------------------------------------------");
}

// ==========================================
// 6. LOOP UTAMA
// ==========================================
void loop() {
  // Membaca suhu simulasi, tanpa sensor fisik
  float suhu = bacaSuhuMock();

  // Menghitung moving average
  float suhuFilter = hitungMovingAverage(suhu);

  // [BARU] Menghitung simpangan baku dari isi buffer
  float simpanganBaku = hitungSimpanganBaku(suhuFilter);

  // Memperbarui suhu minimum dan maksimum
  if (suhu < suhuMin) {
    suhuMin = suhu;
  }

  if (suhu > suhuMax) {
    suhuMax = suhu;
  }

  // Menampilkan hasil pada Serial Monitor
  Serial.print("Mentah: ");
  Serial.print(suhu, 2);

  Serial.print(" C | Filter: ");
  Serial.print(suhuFilter, 2);

  // [BARU] Menampilkan simpangan baku dengan 2 angka desimal
  Serial.print(" C | SD: ");
  Serial.print(simpanganBaku, 2);

  Serial.print(" C | Min: ");
  Serial.print(suhuMin, 2);

  Serial.print(" C | Max: ");
  Serial.print(suhuMax, 2);

  Serial.println(" C");

  // Interval pembacaan 2 detik
  delay(2000);
}
```

Penambahan terjadi pada **tiga bagian** program, yang ditandai komentar `[BARU]`:

1. **Fungsi baru `hitungSimpanganBaku()`**, diletakkan setelah `hitungMovingAverage()`.
2. **Fungsi `loop()`**, memanggil fungsi baru tepat setelah moving average dihitung:

```cpp
float simpanganBaku = hitungSimpanganBaku(suhuFilter);   // BARIS BARU
```

3. **Fungsi `loop()`**, menampilkan hasilnya di Serial Monitor:

```cpp
Serial.print(" C | SD: ");        // BARIS BARU
Serial.print(simpanganBaku, 2);    // BARIS BARU
```

Buffer, `hitungMovingAverage()`, dan pembacaan suhu simulasi **tidak diubah**.

---

## 3. Penjelasan Baris yang Ditambahkan

### 3.1 Rumus yang digunakan

Simpangan baku populasi dari data pada buffer dengan `n = jumlah` data:

```
SD = sqrt( ( (x1 - rata)^2 + (x2 - rata)^2 + ... + (xn - rata)^2 ) / n )
```

Langkahnya: (1) hitung selisih tiap data terhadap rata-rata, (2) kuadratkan, (3) jumlahkan, (4) bagi dengan `n` (hasilnya disebut **variansi**), (5) ambil akar kuadratnya.

### 3.2 Fungsi `hitungSimpanganBaku()`

```cpp
float hitungSimpanganBaku(float rataRata) {
  float totalKuadrat = 0;

  for (int i = 0; i < jumlah; i++) {
    float selisih = buffer[i] - rataRata;
    totalKuadrat += selisih * selisih;
  }

  float variansi = totalKuadrat / jumlah;

  return sqrt(variansi);
}
```

- **`float hitungSimpanganBaku(float rataRata)`** — Mendeklarasikan fungsi yang menerima rata-rata buffer (hasil `hitungMovingAverage()`) dan mengembalikan simpangan baku bertipe `float`. Rata-rata dikirim sebagai parameter agar tidak dihitung ulang.
- **`float totalKuadrat = 0;`** — Variabel penampung jumlah kuadrat selisih. Nilainya diawali 0 sebelum penjumlahan.
- **`for (int i = 0; i < jumlah; i++)`** — Mengulang sebanyak `jumlah`, bukan `N`. Pada beberapa pembacaan awal buffer belum penuh (`jumlah` masih 1 sampai 4), dan elemen yang belum terisi tidak boleh ikut dihitung.
- **`float selisih = buffer[i] - rataRata;`** — Menghitung seberapa jauh data ke-`i` dari rata-rata. Nilainya bisa positif atau negatif.
- **`totalKuadrat += selisih * selisih;`** — Mengkuadratkan selisih lalu menambahkannya ke total. Pengkuadratan membuat selisih negatif dan positif tidak saling menghapus, serta memberi bobot lebih besar pada data yang jauh dari rata-rata.
- **`float variansi = totalKuadrat / jumlah;`** — Membagi total dengan jumlah data untuk mendapat **variansi**. Karena `jumlah` minimal 1 saat fungsi dipanggil, tidak terjadi pembagian dengan nol.
- **`return sqrt(variansi);`** — Mengambil akar kuadrat variansi agar satuannya kembali ke °C, sama dengan data suhu, lalu mengembalikannya ke pemanggil.

> **Pilihan populasi atau sampel.** Program ini memakai pembagi `n` (populasi) karena buffer dianggap seluruh data pada jendela pengamatan. Jika dosen meminta simpangan baku **sampel**, ganti pembagi menjadi `jumlah - 1` dan tambahkan pengaman `if (jumlah < 2) return 0;` di awal fungsi, karena saat `jumlah = 1` pembaginya menjadi 0.

### 3.3 Pemanggilan di `loop()`

```cpp
float simpanganBaku = hitungSimpanganBaku(suhuFilter);
```

- **`hitungSimpanganBaku(suhuFilter)`** — Memanggil fungsi dengan `suhuFilter`, yaitu rata-rata buffer. Pemanggilan harus **setelah** `hitungMovingAverage(suhu)`, karena fungsi tersebut yang memasukkan pembacaan terbaru ke buffer. Jika urutannya terbalik, simpangan baku dihitung dari data yang belum diperbarui.
- **`float simpanganBaku = ...`** — Menyimpan hasilnya dalam variabel agar bisa ditampilkan.

### 3.4 Penampilan di Serial Monitor

```cpp
Serial.print(" C | SD: ");
Serial.print(simpanganBaku, 2);
```

- **`Serial.print(" C | SD: ");`** — Menutup tampilan nilai filter dengan satuan `C`, lalu mencetak label `SD` (*standard deviation*). Posisinya di antara `Filter` dan `Min` sehingga urutan kolom menjadi Mentah, Filter, SD, Min, Max.
- **`Serial.print(simpanganBaku, 2);`** — Mencetak simpangan baku dengan **2 angka desimal**, sama dengan format kolom lainnya. Satuan `C` ditambahkan oleh `Serial.print(" C | Min: ")` berikutnya.

### Contoh perhitungan manual

Tiga pembacaan pertama dari contoh output di bawah (28,06; 28,52; 28,78):

```
rata-rata  = (28,06 + 28,52 + 28,78) / 3 = 28,453
selisih    = -0,393 ; 0,067 ; 0,327
kuadrat    = 0,1544 ; 0,0045 ; 0,1069   (jumlah = 0,2658)
variansi   = 0,2658 / 3 = 0,0886
SD         = sqrt(0,0886) = 0,298  ->  0,30
```

Hasil ini sama dengan baris ke-3 pada contoh output (`SD: 0.30 C`).

### Contoh output setelah modifikasi

Hasil eksekusi simulasi (noise bersifat acak, sehingga angka pada percobaanmu bisa berbeda):

```
Percobaan 5A - ESP8266
Pengolahan Data Suhu Simulasi
---------------------------------------------
Mentah: 28.06 C | Filter: 28.06 C | SD: 0.00 C | Min: 28.06 C | Max: 28.06 C
Mentah: 28.52 C | Filter: 28.29 C | SD: 0.23 C | Min: 28.06 C | Max: 28.52 C
Mentah: 28.78 C | Filter: 28.45 C | SD: 0.30 C | Min: 28.06 C | Max: 28.78 C
Mentah: 28.53 C | Filter: 28.47 C | SD: 0.26 C | Min: 28.06 C | Max: 28.78 C
Mentah: 28.38 C | Filter: 28.45 C | SD: 0.23 C | Min: 28.06 C | Max: 28.78 C
```

Pada pembacaan pertama `SD = 0.00` karena baru ada satu data, sehingga belum ada sebaran.

### Efek lonjakan anomali

Program menambahkan lonjakan +8 °C setiap 25 pembacaan. Hasil simulasi di sekitar pembacaan ke-25:

```
Mentah: 29.58 C | Filter: 29.63 C | SD: 0.19 C | ...   (pembacaan 24)
Mentah: 38.12 C | Filter: 31.36 C | SD: 3.39 C | ...   (pembacaan 25, lonjakan)
Mentah: 29.55 C | Filter: 31.33 C | SD: 3.40 C | ...   (26)
Mentah: 29.77 C | Filter: 31.34 C | SD: 3.39 C | ...   (28)
Mentah: 29.05 C | Filter: 29.46 C | SD: 0.26 C | ...   (pembacaan 30)
```

Simpangan baku naik dari sekitar 0,2 menjadi sekitar 3,4 ketika data lonjakan masuk buffer, bertahan selama data itu masih berada di dalam jendela 5 sampel, lalu turun kembali pada pembacaan ke-30 saat data lonjakan keluar dari buffer. Jadi simpangan baku dapat dipakai sebagai **indikator adanya anomali**, sedangkan nilai `Filter` hanya bergeser sekitar 2 °C.

---
# Modifikasi Percobaan 5B: Moving Average (Jendela 5 Data) pada Data CSV Sebelum Perhitungan Z-Score

Program ini melanjutkan Percobaan 5B. Data suhu simulasi (pengganti sensor DHT22) yang dipublikasikan ESP32 ke broker MQTT diterima oleh skrip Python dan disimpan ke `data_suhu.csv`. Pada program awal, z-score dihitung langsung dari data mentah. Pada modifikasi ini, data pada CSV **dihaluskan terlebih dahulu dengan moving average jendela 5 data**, kemudian z-score dihitung dari data yang sudah dihaluskan. Jumlah anomali dari kedua metode (data mentah dan data terfilter) dibandingkan untuk melihat pengaruh filter terhadap deteksi anomali.

Sisi ESP32 (Wokwi) **tidak diubah**. Seluruh modifikasi berada di skrip Python (sisi server/laptop).

---

## 1. Library / Dependencies yang Diperlukan

Tidak ada library baru. Seluruhnya sudah dipakai pada program awal.

| Library | Fungsi |
|---|---|
| **json** | Mengubah payload JSON dari broker menjadi dictionary |
| **time** | Membuat stempel waktu penerimaan data |
| **paho-mqtt** | Berlangganan (subscribe) ke topik MQTT |
| **pandas** | Membaca/menulis CSV, `rolling()` untuk moving average, `mean()` dan `std()` untuk statistik |
| **matplotlib** | Membuat grafik perbandingan |

---

## 2. Kode Modifikasi

Bagian yang ditambahkan atau diubah ditandai komentar `[BARU]`. Langkah 1 dan 2 (mengambil data dari MQTT dan menyimpan ke CSV) tidak berubah.

```python
import json
import time
import paho.mqtt.client as mqtt  # Komunikasi dengan broker MQTT
import pandas as pd              # Mengolah data dan CSV
import matplotlib.pyplot as plt  # Membuat grafik

# Konfigurasi broker dan topik MQTT
BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "unsoed/tk245004/kelompokrefan/perintah"
JUMLAH_SAMPEL = 30  # Jumlah data yang dikumpulkan
WINDOW = 5          # [BARU] Ukuran jendela moving average
BATAS_Z = 2         # [BARU] Batas |z-score| untuk anomali

data = []  # Menampung waktu penerimaan dan suhu

# Dipanggil ketika berhasil terhubung ke broker
def on_connect(client, userdata, flags, reason_code, properties):
    print("Terhubung ke broker, kode:", reason_code)
    client.subscribe(TOPIC)

# Dipanggil setiap kali pesan diterima
def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        suhu = float(payload["suhu"])
    except (ValueError, KeyError):
        print("Data tidak valid, diabaikan")
        return

    data.append({
        "waktu": time.strftime("%Y-%m-%d %H:%M:%S"),
        "suhu": suhu
    })
    print(f"[{len(data)}/{JUMLAH_SAMPEL}] suhu = {suhu}")

    if len(data) >= JUMLAH_SAMPEL:
        client.disconnect()

# [BARU] Fungsi z-score dipakai dua kali (data mentah dan data MA)
def hitung_zscore(seri):
    rata2 = seri.mean()   # [BARU] Rata-rata seri
    std = seri.std()      # [BARU] Simpangan baku sampel (pembagi n-1)
    if std == 0:
        z = pd.Series(0.0, index=seri.index)  # [BARU] Hindari pembagian nol
    else:
        z = (seri - rata2) / std              # [BARU] Rumus z-score
    return z, rata2, std                      # [BARU] Kembalikan tiga hasil

# 1. Mengambil data suhu dari broker MQTT
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
client.on_connect = on_connect
client.on_message = on_message
client.connect(BROKER, PORT)
client.loop_forever()

# 2. Menyimpan data yang terkumpul ke file CSV
df = pd.DataFrame(data)
df.to_csv("data_suhu.csv", index=False)
print("Data tersimpan di data_suhu.csv")

# 3. [BARU] Membaca kembali data dari file CSV
df = pd.read_csv("data_suhu.csv")

# 4. [BARU] Moving average jendela 5 data pada kolom suhu
df["suhu_ma"] = df["suhu"].rolling(window=WINDOW, min_periods=1).mean()

# 5. [BARU] Z-score dan anomali dari DATA MENTAH (pembanding)
z_mentah, rata2_mentah, std_mentah = hitung_zscore(df["suhu"])
df["zscore_mentah"] = z_mentah
df["anomali_mentah"] = df["zscore_mentah"].abs() > BATAS_Z

# 6. [BARU] Z-score dan anomali dari DATA HASIL MOVING AVERAGE
z_ma, rata2_ma, std_ma = hitung_zscore(df["suhu_ma"])
df["zscore_ma"] = z_ma
df["anomali_ma"] = df["zscore_ma"].abs() > BATAS_Z

# 7. [BARU] Menyimpan hasil lengkap ke CSV baru
df.to_csv("data_suhu_ma.csv", index=False)

# 8. [BARU] Menampilkan perbandingan
n_mentah = int(df["anomali_mentah"].sum())
n_ma = int(df["anomali_ma"].sum())
print("---------------- PERBANDINGAN ----------------")
print(f"Data mentah : rata-rata {rata2_mentah:.2f} | std {std_mentah:.2f} | anomali {n_mentah}")
print(f"Moving avg  : rata-rata {rata2_ma:.2f} | std {std_ma:.2f} | anomali {n_ma}")
print(f"Selisih jumlah anomali: {n_mentah - n_ma}")

# 9. [BARU] Grafik dua panel: data mentah dan data moving average
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7), sharex=True)

ax1.plot(df.index, df["suhu"], marker="o", label="Suhu mentah")
ax1.scatter(df.index[df["anomali_mentah"]], df["suhu"][df["anomali_mentah"]],
            color="red", s=100, label="Anomali", zorder=3)
ax1.axhline(rata2_mentah, color="gray", linestyle="--", label="Rata-rata")
ax1.set_ylabel("Suhu (°C)")
ax1.set_title(f"Z-score data mentah (anomali: {n_mentah})")
ax1.legend()

ax2.plot(df.index, df["suhu_ma"], marker="o", color="green", label="Suhu MA (N=5)")
ax2.scatter(df.index[df["anomali_ma"]], df["suhu_ma"][df["anomali_ma"]],
            color="red", s=100, label="Anomali", zorder=3)
ax2.axhline(rata2_ma, color="gray", linestyle="--", label="Rata-rata")
ax2.set_xlabel("Urutan data")
ax2.set_ylabel("Suhu (°C)")
ax2.set_title(f"Z-score data moving average (anomali: {n_ma})")
ax2.legend()

plt.tight_layout()
plt.savefig("grafik_suhu_ma.png")
plt.show()
```

---

## 3. Penjelasan Baris yang Ditambahkan

### 3.1 Konstanta `WINDOW` dan `BATAS_Z`

```python
WINDOW = 5
BATAS_Z = 2
```

- **`WINDOW = 5`**: Menentukan ukuran jendela moving average sebesar 5 data, sesuai soal. Nilai ditaruh pada konstanta agar mudah diubah (misalnya 3 atau 10) tanpa mengedit logika program.
- **`BATAS_Z = 2`**: Menyimpan batas |z-score| untuk anomali. Pada program awal angka 2 ditulis langsung di dalam kode, sehingga konstanta ini hanya merapikan penulisan dan memastikan kedua metode memakai batas yang sama.

### 3.2 Fungsi `hitung_zscore()`

```python
def hitung_zscore(seri):
    rata2 = seri.mean()
    std = seri.std()
    if std == 0:
        z = pd.Series(0.0, index=seri.index)
    else:
        z = (seri - rata2) / std
    return z, rata2, std
```

Z-score dihitung dua kali (data mentah dan data MA), sehingga logikanya dijadikan fungsi agar tidak ditulis ganda.

- **`def hitung_zscore(seri)`**: Menerima satu kolom data (`pd.Series`) dan mengembalikan z-score, rata-rata, serta simpangan baku.
- **`rata2 = seri.mean()`**: Rata-rata seluruh data pada seri (μ).
- **`std = seri.std()`**: Simpangan baku **sampel** (σ, pembagi n−1), karena `std()` pandas secara bawaan memakai `ddof=1`. Ini sama dengan perhitungan pada program awal.
- **`if std == 0:`**: Mengecek kasus semua data identik. Pada kondisi ini pembagian dengan σ menghasilkan pembagian nol.
- **`z = pd.Series(0.0, index=seri.index)`**: Jika σ = 0, semua z-score diisi 0 (tidak ada penyimpangan). `index=seri.index` menjaga urutan baris agar sejajar dengan `df`.
- **`z = (seri - rata2) / std`**: Rumus z-score, yaitu selisih tiap data terhadap rata-rata dibagi simpangan baku. Pandas mengerjakannya untuk seluruh baris sekaligus (vectorized).
- **`return z, rata2, std`**: Mengembalikan tiga nilai sekaligus. Rata-rata dan σ dibutuhkan untuk ditampilkan dan untuk garis rata-rata pada grafik.

### 3.3 Membaca ulang data dari CSV

```python
df = pd.read_csv("data_suhu.csv")
```

- Membaca file `data_suhu.csv` yang baru disimpan ke `DataFrame` baru. Langkah ini membuat seluruh analisis bersumber dari **data hasil CSV**, sesuai ketentuan soal, dan memungkinkan analisis dijalankan terpisah dari akuisisi data (file CSV lama dapat dipakai tanpa subscribe ulang).

### 3.4 Moving average jendela 5

```python
df["suhu_ma"] = df["suhu"].rolling(window=WINDOW, min_periods=1).mean()
```

- **`df["suhu_ma"] = ...`**: Membuat kolom baru `suhu_ma` untuk menyimpan data terfilter. Kolom `suhu` (mentah) tetap utuh agar dapat dibandingkan.
- **`df["suhu"].rolling(window=WINDOW, ...)`**: Membuat jendela geser (*sliding window*) selebar 5 data. Untuk baris ke-*i*, jendela berisi baris ke-*i−4* sampai ke-*i*, sama dengan 5 pembacaan terakhir pada buffer Percobaan 5A.
- **`min_periods=1`**: Mengizinkan perhitungan walaupun jendela belum penuh. Pada baris ke-1 sampai ke-4, rata-rata dihitung dari data yang tersedia saja (1 sampai 4 data). Perilaku ini setara dengan variabel `jumlah` pada `hitungMovingAverage()` di 5A. Tanpa argumen ini, empat baris pertama bernilai `NaN`.
- **`.mean()`**: Menghitung rata-rata tiap jendela.

### 3.5 Z-score data mentah (pembanding)

```python
z_mentah, rata2_mentah, std_mentah = hitung_zscore(df["suhu"])
df["zscore_mentah"] = z_mentah
df["anomali_mentah"] = df["zscore_mentah"].abs() > BATAS_Z
```

- **`z_mentah, rata2_mentah, std_mentah = hitung_zscore(df["suhu"])`**: Memanggil fungsi pada data mentah dan menampung tiga hasilnya.
- **`df["zscore_mentah"] = z_mentah`**: Menyimpan z-score data mentah sebagai kolom.
- **`df["anomali_mentah"] = df["zscore_mentah"].abs() > BATAS_Z`**: Membuat kolom boolean. Nilai `True` berarti |z| > 2 sehingga data ditandai anomali. Ini sama dengan hasil program awal dan menjadi **pembanding (baseline)**.

### 3.6 Z-score data hasil moving average

```python
z_ma, rata2_ma, std_ma = hitung_zscore(df["suhu_ma"])
df["zscore_ma"] = z_ma
df["anomali_ma"] = df["zscore_ma"].abs() > BATAS_Z
```

- **`hitung_zscore(df["suhu_ma"])`**: Fungsi yang sama dipanggil pada data terfilter. Rata-rata dan σ dihitung ulang dari `suhu_ma`, bukan dari data mentah.
- **`df["zscore_ma"]`** dan **`df["anomali_ma"]`**: Kolom z-score dan penanda anomali untuk data terfilter, dengan batas yang sama (`BATAS_Z`).

### 3.7 Menyimpan hasil

```python
df.to_csv("data_suhu_ma.csv", index=False)
```

- Menyimpan seluruh kolom (suhu, suhu_ma, kedua z-score, kedua penanda anomali) ke file baru. `data_suhu.csv` asli tidak ditimpa. `index=False` mencegah kolom nomor baris ikut tersimpan.

### 3.8 Menampilkan perbandingan

```python
n_mentah = int(df["anomali_mentah"].sum())
n_ma = int(df["anomali_ma"].sum())
print(f"Data mentah : rata-rata {rata2_mentah:.2f} | std {std_mentah:.2f} | anomali {n_mentah}")
print(f"Moving avg  : rata-rata {rata2_ma:.2f} | std {std_ma:.2f} | anomali {n_ma}")
print(f"Selisih jumlah anomali: {n_mentah - n_ma}")
```

- **`df["anomali_mentah"].sum()`**: Menjumlahkan kolom boolean (`True` dihitung 1, `False` dihitung 0), sehingga hasilnya adalah jumlah anomali. `int(...)` mengubah tipe `numpy` menjadi bilangan bulat biasa.
- **`print(... :.2f ...)`**: Menampilkan rata-rata dan σ dengan 2 desimal, konsisten dengan format program awal. Menampilkan σ penting karena σ data MA lebih kecil dan memengaruhi hasil z-score.
- **`n_mentah - n_ma`**: Selisih jumlah anomali. Nilai positif berarti filter mengurangi anomali yang terdeteksi.

### 3.9 Grafik perbandingan

```python
fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 7), sharex=True)
```

- **`plt.subplots(2, 1, ..., sharex=True)`**: Membuat dua panel atas-bawah dengan sumbu-x bersama, sehingga posisi data yang sama sejajar vertikal dan mudah dibandingkan.
- **`ax1.plot(...)`** dan **`ax1.scatter(...)`**: Panel atas menggambar suhu mentah dan menandai titik anomalinya dengan lingkaran merah. `df.index[df["anomali_mentah"]]` memilih nomor baris yang bernilai `True`.
- **`ax1.axhline(rata2_mentah, ...)`**: Garis putus-putus rata-rata suhu mentah.
- **`ax2.plot(df.index, df["suhu_ma"], ...)`** dan **`ax2.scatter(...)`**: Panel bawah menggambar suhu hasil moving average (hijau) dan titik anomali hasil z-score MA.
- **`ax1.set_title(f"... {n_mentah})")`** dan **`ax2.set_title(...)`**: Menampilkan jumlah anomali di judul tiap panel agar perbandingan terbaca langsung dari gambar.
- **`plt.savefig("grafik_suhu_ma.png")`**: Menyimpan grafik ke file baru tanpa menimpa grafik program awal.

---

## 4. Contoh Perhitungan Manual

Mengambil baris ke-14 (indeks 14) dari contoh output di bawah. Jendelanya berisi baris 10 sampai 14:

```
MA        = (37,32 + 30,05 + 29,46 + 29,07 + 29,06) / 5
          = 154,96 / 5 = 30,99
z-score   = (30,99 - 29,46) / 1,10 = 1,39
```

`29,46` dan `1,10` adalah rata-rata dan simpangan baku seluruh kolom `suhu_ma`. Hasilnya sama dengan baris ke-14 pada contoh output (`zscore_ma = 1.39`).

---

## 5. Contoh Output dan Perbandingan

Contoh di bawah berasal dari **simulasi data** dengan pola yang sama seperti `bacaSuhuMock()` (30 sampel, satu lonjakan +8 °C). Karena noise bersifat acak, angka pada percobaan sebenarnya dapat berbeda.

```
---------------- PERBANDINGAN ----------------
Data mentah : rata-rata 29.30 | std 1.77 | anomali 1
Moving avg  : rata-rata 29.46 | std 1.10 | anomali 0
Selisih jumlah anomali: 1
```

Cuplikan data di sekitar lonjakan:

```
 idx   suhu   suhu_ma  zscore_mentah  zscore_ma
   9  30.28     29.93           0.55       0.43
  10  37.32     31.48           4.52       1.83   <- lonjakan
  11  30.05     31.43           0.42       1.79
  12  29.46     31.41           0.09       1.77
  13  29.07     31.24          -0.13       1.61
  14  29.06     30.99          -0.14       1.39
  15  29.45     29.42           0.08      -0.04
```

Tabel perbandingan:

| Metode | Rata-rata (°C) | Simpangan baku (°C) | Jumlah anomali |
|---|---|---|---|
| Z-score data mentah | 29,30 | 1,77 | 1 |
| Z-score setelah moving average (N=5) | 29,46 | 1,10 | 0 |

### Analisis

1. **Lonjakan teredam.** Pada data mentah, lonjakan 37,32 °C berjarak 4,52 simpangan baku dari rata-rata, sehingga jelas terdeteksi. Setelah moving average, lonjakan 8 °C dibagi ke 5 data (sekitar +1,6 °C per data), dan nilai puncak z-score turun menjadi 1,83, di bawah batas 2.
2. **Lonjakan tersebar.** Pengaruh lonjakan bertahan selama 5 baris (baris 10 sampai 14) karena data itu masih berada di dalam jendela. Pada baris 15 nilai filter kembali normal.
3. **Simpangan baku mengecil.** σ turun dari 1,77 menjadi 1,10 karena fluktuasi acak diredam, tetapi penurunan σ tidak cukup mengompensasi turunnya puncak lonjakan.
4. **Kesimpulan.** Moving average efektif menghilangkan noise sehingga mengurangi *false positive* akibat fluktuasi acak kecil. Namun untuk anomali berupa **lonjakan tunggal** yang singkat, filter justru dapat menyembunyikannya (*false negative*). Pemilihan metode bergantung pada jenis anomali yang ingin dideteksi: lonjakan mendadak lebih cocok pada data mentah, sedangkan penyimpangan yang bertahan lama lebih cocok pada data terfilter.

> **Catatan.** Dengan batas |z| > 2 dan sampel hanya 30 data, hasil sangat sensitif terhadap posisi lonjakan dan noise acak. Jika hasil Anda berbeda dari contoh (misalnya anomali MA tidak nol), jelaskan berdasarkan nilai z-score pada tabel `data_suhu_ma.csv` milik Anda sendiri.

---

## 6. Cara Menjalankan

1. Jalankan simulasi ESP32 pada Wokwi sampai `Data terkirim ke MQTT` tampil.
2. Jalankan skrip Python ini. Skrip akan mengumpulkan `JUMLAH_SAMPEL` data, menyimpan `data_suhu.csv`, lalu menghasilkan `data_suhu_ma.csv` dan `grafik_suhu_ma.png`.
3. Salin tiga baris hasil perbandingan dan gambar grafik ke bagian Hasil Pengamatan pada BCP.
