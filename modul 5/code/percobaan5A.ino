
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

  Serial.print(" C | Min: ");
  Serial.print(suhuMin, 2);

  Serial.print(" C | Max: ");
  Serial.print(suhuMax, 2);

  Serial.println(" C");

  // Interval pembacaan 2 detik
  delay(2000);
}
