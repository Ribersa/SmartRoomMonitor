# AGENTS.md — Panduan & Aturan Main AI Coding Agent (SmartRoomMonitor)

Dokumen ini adalah panduan wajib bagi setiap AI coding agent (Antigravity, Codex, Claude Code, Cursor, Copilot, Gemini CLI, dsb.) yang bekerja pada repositori **SmartRoomMonitor**. Patuhi seluruh kontrak spesifikasi, keamanan, dan arsitektur di bawah ini sebelum mengubah kode atau dokumentasi apa pun.

---

## 1. Sumber Kebenaran Tunggal (Single Source of Truth)

1. **Baseline Spesifikasi Resmi:** **`docs/SRS-SmartRoomMonitor-v0.9.md`** (8 FR, 8 NFR, 2 Shared FR).
   - **DILARANG** merujuk, mengembalikan, atau menyalin penomoran/pembagian tugas dari SRS v0.8 atau dokumen arsip lama.
2. **Kontrak Data Backend Firebase (NFR-05):**
   - Skema JSON: **`firebase/schema-v0.9.json`**
   - Security Rules: **`firebase/database.rules.json`**
   - **DILARANG** mengubah nama key JSON, struktur path `/room1/*` atau `/users/{uid}`, maupun nilai string enum (`"auto"`, `"on"`, `"off"`, `"visitor"`, `"staff"`, `"device"`, `"tidak ada"`, `"bergerak"`, `"diam"`, `"Memanaskan"`, `"Baik"`, `"Sedang"`, `"Buruk"`) tanpa menyelaraskan **kedua sisi** (Firmware ESP32-S3 dan Aplikasi Android) sekaligus memperbarui `firebase/schema-v0.9.json` & `firebase/database.rules.json`.

---

## 2. Pembagian Modul & Ruang Lingkup MVP (SRS v0.9)

| Modul | PIC | Kebutuhan Fungsional & Non-Fungsional |
|---|---|---|
| **M1** | Ahmad Syakir Lathifuddin | FR-01 (Suhu & Alert > 35°C), FR-02 (Kontrol Kipas Histeresis), NFR-01 (Latensi <= 7s), NFR-02 (Keandalan & Auto-Reconnect) |
| **M2** | Akhmad Zakariya | FR-03 (Kualitas Udara & Warm-up), FR-04 (Grafik 24 Jam & Event Log), NFR-03 (Keamanan Peran), NFR-04 (Keamanan Kredensial) |
| **M3** | Deswiryawan Saragih (PO) | FR-05 (Kehadiran & Jam Malam 22:00–06:00), FR-06 (Alarm Lokal & Mute <= 7s), NFR-05 (Keterpeliharaan), NFR-06 (Akurasi & Warm-up 3m) |
| **M4** | Fattih Kuwaka Bekti | FR-07 (Tingkat Cahaya Lux & Label), FR-08 (Kontrol Lampu Adaptif), NFR-07 (Usabilitas UI), NFR-08 (Kompatibilitas Android) |
| **Bersama** | Seluruh Tim | FR-B01 (Autentikasi Email/Password & Peran), FR-B02 (Dashboard 5s & Deteksi Offline > 15s) |

**Di Luar Ruang Lingkup MVP (JANGAN ditambahkan kecuali diminta eksplisit):**
Multi-room (selalu gunakan `room1`), web dashboard, OTA update, export PDF, kamera/video/audio, humidifier/dehumidifier, dan push notification (FCM).

---

## 3. Aturan Keamanan & Kredensial (WAJIB — NFR-03 & NFR-04)

1. **Jangan pernah commit kredensial nyata:** Dilarang keras memasukkan `secrets.h`, `secrets.yaml`, `google-services.json`, `.env`, SSID/password Wi-Fi pribadi, Firebase Web API Key, password akun, atau URL RTDB proyek aktif ke dalam kode, log, dokumentasi, maupun riwayat commit Git.
2. Selalu gunakan file template berakhiran `.example` (seperti `firmware/smartroom_esp32s3_firebase/secrets.h.example`) dengan nilai placeholder.
3. Sebelum melakukan `git add` atau `git commit`, verifikasi bahwa tidak ada file sensitif yang ikut ter-stage (`git status`, `git diff --cached`).
4. **Kepatuhan RBAC (`firebase/database.rules.json`):**
   - Tanpa login: akses ditolak total.
   - `visitor` (Pengunjung): hanya baca `/room1` dan `/users/{uid}` miliknya. UI Android **wajib menyembunyikan atau menonaktifkan** kontrol kipas, lampu, dan tombol MUTE bagi peran `visitor`.
   - `staff`: boleh baca `/room1` dan hanya menulis `/room1/controls/fan_mode`, `/room1/controls/lamp_mode`, `/room1/controls/alarm_muted`.
   - `thresholds`: hanya diubah oleh admin melalui **Firebase Console** (bukan ditulis dari aplikasi Android).
   - `device` (ESP32-S3): hanya menulis `telemetry`, `history`, `events`, `controls/fan_status`, `controls/lamp_status`, dan reset `controls/alarm_muted`.

---

## 4. Aturan Pengembangan Aplikasi Android (`app/`)

1. **Stack Wajib (NFR-08):** Android Native, **Kotlin**, **Jetpack Compose (Material 3)**, **MVVM + Clean Architecture**, **Kotlin Coroutines & StateFlow**, `minSdk 26` (Android 8.0+), Firebase Auth + Firebase Realtime Database SDK.
2. **Aturan UI & Usabilitas (NFR-07):**
   - Seluruh teks UI berbahasa **Indonesia**.
   - Semua kondisi ruangan tampil pada **satu layar dashboard utama**.
   - Tombol **MUTE Darurat** harus berukuran `>= 48 dp` dan dapat dieksekusi maksimal **2 tap** (FR-06).
