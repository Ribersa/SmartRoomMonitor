# AGENTS.md — Panduan & Aturan Main AI Coding Agent (SmartRoomMonitor)

Dokumen ini adalah panduan wajib bagi setiap AI coding agent (Antigravity, Codex, Claude Code, Cursor, Copilot, Gemini CLI, dsb.) yang bekerja pada repositori **SmartRoomMonitor**. Patuhi seluruh kontrak spesifikasi, keamanan, dan arsitektur di bawah ini sebelum mengubah kode atau dokumentasi apa pun.

---

## 1. Sumber Kebenaran Tunggal (Single Source of Truth)

1. **Baseline Spesifikasi Resmi:** **`docs/SRS-SmartRoomMonitor-v1.0.md`** (8 FR, 8 NFR, 2 Shared FR) yang diselaraskan dengan **Dokumen Pemodelan UML SmartRoomMonitor**.
   - **DILARANG** merujuk atau menyalin penomoran/pembagian tugas dari SRS v0.8.
2. **Kontrak Data Backend Firebase (NFR-05):**
   - Skema JSON: **`firebase/schema-v0.9.json`**
   - Security Rules: **`firebase/database.rules.json`**
   - **DILARANG** mengubah nama key JSON, struktur path `/room1/*` atau `/users/{uid}`, maupun nilai string enum (`"Staff"`, `"Pengunjung"`, `"Device"`, `"auto"`, `"on"`, `"off"`, `"normal"`, `"aktif"`, `"dibungkam"`, `"tidak ada"`, `"bergerak"`, `"diam"`, `"Memanaskan"`, `"Baik"`, `"Sedang"`, `"Buruk"`, `"Gelap"`, `"Terang"`) tanpa menyelaraskan **kedua sisi** (Firmware ESP32-S3 dan Aplikasi Android) sekaligus memperbarui `firebase/schema-v0.9.json` & `firebase/database.rules.json`.

---

## 2. Pembagian Modul & Ruang Lingkup MVP (SRS v1.0)

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

## 3. Aturan Keamanan, Kredensial & Tanggung Jawab Penulisan (WAJIB — NFR-03, NFR-04, FR-06)

1. **Jangan pernah commit kredensial nyata:** Dilarang keras memasukkan `secrets.h`, `secrets.yaml`, `google-services.json`, `.env`, SSID/password Wi-Fi pribadi, Firebase Web API Key, password akun, atau URL RTDB proyek aktif ke dalam kode, log, dokumentasi, maupun riwayat commit Git.
2. Selalu gunakan file template berakhiran `.example` (`secrets.h.example`) dengan nilai placeholder dan verifikasi `.gitignore` sebelum commit.
3. **Kepatuhan RBAC (`firebase/database.rules.json`):**
   - Tanpa login: akses ditolak total.
   - `Pengunjung`: hanya baca `/room1` dan `/users/{uid}` miliknya. UI Android **wajib menyembunyikan atau menonaktifkan** kontrol kipas, lampu, dan tombol MUTE bagi peran `Pengunjung`.
   - `Staff`: boleh baca `/room1`, dan menulis perintah **hanya** ke:
     - `room1/actuator/fanMode` (`"auto"` / `"on"` / `"off"`)
     - `room1/actuator/lampMode` (`"auto"` / `"on"` / `"off"`)
     - `room1/alarm/mute` (`true` / `false`)
   - `thresholds`: hanya diubah oleh admin melalui **Firebase Console** (bukan ditulis dari aplikasi Android).
   - `Device` (ESP32-S3): satu-satunya penulis `room1/current`, `room1/alarm/{active, buzzer, status, reason}`, `room1/history`, dan `room1/events`.
4. **PENTING — Penyelarasan UML Sequence Diagram (UC-03, UC-04, UC-06):**
   - **Aplikasi Android DILARANG menulis ke `room1/events` maupun `room1/alarm/buzzer`.** Evaluasi bahaya (suhu > 35°C, udara buruk, jam malam), aktivasi buzzer lokal, dan pencatatan `room1/events` dikerjakan secara otonom oleh **ESP32-S3 (`Device`)** agar memenuhi **FR-06** (alarm tetap bunyi saat internet putus), **NFR-03** (`Pengunjung` tetap *read-only*), dan mencegah duplikasi log kejadian dari banyak klien HP.

---

## 4. Aturan Pengembangan Aplikasi Android (`app/`)

1. **Stack Wajib (NFR-08):** Android Native, **Kotlin**, **Jetpack Compose (Material 3)**, **MVVM + Clean Architecture**, **Kotlin Coroutines & StateFlow**, `minSdk 26` (Android 8.0+), Firebase Auth + Firebase Realtime Database SDK.
2. **Aturan UI & Usabilitas (NFR-07):**
   - Seluruh teks UI berbahasa **Indonesia**.
   - Semua kondisi ruangan tampil pada **satu layar dashboard utama** (plus tab Grafik, Kejadian, Kontrol, Profil sesuai mockup UML).
   - Tombol **MUTE Darurat** harus berukuran `>= 48 dp` dan dapat dieksekusi maksimal **2 tap** (FR-06).
