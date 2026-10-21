# Firmware — SmartRoomMonitor

Direktori ini memuat kode program mikrokontroler **ESP32-S3** untuk membaca sensor lingkungan, mengendalikan aktuator lokal, dan menyinkronkan data ke **Firebase Realtime Database**.

---

## 1. Spesifikasi Hardware

* **Mikrokontroler:** ESP32-S3 DevKitC-1 N16R8 (Dual-core Xtensa LX7 @ 240 MHz, 16MB Flash, 8MB PSRAM, USB Type-C)
* **Sensor Mikroklimat:** Sensirion SHT30-D (I2C `0x44`) — Suhu presisi ±0.2°C & RH ±2%
* **Sensor Kualitas Udara:** ScioSense ENS160 (I2C `0x52`) — AQI (1–5 UBA), TVOC (ppb), eCO2 (ppm)
* **Sensor Presensi:** Hilink HLK-LD2410 (UART2 @ 256000 baud) — mmWave 24 GHz
* **Sensor Cahaya:** Rohm BH1750 (I2C `0x23`) — Iluminansi Lux (0–65535 lx)
* **Aktuator:**
  * Modul Driver MOSFET D4184 (`GPIO 7`) $\rightarrow$ Kipas DC 5V (PWM 25 kHz)
  * Modul Relay 1-Channel 5V (`GPIO 15`) $\rightarrow$ Bohlam Lampu USB 5V
  * Modul Active Buzzer 5V (`GPIO 6`) $\rightarrow$ Alarm Keamanan & Peringatan Bahaya

---

## 2. Pemetaan Pinout Resmi (ESP32-S3)

| Pin ESP32-S3 | Komponen Terhubung | Protokol / Fungsi | Catatan |
|---|---|---|---|
| **3V3** | SHT30-D, ENS160, BH1750 | Suplai Daya 3.3V DC | Rail 3.3V Sensor I2C |
| **VIN (5V)** | Radar LD2410, Buzzer, Kipas, Relay | Suplai Daya 5V DC | Jalur Adaptor USB 5V 2A |
| **GND** | Semua Modul | Ground Bersama | Common ground sentral |
| **GPIO 4** | SHT30-D, ENS160, BH1750 | **I2C SDA** | Bus Data I2C bersama (logika 3.3V) |
| **GPIO 5** | SHT30-D, ENS160, BH1750 | **I2C SCL** | Bus Clock I2C bersama (logika 3.3V) |
| **GPIO 18** | HLK-LD2410 | **UART RX (ESP32)** | Menerima data dari TX radar |
| **GPIO 17** | HLK-LD2410 | **UART TX (ESP32)** | Mengirim perintah ke RX radar |
| **GPIO 16** | HLK-LD2410 | **Digital IN (OUT)** | *(Opsional)* Sinyal deteksi cepat |
| **GPIO 6** | Modul Active Buzzer | **Digital OUT** | Alarm jam malam (22:00-06:00) & bahaya |
| **GPIO 7** | Modul MOSFET D4184 | **PWM OUT (LEDC)** | Kontrol RPM kipas 5V halus (25 kHz) |
| **GPIO 15** | Modul Relay 1-Channel | **Digital OUT** | Saklar otomatis lampu USB |
| **GPIO 2** | Status LED | **Digital OUT** | Indikator visual koneksi Wi-Fi |

> [!CAUTION]
> **Larangan Pin:** Jangan gunakan `GPIO 26 s/d 37` karena terhubung ke Octal Flash/PSRAM internal ESP32-S3.

---

## 3. Opsi Firmware

1. **C++ (PlatformIO / Arduino Core) — *Direkomendasikan untuk Produksi Tugas***:
   * Menggunakan library:
     * `Firebase-ESP-Client` by Mobizt
     * `ClosedCube SHT31D` atau `Adafruit SHT31` (kompatibel SHT30)
     * `ScioSense ENS160`
     * `hp-BH1750`
     * `ld2410` by ncmreynolds
2. **ESPHome (Alternatif Pengujian Cepat)**:
   * File konfigurasi siap pakai tersedia di [`sensor-ruangan.yaml`](sensor-ruangan.yaml).
   * Kredensial diisi melalui template [`secrets.yaml.example`](secrets.yaml.example).
