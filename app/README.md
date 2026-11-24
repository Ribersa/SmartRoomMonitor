# Aplikasi Android — SmartRoomMonitor

Direktori ini memuat kode sumber aplikasi mobile Android berbasis **Kotlin + Jetpack Compose** yang terhubung langsung ke **Firebase Realtime Database** dan **Firebase Authentication** sesuai baseline **SRS v0.9**.

---

## 1. Spesifikasi Teknis (Sesuai NFR-07 & NFR-08)

* **Platform:** Android Native
* **Minimum SDK:** API 26 (Android 8.0 Oreo)
* **Target / Compile SDK:** API 34+
* **Bahasa:** Kotlin
* **UI Framework:** Jetpack Compose (Material 3)
* **Arsitektur:** MVVM (Model-View-ViewModel) + Clean Architecture
* **State Management:** Kotlin Coroutines & StateFlow
* **Backend:** Firebase SDK (Firebase Auth & Firebase Realtime Database)

---

## 2. Kontrak Data Firebase Realtime Database v0.9 (Sesuai NFR-05)

Struktur JSON resmi dan aturan akses tersedia pada:
* Skema JSON: [`firebase/schema-v0.9.json`](../firebase/schema-v0.9.json)
* Security Rules: [`firebase/database.rules.json`](../firebase/database.rules.json)

Path utama yang dibaca/ditulis oleh aplikasi Android:
* `users/{uid}/role` — menentukan peran pengguna (`"visitor"` atau `"staff"`).
* `room1/telemetry` — data kondisi ruangan real-time (`temperature`, `humidity`, `humidity_label`, `air_quality_label`, `air_quality_aqi`, `air_quality_tvoc`, `air_quality_eco2`, `air_quality_warming_up`, `light_lux`, `light_label`, `presence_state`, `presence_detected`, `alarm_active`, `alarm_reason`, `night_mode_active`, `device_timestamp`).
* `room1/controls` — membaca status aktual (`fan_status`, `lamp_status`) dan menulis perintah oleh **Staff** (`fan_mode`, `lamp_mode`, `alarm_muted`).
* `room1/thresholds` — membaca batas peringatan yang dikonfigurasi melalui Firebase Console.
* `room1/history` — deret data 24 jam (interval 5 menit: `t`, `h`, `aqi`, `lux`) untuk grafik **FR-04**.
* `room1/events` — daftar log kejadian (`timestamp`, `type`, `severity`, `message`) dengan indeks `timestamp` & `type`.

---

## 3. Daftar Fitur Aplikasi (Sesuai SRS v0.9)

1. **Autentikasi & Peran (FR-B01 / Bersama):** Login email & password. Peran **Pengunjung** (`visitor`) bersifat *read-only*, sedangkan **Staff** (`staff`) dapat mengontrol kipas, lampu, dan membungkam alarm.
2. **Dashboard & Status Offline (FR-B02 / Bersama):** Pembaruan real-time tiap <= 5 detik. Jika `now - device_timestamp > 15 detik`, tampilkan banner *"Perangkat offline"* sambil mempertahankan data terakhir.
3. **Suhu & Peringatan Suhu Tinggi (FR-01 / M1 Syakir):** Menampilkan suhu (°C), kelembapan (%RH + label), serta banner/badge peringatan saat suhu > 35°C.
4. **Kontrol Kipas (FR-02 / M1 Syakir):** Pilihan mode `auto` / `on` / `off` untuk Staff beserta indikator status aktual kipas.
5. **Kualitas Udara & Peringatan (FR-03 / M2 Zakariya):** Menampilkan label *Memanaskan*, *Baik*, *Sedang*, atau *Buruk* (tanpa peringatan selama masa pemanasan 3 menit awal).
6. **Grafik 24 Jam & Event Log (FR-04 / M2 Zakariya):** Visualisasi tren 24 jam (min, max, rata-rata) dari `room1/history` serta daftar kejadian dari `room1/events` dengan filter 24 jam & tipe.
7. **Kehadiran & Jam Malam (FR-05 / M3 Deswiryawan):** Menampilkan status *tidak ada*, *bergerak*, atau *diam*, serta banner bahaya jika terdeteksi pada 22:00–06:00 WIB.
8. **Bunyi Peringatan & Mute Darurat (FR-06 / M3 Deswiryawan):** Tombol MUTE besar (maks. 2 tap, >= 48 dp) untuk Staff yang mengubah `room1/controls/alarm_muted = true` sehingga buzzer berhenti dalam <= 7 detik.
9. **Tingkat Cahaya & Label (FR-07 / M4 Fattih):** Menampilkan nilai lux dan label (*Gelap*, *Normal*, *Terang*) secara real-time.
10. **Kontrol Lampu (FR-08 / M4 Fattih):** Pilihan mode `auto` / `on` / `off` untuk Staff beserta indikator status aktual lampu.
