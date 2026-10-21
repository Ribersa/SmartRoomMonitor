# 🏠 SmartRoomMonitor — IoT Application Project

![ESP32-S3](https://img.shields.io/badge/Hardware-ESP32--S3_N16R8-blue?logo=espressif)
![Android](https://img.shields.io/badge/Platform-Android_minSdk_26+-green?logo=android)
![Kotlin](https://img.shields.io/badge/Language-Kotlin_Jetpack_Compose-purple?logo=kotlin)
![Firebase](https://img.shields.io/badge/Backend-Firebase_RTDB_%26_Auth-orange?logo=firebase)
![Course](https://img.shields.io/badge/Course-Rekayasa_Perangkat_Lunak_(RPL)-lightgrey)
![Status](https://img.shields.io/badge/Status-SRS_v0.8_Approved-brightgreen)

Sistem pemantauan dan otomatisasi kondisi ruangan **real-time** berbasis **ESP32-S3**: memantau suhu dan kelembapan presisi (**Sensirion SHT30-D**), kualitas udara multi-parameter AQI/TVOC/eCO2 (**ScioSense ENS160**), intensitas cahaya ruangan (**Rohm BH1750**), dan pelacakan keberadaan manusia diam maupun bergerak (**Radar mmWave 24 GHz HLK-LD2410**). 

Data dikirimkan secara kontinu via Wi-Fi ke **Firebase Realtime Database** dan divisualisasikan pada aplikasi mobile **Android Native (Kotlin + Jetpack Compose)**, lengkap dengan kontrol otomatis kipas (PWM MOSFET), saklar lampu adaptif (Relay), alarm jam malam (Buzzer), serta grafik riwayat kejadian 24 jam.

> 📄 **Dokumentasi Lengkap:**
> * Profil Tim & Pembagian Modul: [`PROFIL_TIM.md`](PROFIL_TIM.md)
> * Dokumen SRS Resmi: [`docs/SRS-SmartRoomMonitor-v0.8.md`](docs/SRS-SmartRoomMonitor-v0.8.md)
> * Blueprint Wiring Breadboard: [`docs/blueprint-esp32s3-prototipe.md`](docs/blueprint-esp32s3-prototipe.md)
> * Sinkronisasi Spesifikasi & JSON: [`docs/sinkronisasi-sensor-srs.md`](docs/sinkronisasi-sensor-srs.md)
> * Daftar Belanja & Anggaran Komponen: [`docs/daftar kebutuhan.pdf`](docs/daftar%20kebutuhan.pdf)

---

## ✨ Fitur Utama (Sesuai SRS v0.8)

Sistem dirancang berdasarkan **8 Kebutuhan Fungsional (FR)** dan **8 Kebutuhan Non-Fungsional (NFR)** yang dibagi rata ke seluruh anggota tim:

| ID | Fitur Utama | Deskripsi Fungsional | PIC Modul |
|---|---|---|---|
| **FR-01** | 🌡️ Suhu & Peringatan Dini | Menampilkan suhu (°C). Badge/banner & alarm berbunyi saat suhu > 35°C (debounce 60 detik). | M1 Syakir |
| **FR-02** | 🌀 Kontrol Kipas Otomatis | Mode *Otomatis / Selalu ON / Selalu OFF*. Pada Otomatis, kipas menyala saat suhu > 31°C dan mati saat $\le$ 30°C (histeresis 1°C). | M1 Syakir |
| **FR-03** | 💧 Kelembapan & Label | Menampilkan RH (%) dengan label ramah pengguna: *Kering (<40%)*, *Nyaman (40–60%)*, atau *Lembap (>60%)*. | M2 Zakariya |
| **FR-04** | 📈 Grafik 24 Jam & Event Log | Visualisasi tren 24 jam (min, max, rata-rata) serta pencatatan daftar log kejadian berbasis warna dan filter waktu. | M2 Zakariya |
| **FR-05** | 🍃 Kualitas Udara (IAQ) | Menampilkan status kualitas udara: *Memanaskan*, *Baik*, *Sedang*, atau *Buruk*. Alert & buzzer aktif jika udara Buruk. | M3 Fattih |
| **FR-06** | 🔇 Mute Alarm Cepat | Alarm lokal tetap berbunyi saat offline. Staff dapat membungkam alarm lewat tombol MUTE di aplikasi dalam waktu $\le$ 7 detik. | M3 Fattih |
| **FR-07** | 🧍 Presensi & Jam Malam | Status kehadiran (*tidak ada / bergerak / diam*). Memicu alarm keamanan otomatis jika ada orang di jam **22:00–06:00**. | M4 Deswiryawan |
| **FR-08** | 💡 Kontrol Lampu Adaptif | Mode *Otomatis / Selalu ON / Selalu OFF*. Pada Otomatis, lampu menyala saat ruangan gelap **DAN** ada orang. | M4 Deswiryawan |
| **FR-B01** | 🔐 Autentikasi & Peran | Login/logout email & password. Membedakan hak akses antara peran **Pengunjung** (read-only) dan **Staff** (kontrol & mute). | Bersama |
| **FR-B02** | 📡 Dashboard & Deteksi Offline | Update data real-time tiap 5 detik. Banner *"Perangkat offline"* otomatis muncul jika data terhenti > 15 detik. | Bersama |

---

## 🏗️ Arsitektur Sistem

```text
  [ SENSOR MIKROKLIMAT ]       [ SENSOR UDARA ]         [ SENSOR CAHAYA ]        [ RADAR PRESENSI ]
     Sensirion SHT30-D         ScioSense ENS160            Rohm BH1750           Hilink HLK-LD2410
        (I2C 0x44)                (I2C 0x52)                (I2C 0x23)             (UART2 256000)
             │                         │                         │                        │
             +─────────────────────────+─────────────────────────+                        │
                                       │ (Bus I2C: GPIO 4 & 5)                            │ (GPIO 17 & 18)
                                       ▼                                                  ▼
                               +─────────────────────────────────────────────────────────────+
                               |                  ESP32-S3 DevKitC-1 N16R8                   |
                               |          (Dual-Core 240 MHz, 16MB Flash, 8MB PSRAM)         |
                               +─────────────────────────────────────────────────────────────+
                                       │                      │                       │
                (GPIO 7: PWM 25kHz)    │ (GPIO 15: Digital)   │ (GPIO 6: Digital)     │ (Wi-Fi 2.4 GHz)
                        ▼              ▼                      ▼                       ▼
                 [ Driver MOSFET ]  [ Modul Relay 5V ]  [ Active Buzzer ]    [ Firebase Realtime DB ]
                        │              │                      │                       │
                        ▼              ▼                      ▼                       ▼
                 [ Kipas DC 5V ]    [ Bohlam USB 5V ]   [ Alarm Lokal ]     [ Android App (Compose) ]
```

---

## 🔌 Pemetaan Pinout Hardware (ESP32-S3)

| Pin ESP32-S3 | Komponen Terhubung | Protokol / Fungsi | Keterangan Teknis |
|---|---|---|---|
| **3V3** | SHT30-D, ENS160, BH1750 | Suplai Daya 3.3V DC | Rail 3.3V Sensor I2C |
| **VIN (5V)** | Radar LD2410, Buzzer, Kipas, Relay | Suplai Daya 5V DC | Suplai dari Adaptor USB 5V 2A |
| **GND** | Semua Modul | Ground Bersama | Titik referensi *common ground* |
| **GPIO 4** | SHT30-D, ENS160, BH1750 | **I2C SDA** | Bus Data I2C bersama (logika 3.3V) |
| **GPIO 5** | SHT30-D, ENS160, BH1750 | **I2C SCL** | Bus Clock I2C bersama (logika 3.3V) |
| **GPIO 18** | HLK-LD2410 | **UART RX (S3)** | Input data serial telemetri radar |
| **GPIO 17** | HLK-LD2410 | **UART TX (S3)** | Output perintah konfigurasi radar |
| **GPIO 6** | Modul Active Buzzer | **Digital OUT** | Alarm jam malam & bahaya udara |
| **GPIO 7** | Modul MOSFET D4184 | **PWM OUT (LEDC)** | Kontrol RPM kipas 5V halus (25 kHz) |
| **GPIO 15** | Modul Relay 1-Channel | **Digital OUT** | Pemicu saklar bohlam USB 5V |
| **GPIO 2** | Status LED | **Digital OUT** | Indikator visual koneksi Wi-Fi |

---

## 🗄️ Standarisasi Skema Data Firebase (NFR-07)

Struktur data disepakati bersama antara pengembang firmware dan aplikasi Android:

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

## 📁 Struktur Repositori

```text
SmartRoomMonitor/
├── .gitignore              # Proteksi file kredensial & build sampah (NFR-04)
├── PROFIL_TIM.md           # Profil tim, visi proyek, dan pembagian modul
├── README.md               # Dokumentasi utama proyek
├── docs/                   # Berkas spesifikasi teknis dan pengadaan resmi
│   ├── SRS-SmartRoomMonitor-v0.8.md
│   ├── sinkronisasi-sensor-srs.md
│   ├── blueprint-esp32s3-prototipe.md
│   ├── daftar-kebutuhan-prototipe-s3.md
│   ├── keranjang-belanja-shopee.md
│   ├── daftar kebutuhan.docx
│   └── daftar kebutuhan.pdf
├── firmware/               # Kode program ESP32-S3 (PlatformIO / Arduino / ESPHome)
│   ├── README.md           # Panduan perakitan & library firmware
│   ├── sensor-ruangan.yaml # Konfigurasi ESPHome S3 + SHT30
│   └── secrets.yaml.example
└── app/                    # Aplikasi mobile Android (Kotlin + Jetpack Compose)
    └── README.md           # Panduan arsitektur Android MVVM & konfigurasi Firebase
```

---

## 👥 Tim Pengembang (Kelompok RPL)

| Nama | NIM | Peran Utama |
|---|---|---|
| **Deswiryawan Saragih** | 241111008 | Product Owner / Modul M4 |
| **Akhmad Zakariya** | 241111010 | Scrum Master / Modul M2 |
| **Fattih Kuwaka Bekti** | 241111005 | Hardware & Firmware Lead / Modul M3 |
| **Ahmad Syakir Lathifuddin** | 241111026 | Mobile App & Backend Lead / Modul M1 |

---

## 📜 Lisensi & Etika Akademik

Proyek ini dikembangkan untuk pemenuhan tugas mata kuliah **Rekayasa Perangkat Lunak (RPL)**. Bebas digunakan dan dimodifikasi untuk tujuan pembelajaran dan pengembangan akademik.
