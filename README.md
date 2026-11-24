# 🏠 SmartRoomMonitor — IoT Application Project

![ESP32-S3](https://img.shields.io/badge/Hardware-ESP32--S3_N16R8-blue?logo=espressif)
![Android](https://img.shields.io/badge/Platform-Android_minSdk_26+-green?logo=android)
![Kotlin](https://img.shields.io/badge/Language-Kotlin_Jetpack_Compose-purple?logo=kotlin)
![Firebase](https://img.shields.io/badge/Backend-Firebase_RTDB_%26_Auth-orange?logo=firebase)
![Course](https://img.shields.io/badge/Course-Rekayasa_Perangkat_Lunak_(RPL)-lightgrey)
![Status](https://img.shields.io/badge/Status-SRS_v0.9_Approved-brightgreen)

Sistem pemantauan dan otomatisasi kondisi ruangan **real-time** berbasis **ESP32-S3**: memantau suhu dan kelembapan presisi (**Sensirion SHT30-D**), kualitas udara multi-parameter AQI/TVOC/eCO2 (**ScioSense ENS160**), intensitas cahaya ruangan (**Rohm BH1750**), dan pelacakan keberadaan manusia diam maupun bergerak (**Radar mmWave 24 GHz HLK-LD2410**). 

Data dikirimkan secara kontinu via Wi-Fi ke **Firebase Realtime Database** dan divisualisasikan pada aplikasi mobile **Android Native (Kotlin + Jetpack Compose)**, lengkap dengan kontrol otomatis kipas (PWM MOSFET), saklar lampu adaptif (Relay), alarm jam malam (Buzzer), serta grafik riwayat kejadian 24 jam.

> [!IMPORTANT]
> ## 🔥 MULAI DI SINI — Kontrak Firebase untuk Aplikasi Android (SRS v0.9)
> Pengembang aplikasi Android (**Kotlin + Jetpack Compose**) **cukup mengacu pada 3 berkas Firebase berikut** untuk membangun model data, autentikasi peran, dashboard, grafik 24 jam, dan kontrol perangkat:
>
> * **Skema JSON Resmi (Kontrak Field & Tipe Data):** [`firebase/schema-v0.9.json`](firebase/schema-v0.9.json)
> * **Security Rules & Matriks Peran (`visitor` / `staff` / `device`):** [`firebase/database.rules.json`](firebase/database.rules.json)
> * **Panduan Setup Firebase Console & Hak Akses:** [`firebase/README.md`](firebase/README.md)
> * **Aturan Kerja AI Coding Agent untuk Repo Ini:** [`AGENTS.md`](AGENTS.md)
>
> **Ringkasan Path RTDB yang Dipakai Aplikasi:**
> | Path Firebase RTDB | Akses Aplikasi | Kegunaan di Aplikasi Android |
> |---|---|---|
> | `users/{uid}/role` | Read (milik sendiri) | Menentukan peran `"visitor"` (read-only) atau `"staff"` (boleh kontrol & mute). |
> | `room1/telemetry` | Read (live tiap 3–5 dtk) | Suhu, kelembapan, label udara (`Memanaskan`/`Baik`/`Sedang`/`Buruk`), lux, kehadiran (`tidak ada`/`bergerak`/`diam`), status alarm, dan `device_timestamp` (deteksi offline > 15 dtk). |
> | `room1/controls` | Read semua; **Staff** Write `fan_mode`, `lamp_mode`, `alarm_muted` | Saklar mode kipas (`auto`/`on`/`off`), mode lampu (`auto`/`on`/`off`), tombol MUTE darurat (`<= 7 dtk`), serta baca status aktual `fan_status` & `lamp_status`. |
> | `room1/thresholds` | Read-only (diubah via Firebase Console) | Menampilkan batas peringatan aktif (suhu > 35°C, kipas > 31°C, jam malam 22:00–06:00). |
> | `room1/history/{epoch}` | Read (24 jam, interval 5 menit) | Sumber data grafik 24 jam (`t`, `h`, `aqi`, `lux`) beserta nilai min/max/rata-rata (**FR-04**). |
> | `room1/events/{pushId}` | Read (terindeks `timestamp` & `type`) | Daftar log kejadian (`temp_high`, `air_bad`, `night_presence`) dengan filter 24 jam & tipe (**FR-04**). |

> 📄 **Dokumentasi Pendukung Lainnya:**
> * Panduan Aplikasi Android: [`app/README.md`](app/README.md)
> * Dokumen SRS Resmi (Baseline): [`docs/SRS-SmartRoomMonitor-v0.9.md`](docs/SRS-SmartRoomMonitor-v0.9.md)
> * Sinkronisasi Hardware & SRS v0.9: [`docs/sinkronisasi-sensor-srs.md`](docs/sinkronisasi-sensor-srs.md)
> * Profil Tim & Pembagian Modul: [`PROFIL_TIM.md`](PROFIL_TIM.md)
> * Panduan Pinout & Wiring ESP32-S3: [`docs/lembar_panduan_pinout_dan_wiring_esp32s3.md`](docs/lembar_panduan_pinout_dan_wiring_esp32s3.md)

---

## ✨ Fitur Utama (Sesuai SRS v0.9)

Sistem dirancang berdasarkan **8 Kebutuhan Fungsional (FR)** dan **8 Kebutuhan Non-Fungsional (NFR)** yang dibagi rata ke seluruh anggota tim:

| ID | Fitur Utama | Deskripsi Fungsional | PIC Modul |
|---|---|---|---|
| **FR-01** | 🌡️ Suhu & Peringatan Suhu Tinggi | Menampilkan suhu (°C). Badge/banner & alarm berbunyi saat suhu > 35°C (debounce 60 detik). | M1 Syakir |
| **FR-02** | 🌀 Kontrol Kipas | Mode *Otomatis / Selalu ON / Selalu OFF*. Pada Otomatis, kipas menyala saat suhu > 31°C dan mati saat $\le$ 30°C (histeresis 1°C). | M1 Syakir |
| **FR-03** | 🍃 Kualitas Udara & Peringatan | Menampilkan status kualitas udara: *Memanaskan*, *Baik*, *Sedang*, atau *Buruk*. Alert & buzzer aktif jika udara Buruk (3 menit pertama warm-up). | M2 Zakariya |
| **FR-04** | 📈 Grafik 24 Jam & Event Log | Visualisasi tren 24 jam (min, max, rata-rata) serta pencatatan daftar log kejadian berbasis warna dan filter waktu. | M2 Zakariya |
| **FR-05** | 🧍 Kehadiran & Peringatan Jam Malam | Status kehadiran (*tidak ada / bergerak / diam*). Memicu banner/badge dan bunyi peringatan jika ada orang pada jam **22:00–06:00**. | M3 Deswiryawan |
| **FR-06** | 🔇 Bunyi Peringatan & Mute Darurat | Perangkat berbunyi saat ada peringatan (tetap berbunyi lokal saat offline). Staff dapat membungkam alarm maksimal 2 tap dalam $\le$ 7 detik. | M3 Deswiryawan |
| **FR-07** | 💡 Tingkat Cahaya & Label | Menampilkan tingkat cahaya (lux) atau tingkat kecerahan cahaya di aplikasi secara real-time. | M4 Fattih |
| **FR-08** | 💡 Kontrol Lampu | Mode *Otomatis / Selalu ON / Selalu OFF*. Pada Otomatis, lampu menyala saat gelap dan ada orang, dan mati saat terang/kosong. | M4 Fattih |
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

## 🗄️ Standarisasi Skema Data Firebase v0.9 (NFR-05)

Struktur data resmi dan aturan keamanan (*Security Rules* berbasis peran) tersedia di:
* Skema JSON Lengkap: [`firebase/schema-v0.9.json`](firebase/schema-v0.9.json)
* Aturan Keamanan RTDB: [`firebase/database.rules.json`](firebase/database.rules.json)
* Panduan Setup Firebase Console: [`firebase/README.md`](firebase/README.md)

---

## 📁 Struktur Repositori

```text
SmartRoomMonitor/
├── .gitignore                              # Proteksi kredensial (secrets.h, google-services.json)
├── AGENTS.md                               # Aturan main & kontrak arsitektur untuk AI coding agents
├── PROFIL_TIM.md                           # Profil tim & pembagian modul SRS v0.9
├── README.md                               # Dokumentasi utama proyek
├── firebase/                               # Kontrak JSON v0.9, database.rules.json & panduan setup
│   ├── schema-v0.9.json
│   ├── database.rules.json
│   └── README.md
├── docs/                                   # Spesifikasi SRS v0.9, wiring, & protokol uji sensor
│   ├── SRS-SmartRoomMonitor-v0.9.md
│   ├── sinkronisasi-sensor-srs.md
│   ├── blueprint-esp32s3-prototipe.md
│   ├── lembar_panduan_pinout_dan_wiring_esp32s3.md
│   └── Lembar_Metrik_dan_Protokol_Pengujian_SmartRoomMonitor.docx
├── firmware/                               # Kode ESP32-S3 (Produksi Firebase & Uji Modular)
│   ├── smartroom_esp32s3_firebase/         # Firmware produksi Dual-Core + Firebase RTDB
│   ├── tests/                              # Skrip uji 01 s.d. 09 (termasuk BLE modular)
│   └── README.md
└── app/                                    # Aplikasi Android (Kotlin + Jetpack Compose)
    └── README.md
```

---

## 👥 Tim Pengembang (Sesuai SRS v0.9)

| Nama | NIM | Peran & Modul |
|---|---|---|
| **Deswiryawan Saragih** | 241111008 | Product Owner / Modul M3 (FR-05, FR-06, NFR-05, NFR-06) |
| **Akhmad Zakariya** | 241111010 | Scrum Master / Modul M2 (FR-03, FR-04, NFR-03, NFR-04) |
| **Fattih Kuwaka Bekti** | 241111005 | Developer / Modul M4 (FR-07, FR-08, NFR-07, NFR-08) |
| **Ahmad Syakir Lathifuddin** | 241111026 | Developer / Modul M1 (FR-01, FR-02, NFR-01, NFR-02) |

---

## 📜 Lisensi & Etika Akademik

Proyek ini dikembangkan untuk pemenuhan tugas mata kuliah **Rekayasa Perangkat Lunak (RPL)**. Bebas digunakan dan dimodifikasi untuk tujuan pembelajaran dan pengembangan akademik.
