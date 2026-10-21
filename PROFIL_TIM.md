# Profil Tim - Smart Room Monitor

**Mata Kuliah:** Rekayasa Perangkat Lunak (RPL) / Application Project  
**Proyek:** SmartRoomMonitor — Sistem Monitoring & Otomasi Ruangan IoT  
**Hardware:** ESP32-S3 + Sensirion SHT30-D + ScioSense ENS160/AHT21 + HLK-LD2410 + Rohm BH1750 + Aktuator (Kipas, Lampu, Buzzer)  
**Aplikasi:** Android Native (Kotlin + Jetpack Compose) + Firebase Realtime Database  

---

## 1. Anggota Tim & Pembagian Tugas (Sesuai SRS v0.8)

| No | Nama | NIM | Peran Tim | Modul Tanggung Jawab (SRS) |
|---|---|---|---|---|
| 1 | **Deswiryawan Saragih** | 241111008 | Product Owner / M4 Lead | **FR-07** (Presensi Jam Malam), **FR-08** (Kontrol Lampu), **NFR-07** (Standarisasi Data), **NFR-08** (Akurasi Sensor) |
| 2 | **Akhmad Zakariya** | 241111010 | Scrum Master / M2 Lead | **FR-03** (Kelembapan & Label), **FR-04** (Grafik 24 Jam & Event Log), **NFR-03** (Keamanan Akses), **NFR-04** (Aturan Firebase) |
| 3 | **Fattih Kuwaka Bekti** | 241111005 | Developer / M3 Lead (Hardware & Firmware) | **FR-05** (Kualitas Udara & Warning), **FR-06** (Bunyi Buzzer & Mute Darurat), **NFR-05** (Usabilitas), **NFR-06** (Kompatibilitas) |
| 4 | **Ahmad Syakir Lathifuddin** | 241111026 | Developer / M1 Lead (App & Backend) | **FR-01** (Suhu & Warning Tinggi), **FR-02** (Kontrol Kipas Otomatis), **NFR-01** (Kinerja Respon), **NFR-02** (Keandalan Koneksi) |

> **Kebutuhan Bersama:** **FR-B01** (Autentikasi Email/Password & Peran Pengunjung/Staff) dan **FR-B02** (Dashboard Real-time 5s & Deteksi Offline 15s) dikerjakan secara kolaboratif.

---

## 2. Visi & Tujuan Proyek

**Visi:**
> *"Mewujudkan pemantauan mikroklimat, kualitas udara, efisiensi energi, dan keamanan ruangan secara otomatis dan real-time melalui sistem IoT yang terintegrasi dengan aplikasi mobile modern, stabil, dan mudah digunakan."*

**Masalah yang Diselesaikan:**
1. **Pemantauan Pasif & Manual:** Suhu tinggi atau udara memburuk baru disadari saat penghuni ruangan merasa tidak nyaman atau mengeluh.
2. **Pemborosan Energi:** Kipas dan lampu menyala tanpa henti saat ruangan kosong atau kondisi cahaya sudah cukup terang.
3. **Keamanan di Luar Jam Operasional:** Tidak ada pendeteksian otomatis terhadap aktivitas di dalam ruangan pada jam malam (22:00–06:00).
4. **Keterbatasan Sensor PIR Biasa:** Sensor gerak konvensional gagal mendeteksi keberadaan orang yang sedang diam, duduk bekerja, atau tidur.

**Solusi:**
Sistem **SmartRoomMonitor** membaca parameter suhu dan kelembapan (SHT30-D), kualitas udara TVOC/eCO2/AQI (ENS160), tingkat cahaya (BH1750), dan keberadaan orang bergerak/diam (Radar mmWave HLK-LD2410) setiap 5 detik menggunakan **ESP32-S3**. Data disinkronkan ke **Firebase Realtime Database**, ditampilkan pada aplikasi **Android (Kotlin + Jetpack Compose)**, dan secara otomatis mengendalikan kipas (MOSFET), lampu (Relay), serta alarm darurat (Buzzer) dengan fail-safe lokal.

---

## 3. Target MVP & Definisi Selesai (Definition of Done)

* [x] **Spesifikasi Software Disahkan:** Dokumen SRS v0.8 lengkap (8 FR, 8 NFR, batasan sistem).
* [x] **Skematik & Desain Elektrik Selesai:** Blueprint breadboard ESP32-S3 dan optimasi Sensirion SHT30-D.
* [x] **Pengadaan Komponen Terinci:** Daftar belanja Shopee tervalidasi dan siap rakit.
* [ ] **Sprint 1 (Hardware & Firmware Proof-of-Concept):** Seluruh 4 sensor terbaca di serial monitor ESP32-S3 dan data terkirim ke Firebase node `room1/telemetry`.
* [ ] **Sprint 2 (Android UI & Auth):** Aplikasi Android menampilkan dashboard reaktif 5 detik, grafik 24 jam, dan pembedaan peran Pengunjung vs Staff.
* [ ] **Sprint 3 (Otomasi & Fail-safe Testing):** Kipas otomatis menyala saat suhu > 31°C, lampu menyala saat gelap + ada orang, dan alarm berbunyi saat jam malam (22:00-06:00) dengan tombol mute $\le$ 7 detik.
* [ ] **Sprint 4 (Integrasi & Uji Demo):** Uji kestabilan hotspot HP selama 15 menit tanpa putus dan demonstrasi akhir.
