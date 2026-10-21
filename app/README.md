# Aplikasi Android — SmartRoomMonitor

Direktori ini memuat kode sumber aplikasi mobile Android berbasis **Kotlin + Jetpack Compose** yang terhubung langsung ke **Firebase Realtime Database** dan **Firebase Authentication**.

---

## 1. Spesifikasi Teknis (Sesuai NFR-06)

* **Platform:** Android Native
* **Minimum SDK:** API 26 (Android 8.0 Oreo)
* **Target / Compile SDK:** API 34+
* **Bahasa:** Kotlin
* **UI Framework:** Jetpack Compose (Material 3)
* **Arsitektur:** MVVM (Model-View-ViewModel) + Clean Architecture
* **State Management:** Kotlin Coroutines & StateFlow
* **Backend:** Firebase SDK (Firebase Auth & Firebase Realtime Database)

---

## 2. Struktur Data Firebase Realtime Database (Sesuai NFR-07)

Aplikasi mendengarkan path `room1/` dengan struktur JSON standar berikut:

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

## 3. Fitur Utama Aplikasi (Sesuai SRS v0.8)

1. **Autentikasi & Peran (FR-B01):** Login email & password. Peran dibedakan antara **Pengunjung** (hanya melihat) dan **Staff** (dapat mengontrol aktuator dan mute alarm).
2. **Dashboard & Status Offline (FR-B02):** Pembaruan data tiap 5 detik. Jika data berhenti > 15 detik, muncul banner *"Perangkat offline"*.
3. **Peringatan Suhu Tinggi (FR-01):** Banner/badge saat suhu > 35°C (debounce 60 detik).
4. **Kontrol Kipas (FR-02):** Tombol mode Otomatis / Selalu ON / Selalu OFF.
5. **Kelembapan & Label (FR-03):** Menampilkan label *Kering*, *Nyaman*, atau *Lembap*.
6. **Grafik 24 Jam & Event Log (FR-04):** Visualisasi tren suhu, RH, udara, dan daftar log kejadian.
7. **Kualitas Udara (FR-05):** Menampilkan label *Memanaskan*, *Baik*, *Sedang*, atau *Buruk*.
8. **Mute Darurat (FR-06):** Tombol MUTE besar saat alarm aktif, membungkam bunyi dalam $\le$ 7 detik.
9. **Deteksi Presensi Jam Malam (FR-07):** Status *tidak ada*, *bergerak*, atau *diam*. Alert otomatis jika ada orang pada pukul 22:00–06:00.
10. **Kontrol Lampu (FR-08):** Mode Otomatis / Selalu ON / Selalu OFF berdasarkan cahaya dan presensi.
