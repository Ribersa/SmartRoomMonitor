# Sinkronisasi Spesifikasi Sensor Hardware, SRS v1.0 & UML
**Proyek:** SmartRoomMonitor (Aplikasi Android & Node Sensor IoT ESP32-S3)  
**Baseline Dokumen:** [`SRS-SmartRoomMonitor-v1.0.md`](SRS-SmartRoomMonitor-v1.0.md) & Dokumen Pemodelan UML SmartRoomMonitor  
**Kontrak Backend Resmi:** [`firebase/schema-v0.9.json`](../firebase/schema-v0.9.json) & [`firebase/database.rules.json`](../firebase/database.rules.json)

---

## 1. Matriks Sinkronisasi: SRS v1.0, UML & Hardware Fisik Asli

*(Catatan: Menggantikan asumsi pemodelan awal DHT22/MQ-135/LDR/PIR pada UML Hal. 39 dengan hardware fisik aktual.)*

| ID (SRS v1.0) | Use Case UML | Sensor / Aktuator Fisik | Path Firebase & Logika Firmware | PIC Modul |
|---|---|---|---|---|
| **FR-01** | UC-03 | **Sensirion SHT30-D** (I2C `0x44`) + Buzzer | `room1/current/temperature`. Jika `> tempAlert` (`35.0 °C`) → ESP32 bunyikan buzzer lokal, set `room1/alarm`, catat `room1/events` (`temp_high`, debounce 60 dtk). | M1 Syakir |
| **FR-02** | UC-08 | **Driver MOSFET D4184 (GPIO 7)** + Kipas 5V | Staff tulis `room1/actuator/fanMode` (`auto`/`on`/`off`). ESP32 terapkan histeresis (`> 31 °C` ON, `<= 30 °C` OFF) dan kirim `room1/current/fanStatus`. | M1 Syakir |
| **FR-03** | UC-04 | **ScioSense ENS160** (I2C `0x53`) + Buzzer | `room1/current/{airDelta, airLabel, airAqi, airWarmingUp}`. Warm-up 3 menit (`"Memanaskan"`, bebas alarm — **NFR-06**), lalu kunci baseline TVOC untuk hitung `airDelta`. Jika `airDelta >= 160` atau AQI >= 4 → `"Buruk"`, alarm & event `air_bad`. | M2 Zakariya |
| **FR-04** | UC-07 | **SHT30-D, ENS160, BH1750** | ESP32 simpan sampel tiap **10 menit** ke `room1/history/{epochSec}` (144 titik / 24 jam, sesuai `limitToLast(144)`) + catat kejadian ke `room1/events`. | M2 Zakariya |
| **FR-05** | UC-06 | **HLK-LD2410B mmWave 24 GHz** (UART1 `GPIO 17/18`) | 3-Layer Anti-False-Positive Validator + Gate 0 Back-Lobe Shield. Status `room1/current/presence`: `"tidak ada"`, `"bergerak"`, `"diam"`. Jam malam NTP WIB `22:00–06:00` → alarm & event `night_presence`. | M3 Deswiryawan |
| **FR-06** | UC-10 | **Buzzer Pasif Active-LOW PNP (GPIO 6, VCC 3.3V)** | Berbunyi lokal walau internet putus (Core 1 otonom). Staff kirim `room1/alarm/mute = true` → ESP32 matikan buzzer `<= 7 detik` & perbarui `room1/alarm/status = "dibungkam"`. | M3 Deswiryawan |
| **FR-07** | UC-05 | **Rohm BH1750** (I2C `0x23`) | `room1/current/{lightLux, lightLabel}`: `"Gelap"` (`< 40 lux`), `"Terang"` (`>= 40 lux`). | M4 Fattih |
| **FR-08** | UC-09 | **Relay 1-Ch 5V Open-Drain (GPIO 15)** + Lampu 5V | Staff tulis `room1/actuator/lampMode` (`auto`/`on`/`off`). Pada `auto`: ON jika gelap (`< 40 lux`) & ada orang; OFF jika cahaya luar naik `+200 lux` (anti-optical-feedback) atau kosong `>= 10 detik`. Status balik ke `room1/current/lampStatus`. | M4 Fattih |
| **FR-B01** | UC-01, 02, 11 | **Firebase Auth + RTDB Rules** | Login Email/Password. Peran di `users/{uid}/role`: `"Pengunjung"` (read-only), `"Staff"` (kontrol & mute), `"Device"` (ESP32). | Bersama |
| **FR-B02** | UC-12 | **NTP Epoch Heartbeat (`timestamp` ms)** | ESP32 update `room1/current` tiap 3 detik dengan `timestamp` milidetik (13 digit). Jika `System.currentTimeMillis() - timestamp > 15_000` → banner `"Perangkat offline"`. | Bersama |

---

## 2. Kompensasi Lintas Sensor (Cross-Sensor Compensation)

1. Data suhu presisi (±0.2°C) dan kelembapan (±2% RH) dibaca dari **Sensirion SHT30-D** (`0x44`).
2. ESP32-S3 menginjeksikan nilai suhu & RH dari SHT30-D ke register kompensasi **ScioSense ENS160** (`0x53`) pada setiap siklus evaluasi (`setTempAndHum`), dan mengunci baseline TVOC pada menit ke-2 hingga ke-3 masa pemanasan untuk menghasilkan `airDelta` yang stabil (**NFR-06**).
