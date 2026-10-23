# 🧪 Hasil Pengujian Hardware — SmartRoomMonitor

Dokumentasi dan kode pengujian mandiri (*unit testing*) perangkat keras ESP32-S3 dan sensor sebelum diintegrasikan ke firmware utama.

---

## 📁 Daftar Skrip Pengujian (.ino)

| Folder / Skrip | Fungsi & Tujuan | Pin Terhubung | Status Hasil Uji |
|---|---|---|---|
| [`01_i2c_scanner_esp32s3/`](01_i2c_scanner_esp32s3/) | Pemindai bus I2C untuk verifikasi hardware & alamat chip. | SDA: GPIO 4<br>SCL: GPIO 5 | ✅ Terdeteksi `0x23` (BH1750), `0x38` (AHT21), `0x44` (SHT30), `0x53` (ENS160) |
| [`02_uji_telemetri_ens160_aht21/`](02_uji_telemetri_ens160_aht21/) | Pembacaan data awal suhu, kelembapan, TVOC, eCO2, dan AQI dengan AHT21. | SDA: GPIO 4<br>SCL: GPIO 5 | ✅ Responsif terhadap napas & uap alkohol |
| [`03_uji_alarm_bertingkat_aqi_buzzer/`](03_uji_alarm_bertingkat_aqi_buzzer/) | Sistem alarm bertingkat 5-Level AQI menggunakan Buzzer Pasif. | Buzzer: GPIO 6<br>I2C: GPIO 4 & 5 | ✅ Lolos uji sirine otomatis saat udara memburuk |
| [`04_uji_telemetri_sht30_ens160_bh1750_buzzer/`](04_uji_telemetri_sht30_ens160_bh1750_buzzer/) | Integrasi multi-sensor I2C utama (SHT30 presisi M1, BH1750 lux M4, ENS160 gas M3) dan driver buzzer anti-dengung. | SDA: GPIO 4<br>SCL: GPIO 5<br>Buzzer: GPIO 6 | ✅ Lolos uji 4 skenario empiris real-time |
| [`05_uji_telemetri_ble_serial/`](05_uji_telemetri_ble_serial/) | Streaming data sensor nirkabel ke HP Android via Bluetooth Low Energy (BLE 5.0) Nordic UART Service (NUS). | BLE 5.0 Radio<br>I2C: GPIO 4 & 5 | ✅ Terhubung & streaming real-time ke Serial Bluetooth Terminal |

---

## 📊 Hasil Uji Empiris Multi-Sensor (Skenario 4 Elemen)

Pengujian komprehensif dilakukan pada **23 Oktober 2026** menggunakan skrip `04` dan dicatat ke CSV secara real-time via `logger_esp32.py`:

| Skenario Pengujian | Aksi Fisik | Respons Sensor | Metrik Terukur | Status |
|---|---|---|---|---|
| **1. Uji Napas Manusia** | Hembusan napas langsung ke sensor SHT30 | Suhu dan kelembapan naik instan akibat uap hangat napas | Suhu: \(29.5^\circ\text{C} \rightarrow \mathbf{30.79^\circ\text{C}}\)<br>Lembap: \(63\% \rightarrow \mathbf{76.02\%}\) | **LULUS** |
| **2. Uji Gradien Dingin (Es)** | Sensor dimasukkan ke dalam gelas es batu (non-kontak tanpa menyentuh air) | Penurunan suhu cepat dan kondensasi dinding gelas | Suhu anjlok ke \(\mathbf{25.53^\circ\text{C}}\)<br>Lembap turun ke \(\mathbf{35.21\%}\) lalu pulih ke \(28.4^\circ\text{C}\) | **LULUS** |
| **3. Uji Dinamika Cahaya** | Sensor BH1750 ditutup telapak tangan lalu disorot lampu senter/flash HP | Transisi drastis dari kegelapan pekat ke penerangan intens | Tertutup: \(\mathbf{0.00\text{ Lux}}\)<br>Disorot: \(\mathbf{2.197,50\text{ Lux}}\) (respons < 2 detik) | **LULUS** |
| **4. Uji Gas Kritis & Sirine** | Paparan uap organik pekat ke sensor ENS160 | Algoritma UBA Jerman langsung menaikkan indeks ke AQI 5 dan memicu sirine | TVOC: \(\mathbf{5.897\text{ ppb}}\)<br>eCO2: \(\mathbf{2.006\text{ ppm}}\)<br>AQI: \(\mathbf{5}\) (Sirine Melengking 4.200 Hz) | **LULUS** |

---

## 🛠️ Catatan Arsitektur & Hardware Penting

1. **Bus I2C Bersama (Shared Bus):**
   - Jalur I2C pada **GPIO 4 (SDA)** dan **GPIO 5 (SCL)** terbukti stabil menjalankan 3 modul aktif secara simultan tanpa *bus contention* atau tabrakan data:
     - `0x23`: Rohm BH1750 (Intensitas Cahaya / Lux)
     - `0x44`: Sensirion SHT30-D (Suhu & Kelembapan Presisi Tinggi)
     - `0x53`: ScioSense ENS160 (Kualitas Udara TVOC/eCO2)
2. **Karakteristik Buzzer (Active-LOW PNP Transistor):**
   - Modul buzzer menggunakan transistor PNP bertanda `低电平触发` (Low-Level Trigger).
   - **Kondisi Diam/Senyap:** Pin GPIO 6 **WAJIB diatur `HIGH` (3.3V)** untuk mematikan transistor (*cut-off*). Jika diberi `LOW`, transistor akan *jenuh (ON)* dan mengalirkan arus DC statis yang menyebabkan dengung (*humming noise*).
   - **Tegangan VCC Buzzer:** Wajib terhubung ke **rel 3.3V** (jangan ke 5V), agar selisih tegangan \(V_{EB}\) pada transistor PNP menjadi 0V saat pin berlogika `HIGH`.
3. **Kompensasi Suhu SHT30 ke ENS160:**
   - Data suhu murni dari SHT30 diumpankan secara berkala ke ENS160 lewat `ens160.setTempAndHum(suhu, kelembapan)` untuk mengoreksi sensitivitas filamen gas MOX terhadap kelembapan relatif.
