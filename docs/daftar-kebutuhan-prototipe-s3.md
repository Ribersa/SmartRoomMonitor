# Daftar Kebutuhan Lengkap Prototipe SmartRoomMonitor (ESP32-S3 + SHT30)
**Khusus Tahap Pengujian Breadboard (Tanpa Boks / Tanpa Solder Permanen)**

Dokumen ini memuat seluruh komponen modul, aktuator simulator, komponen pasif, kabel, dan alat kerja yang dibutuhkan untuk merakit prototipe di breadboard sesuai spesifikasi SRS v0.8.

---

## 1. Modul Pengendali & Sensor Utama

| No | Komponen | Spesifikasi Teknis | Qty | Fungsi pada SRS | Catatan Pembelian |
|---|---|---|---|---|---|
| 1 | **ESP32-S3 DevKit** | ESP32-S3-WROOM-1 / DevKitC-1 N16R8 (Pre-soldered Type-C) | 1 | Otak utama IoT & komunikasi Firebase | Toko Khurs IOT, sudah disolder kakinya dari pabrik. |
| 2 | **Modul Sensirion SHT30-D** | I2C 3.3V (Alamat `0x44` / `0x45`) | 1 | Suhu presisi (±0.2°C) & Kelembapan (±2% RH) (FR-01, FR-03, FR-04) | Toko Fortuna Jogja. Akurasi tinggi, bebas jebakan sensor palsu. |
| 3 | **Modul ScioSense ENS160 + AHT21** | Combo I2C (Alamat `0x52` & `0x38`) | 1 | Kualitas Udara (AQI, TVOC, eCO2) (FR-05) | Toko Akhishop Electronics. Menggunakan kompensasi suhu dari SHT30. |
| 4 | **Modul Radar HLK-LD2410** | mmWave 24 GHz (LD2410 / LD2410C) | 1 | Presensi: tidak ada / bergerak / diam (FR-07) | Toko Syalis Electrical Store. Lengkap dengan kabel bawaan 5-pin. |
| 5 | **Modul Sensor Cahaya BH1750** | I2C 3.3V (Alamat `0x23`) | 1 | Iluminansi Cahaya / Lux (FR-08) | Toko Java.Tech. Sensor digital lux (GY-302). |

---

## 2. Modul Aktuator Simulator (Kipas, Lampu, Buzzer)

| No | Komponen Aktuator | Spesifikasi Teknis | Qty | Fungsi pada SRS | Catatan |
|---|---|---|---|---|---|
| 1 | **Modul Active Buzzer 5V** | Tipe Modul 3-Pin dengan Driver Transistor | 1 | Alarm jam malam (FR-07) & alert bahaya (FR-01, FR-05, FR-06) | Toko Sholap1992. Suara kencang langsung dari logika GPIO. |
| 2 | **Kipas DC 5V 0.49A** | Ukuran 4x4 cm Tebal 2 cm High Speed | 1 | Pengujian kontrol otomatis suhu > 31°C (FR-02) | Toko Safira ihsana. Suplai dari rel 5V adaptor. |
| 3 | **Modul Driver MOSFET D4184** | Switching Trigger PWM Driver Module | 1 | Pengatur RPM halus kipas via PWM (GPIO 7) | Toko Starlectric. Menghubungkan GPIO 3.3V ke beban 5V 500mA. |
| 4 | **Bohlam Lampu USB 5V 10W** | Mitsuyama Samsung LED 5V USB | 1 | Pengujian kontrol lampu otomatis gelap + ada orang (FR-08) | Toko CV.AKA. Bohlam nyata tapi aman 5V tanpa risiko kesetrum. |
| 5 | **Modul Relay 1-Channel 5V** | Output 250VAC / 30VDC dengan Optocoupler | 2 | Saklar pemutus-penyambung daya bohlam USB | Toko AHMMUARIEF NETWORK. Dilengkapi LED indikator dan bunyi mekanik. |

---

## 3. Komponen Breadboard, Daya & Pengkabelan

| No | Komponen | Spesifikasi | Qty | Toko & Catatan |
|---|---|---|---|---|
| 1 | **Breadboard MB-102** | 830 titik | 1 | Sudah tersedia mandiri di meja. |
| 2 | **Kabel Jumper Dupont 15cm** | Female to Female (40 pcs) | 1 pack | Toko AIF Robotic. |
| 3 | **Kabel Jumper Dupont 15cm** | Male to Female (40 pcs) | 1 pack | Toko AIF Robotic. |
| 4 | **Kabel Jumper Dupont 15cm** | Male to Male (40 pcs) | 1 pack | Toko AIF Robotic. |
| 5 | **Male Pin Header 40 Pin** | Pitch 2.54 mm Breakable | 2 strip | Toko aiyothings. Cadangan pin modul. |
| 6 | **Adaptor USB 5V 2A 10W** | Olike C116 USB-A Charger | 1 | Toko Hwartzcell. Suplai daya utama sistem. |
| 7 | **Kabel USB A to Type-C 3A** | Vention Fast Charging (1.8 Meter) | 1 | Toko Vention Flagship Store. Kawat tembaga tebal anti-drop tegangan. |
| 8 | **Kapasitor Elektrolit (Elco)** | Rubycon 1000 µF / 16V | 2 | Toko EL-TECH ELECTRONICS. Bulk reservoir jalur 5V. |
| 9 | **Kapasitor Keramik Monolithic** | 100 nF 50V (Kode 104) | 5 | Toko Toko Mikrokontroler Jogja. Decoupling noise frekuensi tinggi. |
| 10 | **Resistor 330 Ω** | 1/4W Metal Film 1% | 10 pack (100 pcs) | Toko RAPTOR. Pembatas arus LED dan sinyal. |