3. **Kontrak Path & Logika Wajib di Sisi Aplikasi (Sinkron UML):**
   - **Peran (`users/{uid}/role`):** Cek nilai `"Staff"` vs `"Pengunjung"` (huruf awal kapital).
   - **Data Live (`room1/current`):** Gunakan `addValueEventListener` untuk membaca `temperature`, `humidity`, `humidityLabel`, `airDelta`, `airLabel`, `lightLux`, `lightLabel`, `presence`, `fanStatus`, `lampStatus`, `airWarmingUp`, dan `timestamp` (Unix epoch **milidetik**, 13 digit).
   - **Deteksi Offline (FR-B02 / UC-12):** Hitung `System.currentTimeMillis() - current.timestamp > 15_000`. Jika terlampaui (atau `timestamp == 0`), tampilkan banner **"Perangkat offline"** sambil mempertahankan data terakhir.
   - **Proteksi Masa Pemanasan Udara (FR-03 & NFR-06):** Jika `airWarmingUp == true` atau `airLabel == "Memanaskan"`, tampilkan status pemanasan dan **jangan munculkan banner bahaya udara**.
   - **Banner Alarm & Mute (`room1/alarm`):** Dengarkan `room1/alarm` (`active`, `status`, `reason`, `mute`). Saat `active == true`, tampilkan banner peringatan dan tombol MUTE (khusus `Staff`) yang memanggil `getReference("room1/alarm/mute").setValue(true)`.
   - **Kontrol Kipas & Lampu (`room1/actuator`):** Mapping tombol UI `"Otomatis"` → `"auto"`, `"Selalu ON"` → `"on"`, `"Selalu OFF"` → `"off"` ke `room1/actuator/fanMode` dan `room1/actuator/lampMode`. Status nyala/mati fisik dibaca dari `room1/current/fanStatus` & `lampStatus`.
   - **Grafik 24 Jam (`room1/history` — FR-04 / UC-07):** Panggil `getReference("room1/history").orderByKey().limitToLast(144).get()` (ESP32 menyimpan 1 titik tiap **10 menit** = tepat 144 titik untuk 24 jam). Hitung min, max, dan rata-rata dari `temperature`, `humidity`, dan `airDelta`.
   - **Event Log (`room1/events` — FR-04):** Baca daftar kejadian (`timestamp` ms, `type`, `severity`, `message`) dengan filter 24 jam dan filter per tipe (`temp_high`, `air_bad`, `night_presence`).

---

## 5. Aturan Pengembangan Firmware ESP32-S3 (`firmware/`)

1. **Hardware Asli (Menggantikan Asumsi Pemodelan Awal di UML Hal. 39):**
   - Mikrokontroler: **ESP32-S3 DevKitC-1 N16R8** (Dual-Core 240 MHz, 16 MB Flash, 8 MB Octal PSRAM). **DILARANG memakai GPIO 26–37**.
   - Sensor & Pinout Terkunci (`config.h`):
     - I2C `GPIO 4 (SDA)` & `GPIO 5 (SCL)` → **SHT30-D** (`0x44`), **ENS160** (`0x53`), **BH1750** (`0x23`)
     - UART1 256.000 bps `GPIO 18 (RX)` & `GPIO 17 (TX)` → **HLK-LD2410B** mmWave 24 GHz
     - Active-LOW PNP Buzzer (VCC 3.3V): `GPIO 6` (`HIGH` = cut-off/mati)
     - MOSFET Driver Kipas D4184: `GPIO 7`
     - Relay Lampu 1-Ch 5V (Open-Drain / Hi-Z): `GPIO 15` (`INPUT` = mati, `OUTPUT + LOW` = nyala)
2. **Pemisahan Dual-Core FreeRTOS (`firmware/smartroom_esp32s3_firebase/`):**
   - **Core 1 (`loop()`):** Eksklusif untuk UART radar non-blocking, pembacaan I2C, baseline `airDelta`, evaluasi aktuator lokal, dan buzzer *offline failsafe* (FR-06). **DILARANG** menjalankan HTTP/TLS atau `delay()` panjang di Core 1.
   - **Core 0 (`FirebaseManager`):** Eksklusif untuk Wi-Fi auto-reconnect non-blocking, NTP WIB (`UTC+7`), Firebase Auth REST (`idToken` refresh tiap 50 menit), dan HTTPS RTDB sync via `SharedState` (FreeRTOS Mutex).
3. **Algoritma yang Wajib Dipertahankan:**
   - **3-Layer Anti-False-Positive Radar** (`on-delay 800 ms`, validasi biometrik diam/gerak, perisai *back-lobe* Gate 0 `65%/50%`).
   - **Kompensasi Lintas Sensor:** Injeksi suhu & RH dari SHT30-D ke ENS160 (`setTempAndHum`).
   - **Baseline `airDelta` + Warm-Up Suppression 3 Menit (NFR-06):** Kunci baseline TVOC di akhir warm-up 3 menit; label `"Buruk"` jika `airDelta >= 160` atau AQI >= 4 pasca warm-up.
   - **Kompensasi Umpan Balik Optik Lampu:** Rekam `_luxPatokanSaatNyala` + histeresis eksternal `+200 Lux` + batas gelap `40 Lux` + jeda kosong `10 detik`.
   - **Histeresis Kipas:** Nyala `> 31.0 °C`, mati `<= 30.0 °C` (atau mengikuti `room1/thresholds`).
4. **Jangan Hapus Firmware Uji Lokal:** Pertahankan `firmware/tests/09_uji_integrasi_modular_ble/` tetap utuh untuk kalibrasi BLE lokal.

---

## 6. Aturan Git & Branching

1. **`main`:** Hanya berisi kode, skema, dan dokumentasi yang sudah terverifikasi/diuji.
2. **Branch Fitur (`feat/*`, `fix/*`):** Gunakan branch terpisah (mis. `feat/firebase-integration`, `feat/android-dashboard`) untuk pengembangan dan integrasi yang belum selesai diuji di perangkat fisik.
3. **Commit Message:** Gunakan gaya ringkas dan jelas (contoh: `feat(android): implement UC-03 temperature dashboard`).
4. Jangan memaksa *force-push* ke `main` dan jangan mengubah riwayat commit yang sudah ditarik anggota tim lain.
