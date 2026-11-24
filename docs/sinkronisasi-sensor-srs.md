# Sinkronisasi Spesifikasi Sensor Hardware & SRS v0.9
**Proyek:** SmartRoomMonitor (Aplikasi Android & Node Sensor IoT ESP32-S3)  
**Baseline Dokumen:** [`SRS-SmartRoomMonitor-v0.9.md`](SRS-SmartRoomMonitor-v0.9.md)  
**Kontrak Backend Resmi:** [`firebase/schema-v0.9.json`](../firebase/schema-v0.9.json) & [`firebase/database.rules.json`](../firebase/database.rules.json)

---

## 1. Matriks Sinkronisasi: Kebutuhan SRS v0.9 vs Sensor Hardware

| ID (SRS v0.9) | Nama Kebutuhan | Sensor / Aktuator Hardware | Karakteristik Output & Logika Firmware | PIC Modul |
|---|---|---|---|---|
| **FR-01** | Suhu & Peringatan Suhu Tinggi | **Sensirion SHT30-D** (I2C `0x44`) + Buzzer | Float (°C), ±0.2°C. Jika `temperature > temp_alert` (default `35.0 °C`) → `alarm_active = true`, bunyi buzzer lokal, catat `events` (`temp_high`, debounce 60 dtk). | M1 Syakir |
| **FR-02** | Kontrol Kipas | **Driver MOSFET D4184 (GPIO 7)** + Kipas 5V | Mode: `auto` / `on` / `off`. Pada `auto`: ON jika suhu `> temp_fan_on` (`31.0 °C`), OFF jika turun `temp_fan_hysteresis` (`<= 30.0 °C`). | M1 Syakir |
| **FR-03** | Kualitas Udara & Peringatan | **ScioSense ENS160** (I2C `0x53`) + Buzzer | Output `air_quality_aqi` (1–5), `tvoc` (ppb), `eco2` (ppm). Selama 3 menit awal (`air_quality_warming_up = true`) label `"Memanaskan"` dan alarm ditekan (**NFR-06**). Pasca warm-up: AQI 1–2 `"Baik"`, AQI 3 `"Sedang"`, AQI 4–5 `"Buruk"` → trigger alarm & event `air_bad`. | M2 Zakariya |
| **FR-04** | Grafik 24 Jam & Event Log | **SHT30-D, ENS160, BH1750** | ESP32 menyimpan sampel tiap 5 menit ke `/room1/history/{epoch}` (`t`, `h`, `aqi`, `lux`, retensi 24 jam) dan mencatat kejadian penting ke `/room1/events`. | M2 Zakariya |
| **FR-05** | Kehadiran & Peringatan Jam Malam | **HLK-LD2410B mmWave 24 GHz** (UART1 `GPIO 17/18`) | 3-Layer Anti-False-Positive Validator (`on-delay 800 ms`, validasi biometrik diam/gerak, back-lobe shield Gate 0). Status: `"tidak ada"`, `"bergerak"`, `"diam"`. Jika jam NTP WIB `22:00–06:00` dan `presence_detected == true` → alarm & event `night_presence`. | M3 Deswiryawan |
| **FR-06** | Bunyi Peringatan & Mute Darurat | **Buzzer Pasif Active-LOW PNP (GPIO 6, VCC 3.3V)** | Tetap berbunyi lokal walau Wi-Fi/internet putus (Core 1 otonom). Flag `controls/alarm_muted = true` dari Staff membungkam buzzer dalam <= 7 detik (polling Core 0 tiap 2 dtk). | M3 Deswiryawan |
| **FR-07** | Tingkat Cahaya & Label | **Rohm BH1750** (I2C `0x23`) | Output `light_lux` (float) + `light_label`: `"Gelap"` (`< 50 Lux`), `"Normal"` (`50–249 Lux`), `"Terang"` (`>= 250 Lux`). | M4 Fattih |
| **FR-08** | Kontrol Lampu | **Relay 1-Ch 5V Open-Drain (GPIO 15)** + Lampu 5V | Mode: `auto` / `on` / `off`. Pada `auto`: ON saat gelap (`< light_dark_lux`) DAN ada orang; OFF jika cahaya naik `+200 Lux` di atas cahaya lampu sendiri (kompensasi umpan balik optik) ATAU kosong `>= 10 detik`. | M4 Fattih |
| **FR-B01** | Autentikasi & Peran | **Firebase Auth + RTDB Rules** | Login Email/Password. Peran disimpan di `/users/{uid}/role`: `"visitor"` (read-only), `"staff"` (kontrol & mute), `"device"` (khusus ESP32). | Bersama |
| **FR-B02** | Dashboard & Status Offline | **NTP Epoch Heartbeat (`device_timestamp`)** | ESP32 mengirim `/room1/telemetry` tiap 3 detik. Jika `waktu_sekarang - device_timestamp > 15 detik`, aplikasi menampilkan banner `"Perangkat offline"`. | Bersama |

---

## 2. Standar Skema Data Firebase RTDB v0.9 (NFR-05)

Struktur JSON resmi mengacu penuh pada [`firebase/schema-v0.9.json`](../firebase/schema-v0.9.json):
- `/users/{uid}` — metadata peran (`staff` / `visitor` / `device`).
- `/room1/telemetry` — pembacaan sensor real-time & status alarm terbaru.
- `/room1/controls` — mode perintah dari Staff (`fan_mode`, `lamp_mode`, `alarm_muted`) serta feedback aktual perangkat (`fan_status`, `lamp_status`).
- `/room1/thresholds` — batas ambang dinamis (diubah lewat Firebase Console).
- `/room1/history/{epoch}` — data time-series 24 jam (interval 5 menit) untuk grafik FR-04.
- `/room1/events/{pushId}` — log kejadian suhu tinggi, udara buruk, dan kehadiran jam malam.

---

## 3. Kompensasi Lintas Sensor (Cross-Sensor Compensation)

1. Data suhu presisi (±0.2°C) dan kelembapan (±2% RH) dibaca dari **Sensirion SHT30-D** (`0x44`).
2. ESP32-S3 menginjeksikan nilai suhu & RH dari SHT30-D ke register kompensasi **ScioSense ENS160** (`0x53`) pada setiap siklus evaluasi (`setTempAndHum`), sehingga pembacaan TVOC/eCO2/AQI stabil terhadap perubahan suhu dan kelembapan ruangan.
