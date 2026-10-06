# 🏠 Smart Room Monitor — IoT Application Project

![ESP32](https://img.shields.io/badge/Hardware-ESP32-blue)
![Firebase](https://img.shields.io/badge/Backend-Firebase-orange)
![Status](https://img.shields.io/badge/Status-MVP_Development-green)
![Course](https://img.shields.io/badge/Course-Application_Project-lightgrey)

Sistem monitoring ruangan **real-time** berbasis **ESP32**: memantau suhu, kelembaban, kualitas udara, dan okupansi ruangan. Data dikirim via WiFi ke **Firebase Realtime Database** dan ditampilkan di aplikasi mobile/web berupa dashboard live, grafik histori, dan notifikasi peringatan otomatis.

> 📄 Profil tim & visi proyek: [PROFIL_TIM.md](PROFIL_TIM.md)

---

## ✨ Fitur Utama

| Fitur | Deskripsi | Status |
|-------|------------|--------|
| 🌡️ Dashboard Live | Suhu, kelembaban, kualitas udara, status hunian (update tiap 5 detik) | 🚧 MVP |
| 📈 Grafik Histori 24 Jam | Min / max / rata-rata harian | 🚧 MVP |
| 🚨 Alert Otomatis | Buzzer lokal + notifikasi app saat suhu > 35°C / udara > 300 PPM | 🚧 MVP |
| 🔇 Kontrol dari HP | Mute buzzer, mode LED, mode kipas (Auto/ON/OFF) | 🚧 MVP |
| 📋 Event Log | Catatan `motion detected`, `high-temp`, `poor-air` + timestamp | 💡 Bonus |
| 🌀 Kipas Otomatis | Relay menyalakan kipas jika suhu > 31°C | 💡 Bonus |

---

## 🏗️ Arsitektur Sistem

```
[DHT11 + PIR + MQ-135]
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

### Struktur Data Firebase

```json
{
  "rooms": {
    "room1": {
      "live": { "temp": 31.2, "hum": 72, "air": 180, "motion": 1, "updatedAt": 1234567890 },
      "control": { "buzzerMuted": false, "fanMode": "auto", "ledMode": "auto" },
      "events": { "-Nx123": { "type": "motion", "at": 1234567890 } }
    }
  }
}
```

---

## 🛠️ Hardware & Software

**Hardware (± Rp 250–400rb):**
- ESP32 DevKit V1, DHT11/DHT22, HC-SR501 PIR, MQ-135/MQ-2, buzzer + LED, breadboard + kabel jumper, powerbank (untuk demo)

**Software:**
- Firmware: Arduino IDE / PlatformIO (C++)
- Backend: Firebase Realtime Database (alternatif cepat: Blynk)
- Aplikasi: Flutter / MIT App Inventor (pemula) / React + Web
- Tools: Git, Fritzing (wiring diagram)

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
# Install library: DHT sensor, Firebase ESP Client
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
1. Panaskan DHT11 dengan tangan → suhu naik di app
2. Lambaikan tangan di depan PIR → `Occupied`
3. Dekatkan spidol ke MQ → skor udara naik + buzzer bunyi

> 💡 **Tips demo:** pakai hotspot HP (bukan WiFi kampus), panaskan MQ 2–3 menit sebelum demo.

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
