# 🏠 Smart Room Monitor — IoT Application Project

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue)
![Firebase](https://img.shields.io/badge/Backend-Firebase-orange)
![Status](https://img.shields.io/badge/Status-MVP_Development-green)
![Course](https://img.shields.io/badge/Course-Application_Project-lightgrey)

Sistem monitoring ruangan **real-time** berbasis **ESP32**: memantau suhu, kelembaban, (opsional tekanan udara), kualitas udara, dan **keberadaan manusia bahkan saat diam**. Data dikirim via WiFi ke **Firebase Realtime Database** dan ditampilkan di aplikasi mobile/web berupa dashboard live, grafik histori, dan notifikasi peringatan otomatis.

> 📄 Profil tim & visi proyek: [PROFIL_TIM.md](PROFIL_TIM.md)

---

## ✨ Fitur Utama

| Fitur | Deskripsi | Status |
|-------|------------|--------|
| 🌡️ Dashboard Live | Suhu, kelembaban, kualitas udara, presence + jarak (update tiap 5 detik) | 🚧 MVP |
| 🧍 Deteksi Orang Diam | Status `Kosong / Bergerak / Diam` + `diam selama X menit` via radar | 🚧 MVP |
| 📈 Grafik Histori 24 Jam | Min / max / rata-rata harian | 🚧 MVP |
| 🚨 Alert Otomatis | Buzzer lokal + notifikasi app saat suhu > 35°C / udara > 300 PPM / ada orang jam malam | 🚧 MVP |
| 🔇 Kontrol dari HP | Mute buzzer, mode LED, mode kipas (Auto/ON/OFF) | 🚧 MVP |
| 📋 Event Log | Catatan `presence`, `high-temp`, `poor-air` + timestamp | 💡 Bonus |
| 🌀 Kipas Otomatis | Relay menyalakan kipas jika suhu > 31°C | 💡 Bonus |

---

## 🏗️ Arsitektur Sistem

```
[BME280/DHT20 + HLK-LD2410 + MQ-135]
        │  baca tiap 5 detik
        ▼
   [ESP32 DevKit] ──buzzer/LED──> Alarm lokal
        │ WiFi (HTTP/MQTT)
        ▼
[Firebase Realtime DB]
        │ live listener
        ▼
[Mobile / Web App] ──tulis──> /control (mute, fan, led)
```

### Pilihan Sensor Iklim
- **Opsi A — BME280:** suhu + humidity + tekanan udara. Nilai plus (grafik tekanan), harga ~Rp 35–60rb. Pastikan **BME280 asli, bukan BMP280** (BMP280 tanpa humidity). Tegangan **wajib 3.3V**.
- **Opsi B — DHT20:** suhu + humidity, I2C, murah ~Rp 20–30rb, toleran 3.3V/5V. Cukup untuk MVP.
- Keduanya I2C: SDA GPIO21, SCL GPIO22 di ESP32.

### Sensor Presence
- **HLK-LD2410 (radar 24GHz)** menggantikan PIR. Mendeteksi orang diam (napas/gerak mikro), jarak s/d ~6m, 8 gate jarak.
- Koneksi UART ke ESP32 (baud default 256000) + 5V stabil. Setting sensitivitas via aplikasi HLKRadarTool (Bluetooth, varian LD2410C) atau serial.
- Mounting: tinggi 1.5–2m menghadap tengah ruangan, hindari kipas/metal.

### Struktur Data Firebase

```json
{
  "rooms": {
    "room1": {
      "live": { "temp": 31.2, "hum": 72, "pressure": 1008.2, "air": 180, "presence": "still", "distance": 2.3, "stationaryFor": 480, "updatedAt": 1234567890 },
      "control": { "buzzerMuted": false, "fanMode": "auto", "ledMode": "auto" },
      "events": { "-Nx123": { "type": "presence", "at": 1234567890 } }
    }
  }
}
```
`presence`: `empty` / `moving` / `still`. `pressure` hanya ada jika pakai BME280.

---

## 🛠️ Hardware & Software

**Hardware (± Rp 350–550rb):**
- ESP32 DevKit V1, BME280 **atau** DHT20, HLK-LD2410/LD2410C, MQ-135/MQ-2, buzzer + LED, breadboard + kabel jumper, powerbank (untuk demo)

**Software:**
- Firmware: Arduino IDE / PlatformIO (C++)
- Backend: Firebase Realtime Database (alternatif cepat: Blynk)
- Aplikasi: Flutter / MIT App Inventor (pemula) / React + Web
- Tools: Git, Fritzing (wiring diagram), HLKRadarTool (setting LD2410)

---

## 📁 Struktur Repository

```
SmartRoomMonitor/
├── PROFIL_TIM.md      # Profil tim, peran Scrum, visi proyek
├── README.md          # Dokumentasi utama (file ini)
├── firmware/          # Kode ESP32 (.ino) + daftar library
├── app/               # Kode aplikasi mobile/web
└── docs/              # Proposal, wiring diagram, slide presentasi
```

> Folder `firmware/`, `app/`, `docs/` akan diisi bertahap mengikuti sprint.

---

## 🚀 Quick Start

### 1. Firmware (ESP32)
```bash
# Buka firmware/smart_room_monitor.ino di Arduino IDE
# Install library: Adafruit BME280 + Adafruit Sensor (atau DHT20 by Rob Tillaart), ld2410 by ncmreynolds, Firebase ESP Client
# Isi WiFi SSID/password + Firebase URL/API key
# Upload ke ESP32, buka Serial Monitor 115200
```

### 2. Firebase
1. Buat project di Firebase Console → Realtime Database → mode test
2. Buat path `rooms/room1/live`, `rooms/room1/control`, `rooms/room1/events`
3. Salin URL + API key ke firmware dan aplikasi

### 3. Aplikasi
```bash
# Contoh Flutter:
cd app
flutter pub get
flutter run
```

### 4. Uji Demo (urutan yang selalu berhasil)
1. Genggam sensor iklim dengan tangan → suhu/humidity naik di app
2. Duduk diam di depan LD2410 1–2 menit → status `Diam di ruangan` + jarak tampil
3. Dekatkan spidol ke MQ → skor udara naik + buzzer bunyi

> 💡 **Tips demo:** pakai hotspot HP (bukan WiFi kampus), panaskan MQ 2–3 menit sebelum demo, kalibrasi gate LD2410 di ruangan demo yang sebenarnya.

---

## 🗓️ Roadmap 6 Minggu

- [x] **W1–W2:** 1 sensor live ke Firebase + tampil 1 angka di app (MVP proof)
- [ ] **W3–W4:** 3 sensor + dashboard + grafik histori + threshold
- [ ] **W5:** Alert + buzzer/LED + integration test + laporan
- [ ] **W6:** Buffer bugfix, video demo, slide presentasi

---

## 👥 Tim

| Nama | NIM | Peran |
|------|-----|-------|
| Deswiryawan Saragih | 241111008 | Product Owner |
| Akhmad Zakariya | 241111010 | Scrum Master |
| Fattih Kuwaka Bekti | 241111005 | Developer (Hardware & Firmware) |
| Ahmad Syakir Lathifuddin | 241111026 | Developer (App & Backend) |

---

## 📜 Lisensi

Proyek akademik — bebas digunakan untuk pembelajaran.
