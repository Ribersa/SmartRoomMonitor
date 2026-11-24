# Aplikasi Android — SmartRoomMonitor

Direktori ini memuat kode sumber aplikasi mobile Android berbasis **Kotlin + Jetpack Compose** yang terhubung langsung ke **Firebase Realtime Database** dan **Firebase Authentication** sesuai baseline **SRS v1.0** dan **Pemodelan UML**.

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

## 2. Kontrak Data Firebase Realtime Database v0.9 (Sinkron UML & NFR-05)

Struktur JSON resmi dan aturan akses tersedia pada:
* Skema JSON: [`firebase/schema-v0.9.json`](../firebase/schema-v0.9.json)
* Security Rules: [`firebase/database.rules.json`](../firebase/database.rules.json)
* Aturan AI Agent: [`AGENTS.md`](../AGENTS.md)

Path utama yang dipakai aplikasi Android:
* `users/{uid}/role` — menentukan peran pengguna (`"Pengunjung"` atau `"Staff"`).
* `room1/current` — data live kondisi ruangan (`temperature`, `humidity`, `humidityLabel`, `airDelta`, `airLabel`, `airAqi`, `airTvoc`, `airEco2`, `airWarmingUp`, `lightLux`, `lightLabel`, `presence`, `presenceDetected`, `fanStatus`, `lampStatus`, `nightModeActive`, `timestamp` dalam ms).
* `room1/actuator` — menulis perintah mode oleh **Staff** (`fanMode`, `lampMode`: `"auto"` / `"on"` / `"off"`).
* `room1/alarm` — membaca status alarm (`active`, `buzzer`, `status`: `"normal"`/`"aktif"`/`"dibungkam"`, `reason`) dan menulis `room1/alarm/mute = true` oleh **Staff**.
* `room1/thresholds` — membaca batas peringatan (diubah lewat Firebase Console).
* `room1/history` — query `orderByKey().limitToLast(144)` (interval 10 menit = 144 titik untuk 24 jam: `temperature`, `humidity`, `airDelta`, `airAqi`, `lightLux`, `timestamp`).
* `room1/events` — daftar log kejadian (`timestamp` ms, `type`, `severity`, `message`) yang ditulis secara otonom oleh ESP32 (`Device`) agar `Pengunjung` tetap *read-only* (**NFR-03**) dan bebas duplikasi.

---

## 3. Daftar Fitur Aplikasi (Sesuai SRS v1.0 & UML UC-01 s.d. UC-12)

1. **Autentikasi & Peran (FR-B01 / UC-01, UC-02, UC-11):** Login email & password. Peran `"Pengunjung"` hanya membaca, sedangkan `"Staff"` dapat mengontrol kipas, lampu, dan membungkam alarm.
2. **Dashboard & Status Offline (FR-B02 / UC-12):** Pembaruan real-time tiap <= 5 detik. Jika `System.currentTimeMillis() - timestamp > 15_000`, tampilkan banner *"Perangkat offline"* sambil mempertahankan data terakhir.
3. **Suhu & Peringatan Suhu Tinggi (FR-01 / UC-03 — M1 Syakir):** Menampilkan suhu (°C), kelembapan (%RH), dan banner peringatan saat `temperature > 35 °C`.
4. **Kontrol Kipas (FR-02 / UC-08 — M1 Syakir):** Pilihan mode `Otomatis` (`auto`) / `Selalu ON` (`on`) / `Selalu OFF` (`off`) di `room1/actuator/fanMode`, status fisik dari `room1/current/fanStatus`.
5. **Kualitas Udara & Peringatan (FR-03 / UC-04 — M2 Zakariya):** Menampilkan label *Memanaskan*, *Baik*, *Sedang*, atau *Buruk* beserta `airDelta` dari baseline (tanpa peringatan saat `airWarmingUp == true`).
6. **Grafik 24 Jam & Event Log (FR-04 / UC-07 — M2 Zakariya):** Grafik 144 titik (interval 10 menit) dari `room1/history` + min/max/rata-rata, serta daftar kejadian dari `room1/events` dengan filter 24 jam & tipe.
7. **Kehadiran & Jam Malam (FR-05 / UC-06 — M3 Deswiryawan):** Status *tidak ada*, *bergerak*, atau *diam* + banner bahaya jam malam 22:00–06:00 WIB.
8. **Bunyi Peringatan & Mute Darurat (FR-06 / UC-10 — M3 Deswiryawan):** Tombol MUTE besar (`>= 48 dp`, maks. 2 tap) untuk Staff yang menulis `room1/alarm/mute = true` (berhenti dalam <= 7 detik).
9. **Tingkat Cahaya & Label (FR-07 / UC-05 — M4 Fattih):** Menampilkan `lightLux` dan `lightLabel` (`Gelap` < 40 lux, `Terang` >= 40 lux).
10. **Kontrol Lampu (FR-08 / UC-09 — M4 Fattih):** Pilihan mode `Otomatis` (`auto`) / `Selalu ON` (`on`) / `Selalu OFF` (`off`) di `room1/actuator/lampMode`, status fisik dari `room1/current/lampStatus`.


---

## 4. Cara Build

Prasyarat: JDK 17+ (diuji JDK 21), Android SDK dengan `platforms;android-35` dan `build-tools;35.0.0`, Gradle 8.9.

1. Isi `local.properties` di root repo:
   ```properties
   sdk.dir=C\:\\Android\\sdk
   ```
2. Taruh `google-services.json` dari Firebase Console di `app/` (di-ignore `.gitignore`, **jangan di-commit**).
3. Build:
   ```powershell
   gradle :app:assembleDebug
   ```
   APK: `app/build/outputs/apk/debug/app-debug.apk`

Tanpa `google-services.json`, aplikasi tetap ter-build dan layar masuk tampil, tetapi login akan gagal dengan pesan kredensial salah karena Firebase belum terinisialisasi.

Struktur kode: `data/` (auth, rtdb, local Room), `domain/` (model, usecase), `ui/` (login, dashboard, navigation, session, theme), `di/AppContainer.kt` untuk manual DI.