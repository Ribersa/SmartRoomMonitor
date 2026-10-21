# Sinkronisasi Spesifikasi Sensor Hardware & SRS v0.8
**Proyek:** SmartRoomMonitor (Aplikasi Android & Node Sensor IoT ESP32-S3)  
**Tujuan:** Menjembatani kebutuhan fungsional software (SRS v0.8) dengan implementasi perangkat keras nyata (ESP32-S3 + SHT30-D).

---

## 1. Matriks Sinkronisasi: Kebutuhan SRS vs Sensor Hardware

| ID Kebutuhan (SRS) | Nama Kebutuhan SRS | Sensor / Aktuator Hardware | Karakteristik Output Sensor | Pemetaan Logika ke Software (SRS) | PIC |
|---|---|---|---|---|---|
| **FR-01** | Suhu & Peringatan Suhu Tinggi | **Sensirion SHT30-D** (I2C `0x44`) | Float (°C), resolusi 0.01°C, akurasi presisi ±0.2°C | Jika Suhu > 35°C $\rightarrow$ Trigger Banner/Badge, Bunyikan Buzzer, Catat Event Log. Debounce 60 detik. | M1 Syakir |
| **FR-02** | Kontrol Kipas | **Driver MOSFET D4184 (GPIO 7)** + Kipas 5V | Output PWM LEDC 25 kHz | Mode: `Auto` / `ON` / `OFF`. Pada Auto: Kipas ON jika Suhu > 31°C, Kipas OFF jika Suhu $\le$ 30°C (Histeresis 1°C). | M1 Syakir |
| **FR-03** | Kelembapan & Label | **Sensirion SHT30-D** (I2C `0x44`) | Float (% RH), akurasi ±2% RH | Pemetaan Label:<br>• `< 40%` : **Kering**<br>• `40% - 60%` : **Nyaman**<br>• `> 60%` : **Lembap** | M2 Zakariya |
| **FR-04** | Grafik 24 Jam & Event Log | **SHT30-D & ENS160** | Array Data Historis (Time-series) | Agregasi data tiap 5 detik di Firebase untuk kalkulasi nilai Min, Max, Average per 24 jam. | M2 Zakariya |
| **FR-05** | Kualitas Udara & Peringatan | **ScioSense ENS160** (I2C `0x52`) | Register Status, AQI (1-5 UBA), TVOC (ppb), eCO2 (ppm) | Pemetaan Label:<br>• Status Register `0x01`: **Memanaskan** (3 menit pertama, abaikan alarm)<br>• AQI 1-2: **Baik**<br>• AQI 3: **Sedang**<br>• AQI 4-5: **Buruk** $\rightarrow$ Trigger Banner & Buzzer | M3 Fattih |
| **FR-06** | Bunyi Peringatan & Mute Darurat | **Buzzer Fisik Active (GPIO 6)** | Sinyal Digital Transistor Driver | Bunyi lokal independen internet jika ada status Buruk / Suhu Bahaya / Jam Malam. Mute via Firebase: Flag `alarm_muted = true` mematikan buzzer dalam $\le$ 7 detik. | M3 Fattih |
| **FR-07** | Kehadiran & Peringatan Jam Malam | **HLK-LD2410 mmWave** (UART2 `GPIO 17/18`) | Telemetri State: None / Moving / Still | Pemetaan Status Kehadiran:<br>• None $\rightarrow$ **"tidak ada"**<br>• Moving $\rightarrow$ **"bergerak"**<br>• Still $\rightarrow$ **"diam"**<br>Jika jam 22:00-06:00 status $\neq$ "tidak ada" $\rightarrow$ Alarm Keamanan Malam. | M4 Deswiryawan |
| **FR-08** | Kontrol Lampu | **Rohm BH1750** (I2C `0x23`) & **Relay 5V + Bohlam USB** | Iluminansi Integer (0 - 65535 Lux) | Mode: `Auto` / `ON` / `OFF`. Pada Auto: Lampu ON jika (Lux < 40) DAN (Kehadiran $\neq$ "tidak ada"). Lampu OFF jika (Lux $\ge$ 80) ATAU (Kehadiran "tidak ada" > 60 detik). | M4 Deswiryawan |
| **FR-B02** | Dashboard & Status Offline | **ESP32-S3 Heartbeat Timestamp** | Epoch Timestamp (detik) dikirim tiap 5 dtk | Jika selisih `waktu_sekarang - device_timestamp > 15 detik` $\rightarrow$ Tampilkan Banner "Perangkat Offline". | Bersama |

---

## 2. Struktur Skema Data Standar Firebase Realtime Database (NFR-07)

```json
{
  "room1": {
    "telemetry": {
      "temperature": 28.6,
      "humidity": 58.2,
      "humidity_label": "Nyaman",
      "air_quality_label": "Baik",
      "air_quality_aqi": 2,
      "air_quality_tvoc": 80,
      "air_quality_eco2": 440,
      "light_lux": 110,
      "presence_state": "diam",
      "device_timestamp": 1729015000
    },
    "controls": {
      "fan_mode": "auto",
      "fan_status": false,
      "lamp_mode": "auto",
      "lamp_status": false,
      "alarm_muted": false
    },
    "thresholds": {
      "temp_fan_on": 31.0,
      "temp_fan_off": 30.0,
      "temp_alert": 35.0,
      "light_dark_threshold": 40,
      "light_bright_threshold": 80,
      "night_start": "22:00",
      "night_end": "06:00"
    },
    "events": {
      "-O8xyz123abc": {
        "timestamp": 1729014900,
        "type": "night_presence",
        "severity": "danger",
        "message": "Deteksi pergerakan di luar jam operasional (23:14)"
      }
    }
  }
}
```

---

## 3. Kompensasi Lintas Sensor (Cross-Sensor Compensation)

1. Data suhu presisi (±0.2°C) dan kelembapan (±2% RH) dibaca dari **Sensirion SHT30-D** (`0x44`).
2. ESP32-S3 menginjeksikan data suhu & RH dari SHT30-D ke register kompensasi **ScioSense ENS160** (`0x52`) setiap 10 detik.
3. Ini menghilangkan penyimpangan pembacaan gas yang diakibatkan oleh perubahan mikroklimat ruangan tanpa terpengaruh panas internal sensor gas.