3. **Logika Wajib di Sisi Aplikasi:**
   - **Deteksi Offline (FR-B02):** Bandingkan epoch saat ini (detik) dengan `room1/telemetry/device_timestamp`. Jika selisih `> 15 detik` (atau `device_timestamp == 0`), tampilkan banner **"Perangkat offline"** sambil tetap menampilkan data terakhir.
   - **Proteksi Masa Pemanasan Udara (FR-03 & NFR-06):** Jika `air_quality_warming_up == true` atau `air_quality_label == "Memanaskan"`, tampilkan status pemanasan dan **jangan munculkan banner bahaya udara**.
   - **Tampilan Label:** Tampilkan label yang mudah dipahami (`Memanaskan`/`Baik`/`Sedang`/`Buruk`, `Kering`/`Nyaman`/`Lembap`, `Gelap`/`Normal`/`Terang`, `tidak ada`/`bergerak`/`diam`), bukan hanya angka mentah.
   - **Grafik 24 Jam & Event Log (FR-04):** Baca `room1/history/{epoch}` (key berupa Unix epoch detik, field pendek `t`, `h`, `aqi`, `lux`) untuk menampilkan tren 24 jam + nilai min, max, dan rata-rata; baca `room1/events` dengan dukungan filter 24 jam dan filter tipe (`temp_high`, `air_bad`, `night_presence`).
   - **Status Aktuator vs Mode:** Bedakan tombol pilihan mode (`fan_mode`, `lamp_mode`: `"auto"` / `"on"` / `"off"`) dengan indikator status aktual fisik dari ESP32 (`fan_status`, `lamp_status`: `true` / `false`).

---

## 5. Aturan Pengembangan Firmware ESP32-S3 (`firmware/`)

1. **Hardware Target:** **ESP32-S3 DevKitC-1 N16R8** (Dual-Core 240 MHz, 16 MB Flash, 8 MB Octal PSRAM).
   - **LARANGAN PIN:** Jangan pernah menggunakan **GPIO 26 s.d. 37** (jalur internal Octal Flash/PSRAM).
   - **Pinout Terkunci (`config.h`):**
     - I2C Bersama: `GPIO 4` (SDA), `GPIO 5` (SCL) → SHT30-D (`0x44`), ENS160 (`0x53`), BH1750 (`0x23`)
     - UART1 Radar HLK-LD2410B (256.000 bps): `GPIO 18` (RX ESP32), `GPIO 17` (TX ESP32)
     - Active-LOW PNP Buzzer (VCC 3.3V): `GPIO 6` (`HIGH` = mati/cut-off, `LOW` = aktif)
     - MOSFET Driver Kipas D4184: `GPIO 7`
     - Relay Lampu 1-Ch 5V (Open-Drain / Hi-Z): `GPIO 15` (`INPUT` = mati, `OUTPUT + LOW` = nyala)
2. **Pemisahan Dual-Core FreeRTOS (`firmware/smartroom_esp32s3_firebase/`):**
   - **Core 1 (`loop()`):** Eksklusif untuk pembacaan UART radar non-blocking, I2C sensor, evaluasi aktuator lokal, dan alarm buzzer (*offline failsafe* sesuai FR-06). **DILARANG** meletakkan panggilan HTTP/TLS atau `delay()` panjang di Core 1 karena akan membuat buffer UART radar overflow (`DISCONNECT`).
   - **Core 0 (`FirebaseManager`):** Eksklusif untuk Wi-Fi auto-reconnect non-blocking, NTP WIB (`UTC+7`), Firebase Auth REST (`idToken` refresh 50 menit), dan HTTPS RTDB sync via `SharedState` (FreeRTOS Mutex).
3. **Algoritma yang Wajib Dipertahankan:**
   - **3-Layer Anti-False-Positive Radar** (`on-delay 800 ms`, validasi biometrik diam/gerak, dan perisai *back-lobe* Gate 0 `65%/50%`).
   - **Kompensasi Lintas Sensor:** Injeksi suhu & RH dari SHT30-D ke ENS160 (`setTempAndHum`).
   - **Kompensasi Umpan Balik Optik Lampu:** Rekam `_luxPatokanSaatNyala` + histeresis eksternal `+200 Lux` + jeda kosong `10 detik` agar lampu tidak mati-nyala sendiri.
   - **Histeresis Kipas:** Nyala `> 31.0 °C`, mati `<= 30.0 °C` (atau mengikuti `room1/thresholds`).
   - **Warm-Up Suppression ENS160:** 3 menit pertama (`180.000 ms`) setelah boot bebas alarm udara palsu.
4. **Jangan Hapus Firmware Uji Lokal:** Pertahankan `firmware/tests/09_uji_integrasi_modular_ble/` tetap utuh sebagai alat kalibrasi & uji hardware BLE lokal.

---

## 6. Aturan Git & Branching

1. **`main`:** Hanya berisi kode, skema, dan dokumentasi yang sudah terverifikasi/diuji.
2. **Branch Fitur (`feat/*`, `fix/*`):** Gunakan branch terpisah (mis. `feat/firebase-integration`, `feat/android-dashboard`) untuk pengembangan dan integrasi yang belum selesai diuji di perangkat fisik.
3. **Commit Message:** Gunakan gaya ringkas dan jelas (contoh: `feat(android): implement FR-01 temperature alert banner`, `fix(firmware): handle NTP reconnect timeout`).
4. Jangan memaksa *force-push* ke `main` dan jangan mengubah riwayat commit yang sudah ditarik anggota tim lain.
