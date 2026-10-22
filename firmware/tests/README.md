# 🧪 Hasil Pengujian Hardware — SmartRoomMonitor

Dokumentasi dan kode pengujian mandiri (*unit testing*) perangkat keras ESP32-S3 dan sensor sebelum diintegrasikan ke firmware utama.

---

## 📁 Daftar Skrip Pengujian (.ino)

| Folder / Skrip | Fungsi & Tujuan | Pin Terhubung | Status Hasil Uji |
|---|---|---|---|
| [`01_i2c_scanner_esp32s3/`](01_i2c_scanner_esp32s3/) | Pemindai bus I2C untuk verifikasi hardware & alamat chip. | SDA: GPIO 4<br>SCL: GPIO 5 | ✅ Terdeteksi `0x38` (AHT21) & `0x53` (ENS160) |
| [`02_uji_telemetri_ens160_aht21/`](02_uji_telemetri_ens160_aht21/) | Pembacaan data riil suhu, kelembapan, TVOC, eCO2, dan AQI dengan kompensasi lingkungan. | SDA: GPIO 4<br>SCL: GPIO 5 | ✅ Responsif terhadap napas & uap alkohol |
| [`03_uji_alarm_bertingkat_aqi_buzzer/`](03_uji_alarm_bertingkat_aqi_buzzer/) | Sistem alarm bertingkat 5-Level AQI menggunakan Buzzer Pasif (Level 1-3 senyap, Level 4 peringatan, Level 5 kritis melengking). | Buzzer: GPIO 6<br>I2C: GPIO 4 & 5 | ✅ Lolos uji sirine otomatis saat udara memburuk |

---

## 📊 Hasil Pengujian Ekstrem Sensor ENS160
Hasil uji paparan uap alkohol pekat membuktikan responsivitas filamen MOX dan kurva pemulihan (*recovery curve*):

- **Kondisi Normal (Ambient):**  
  `Suhu: 31.8°C | Lembap: 60.9% | AQI: 2 (Baik) | TVOC: 104 ppb | eCO2: 555 ppm`
- **Puncak Paparan Gas Ekstrem (Peak Saturation):**  
  `Suhu: 31.09°C | Lembap: 61.17% | AQI: 5 (Sangat Buruk) | TVOC: 53.082 ppb | eCO2: 32.654 ppm`
- **Waktu Pemulihan (*Recovery Time*):**  
  Sekitar 36 detik untuk kembali meluruh dari `53.082 ppb` (AQI 5) $\rightarrow$ `62 ppb` (AQI 1) secara bertahap tanpa keracunan sensor (*zero sensor poisoning*).

---

## 🛠️ Catatan Teknis Penting untuk Tim
1. **Alamat I2C:** Modul combo ENS160+AHT21 beralamat di `0x53` (ENS160) dan `0x38` (AHT21).
2. **Karakteristik Buzzer:** Modul buzzer yang datang berkarakter **Buzzer Pasif** (membutuhkan sinyal frekuensi gelombang mikrodetik untuk berbunyi, bukan DC statis).
3. **Kompensasi Lingkungan:** Selalu panggil `ens160.setTempAndHum(suhu, kelembapan)` sebelum membaca AQI agar hasil pembacaan gas bebas dari gangguan kelembapan udara.
