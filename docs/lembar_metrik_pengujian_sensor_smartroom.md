# Lembar Metrik & Protokol Pengujian Empiris Sensor SmartRoomMonitor
**Proyek:** SmartRoomMonitor (RPL - Tim ESP32-S3)  
**Baseline Dokumen:** SRS v0.9 (FR-01 s.d. FR-08 & NFR-01 s.d. NFR-08)  
**Firmware Uji:** [`09_uji_integrasi_modular_ble`](file:///c:/Users/deswi/OneDrive/Dokumen/AP2xRPL/pengujian_hardware/09_uji_integrasi_modular_ble/09_uji_integrasi_modular_ble.ino)  

---

## 1. Ringkasan Variabel Data & Standar Perulangan Tiap Sensor

Tabel ini merangkum seluruh data yang dikeluarkan oleh masing-masing sensor pada layar telemetri beserta jumlah perulangan minimum agar secara statistik valid untuk laporan RPL/skripsi:

| Modul & PIC | Sensor & Aktuator | Data Mentah & Satuan | Data Olahan / Status | Jumlah Perulangan ($N$) | Hal Kritis yang Wajib Diperhatikan |
|---|---|---|---|---|---|
| **M3** (Deswiryawan) | **HLK-LD2410B** (mmWave 24 GHz) + Buzzer | `jarakGerak` ($\text{cm}$), `energiGerak` ($\%$), `jarakDiam` ($\text{cm}$), `energiDiam` ($\%$) | `statusRadar` (`KOSONG`, `VERIF...`, `BERGERAK`, `DIAM`), `adaOrang` (`bool`) | **$10\times$** per skenario jarak/objek; **$5\times$** per halangan | Pastikan tidak ada orang lain bergerak di belakang penguji; tunggu status benar-benar `KOSONG` ($3\text{ dtk}$) sebelum mengulang. |
| **M1** (Syakir) | **SHT30-D** + Kipas MOSFET | `suhu` ($^\circ\text{C}$), `kelembapan` ($\%\text{RH}$) | `statusKipas` (`AUTO [NYALA]` / `[MATI]`), Alert $> 35^\circ\text{C}$ | **$10\times$** sampel stabil; **$5\times$** siklus histeresis | Jangan menyentuh elemen sensor dengan jari (gunakan uap/udara hangat dari jarak $\ge 25\text{ cm}$); jauhkan dari panas modul radar/ENS160. |
| **M2** (Zakariya) | **ENS160** + Buzzer Sirine | `aqi` ($1-5$), `tvoc` ($\text{ppb}$), `eco2` ($\text{ppm}$) | `labelAQI` (`1-Sangat Baik` s.d. `5-Bahaya`), Sirine Level 4/5 | **$5\times$** per jenis polutan & jarak paparan | Wajib lewati masa pemanasan (*warm-up*) **3 menit pertama** (NFR-06); beri jeda tuang udara segar hingga kembali ke AQI 1 antar uji. |
| **M4** (Fattih) | **BH1750** + Relay Lampu 5V | `lux` ($\text{Lux}$), `_luxPatokanSaatNyala` ($\text{Lux}$) | `labelCahaya` (`Gelap`, `Normal`, `Terang`), `statusLampu` | **$5\times$** per kuadran kondisi & jarak cahaya | Perhatikan sudut datang cahaya terhadap kubah sensor; uji kompensasi umpan balik optik agar lampu tidak kedap-kedip sendiri. |

---

## 2. Protokol Pengujian Modul M3: Radar mmWave HLK-LD2410B (FR-05 & FR-06)

Karena radar 24 GHz memiliki variabel fisika paling kompleks (pantulan Doppler, mikro-gerak napas, penetrasi material, dan *false positive*), pengujian dibagi menjadi **5 Dimensi Uji**:

### 2.1 Dimensi A: Akurasi Jarak & Sensitivitas per Gate (Tanpa Penghalang / *Line-of-Sight*)
- **Kondisi Lingkungan:** Ruang terbuka/kamar bebas pergerakan orang lain, mode preset diatur via perintah `'3'` (Kamar Penuh $\sim 4,5\text{m}$) atau `'4'` (Maksimal $\sim 6,0\text{m}$).
- **Perulangan:** **$10\times$ percobaan** pada setiap titik jarak.
- **Metrik yang Dicatat:** Jarak aktual meteran ($D_{\text{aktual}}$) vs Jarak bacaan radar (`jarakGerak` & `jarakDiam`), nilai `energiGerak` ($\%$), `energiDiam` ($\%$), dan Waktu Kunci Verifikasi ($t_{\text{verif}}$, target $\approx 0,8\text{ detik}$).

| ID Uji | Titik Jarak Uji | Gate Target | Aktivitas Penguji | Target Keberhasilan |
|---|---|---|---|---|
| **M3-A1** | $0,75\text{ m}$ ($75\text{ cm}$) | Gate 0–1 | Berdiri lalu duduk diam bernapas | Terdeteksi `BERGERAK` $\rightarrow$ `DIAM` dalam $\le 1,5\text{ dtk}$ ($10/10$ sukses) |
| **M3-A2** | $1,50\text{ m}$ ($150\text{ cm}$) | **Gate 2 (Meja)** | Duduk tenang mengetik & bernapas halus | `energiDiam` bertahan $\ge 15\%$, tidak putus selama $60\text{ dtk}$ ($10/10$) |
| **M3-A3** | $2,25\text{ m}$ ($225\text{ cm}$) | **Gate 3 (Kasur)** | Rebahan / duduk diam menghadap sensor | Terkunci `DIAM` tanpa jatuh ke `KOSONG` ($9/10$ sukses) |
| **M3-A4** | $3,50\text{ m}$ ($350\text{ cm}$) | Gate 4–5 | Berjalan masuk lalu berdiri diam | Terdeteksi `BERGERAK` $\rightarrow$ `DIAM` ($9/10$ sukses) |
| **M3-A5** | $5,00\text{ m}$ ($500\text{ cm}$) | Gate 6–7 | Melangkah di ujung ruangan | Terdeteksi minimal sebagai `BERGERAK` ($8/10$ sukses) |

---

### 2.2 Dimensi B: Variasi Sudut Pancaran Antena (Azimuth & Proteksi *Back-Lobe*)
- **Jarak Tetap:** $1,50\text{ meter}$ dari sensor.
- **Perulangan:** **$5\times$ percobaan** per sudut.

| ID Uji | Sudut Terhadap Muka Antena | Posisi Fisik | Ekspektasi Hasil & Alasan Teknis |
|---|---|---|---|
| **M3-B1** | **$0^\circ$ (Tegak Lurus)** | Tepat di depan sensor | Energi pantul maksimum (`energiGerak` $70-100\%$, `energiDiam` $40-100\%$). |
| **M3-B2** | **$\pm 30^\circ$ (Serong Sedang)** | Di sisi kiri/kanan meja kerja | Masih di dalam kerucut utama ($\pm 60^\circ$); deteksi gerak & napas tetap stabil. |
| **M3-B3** | **$\pm 60^\circ$ (Batas Tepi Beam)** | Di sudut samping ruangan | Batas spesifikasi pabrikan; `energiGerak` mulai turun ke $35-50\%$, napas melemah. |
| **M3-B4** | **$180^\circ$ (Tepat di Belakang PCB)** | Jarak $30-100\text{ cm}$ di **belakang** radar | **WAJIB TETAP `KOSONG`!** Menguji perisai Gate 0 (`65% Gerak / 50% Diam`) agar bocoran radiasi belakang PCB tidak memicu `[VERIF...]`. |

---

### 2.3 Dimensi C: Uji Tembus Halangan Material (*Obstacle Penetration Test* 24 GHz)
Pengujian ini sangat penting untuk menentukan bahan boks (*enclosure*) perangkat dan pengaruh benda di kamar.
- **Setup:** Penguji duduk pada jarak **$1,50\text{ meter}$**, lalu material penghalang ditempatkan tepat di depan antena radar (jarak $5-20\text{ cm}$ dari sensor).
- **Perulangan:** **$5\times$ percobaan** per material (catat penurunan `energiGerak` dan `energiDiam`).

| ID Uji | Material Penghalang | Ketebalan | Ekspektasi Fisika Gelombang 24 GHz | Apa yang Perlu Diperhatikan |
|---|---|---|---|---|
| **M3-C1** | **Tanpa Halangan (Udara)** | - | Baseline ($100\%$ transmisi) | Catat rata-rata `energiDiam` sebagai patokan awal. |
| **M3-C2** | **Kain / Gorden / Selimut** | $1-3\text{ mm}$ | Tembus sempurna (redaman $< 5\%$) | Membuktikan radar tetap mendeteksi orang di balik selimut/tirai. |
| **M3-C3** | **Plastik Akrilik / ABS Boks** | $2-3\text{ mm}$ | Tembus baik (redaman $5-15\%$) | **Syarat wajib boks casing:** Jangan ada cat metalik/logam di plastik. |
| **M3-C4** | **Kardus / Buku Cetak** | $3-10\text{ mm}$ | Tembus sedang (redaman $15-30\%$) | Jika kardus lembap, redaman air akan menurunkan `energiDiam`. |
| **M3-C5** | **Papan Tripleks / Kayu Tipis** | $3-5\text{ mm}$ | Tembus terbatas (redaman $30-50\%$) | Gerakan besar (`BERGERAK`) masih tembus, napas halus (`DIAM`) mulai sulit. |
| **M3-C6** | **Pelat Logam / Aluminium Foil** | $\ge 0,5\text{ mm}$ | **TERBLOKIR TOTAL ($0\%$)** | Gelombang mikro dipantulkan 100% oleh konduktor logam (`KOSONG`). |

---

### 2.4 Dimensi D: Uji Ketahanan *Anti-False-Positive* (Benda Jatuh vs Manusia)
Menguji *State Machine* 3-Lapis di [`SensorManager::evaluasiFilterKehadiran`](file:///c:/Users/deswi/OneDrive/Dokumen/AP2xRPL/pengujian_hardware/09_uji_integrasi_modular_ble/SensorManager.cpp#L120-L195) (`On-Delay 800ms` + `Dual-Path Biometric` + `Hysteresis`):
- **Setup:** Penguji berdiri di luar jangkauan/di belakang sensor hingga status stabil **`[KOSONG]`**.
- **Perulangan:** **$10\times$ percobaan** untuk setiap jenis gangguan.

| ID Uji | Skenario Gangguan (Bukan Manusia) | Durasi Gangguan | Hasil yang Diharapkan (Lulus = *True Negative*) | Metrik Evaluasi |
|---|---|---|---|---|
| **M3-D1** | **Lempar Bantal / Boneka** melintas di depan radar (jarak $1\text{ m}$) | $0,2 - 0,5\text{ detik}$ | Sempat tampil `[VERIF...]` sesaat lalu kembali ke `[KOSONG]`; **`adaOrang` tetap `false`** (Lampu/Alarm tidak nyala). | Hitung *False Positive Rate*: $\text{FPR} = \frac{\text{Jumlah Salah Picu}}{10} \times 100\%$ (Target: **$0\%$**). |
| **M3-D2** | **Jatuhkan Buku / Botol Plastik** ke lantai di depan radar lalu biarkan diam | Jatuh $0,3\text{ dtk}$, lalu diam di lantai | Saat mendarat di lantai, benda mati tidak bernapas (`energiDiam = 0%`) $\rightarrow$ verifikasi gugur ke `[KOSONG]`. | Target FPR: **$0\%$** ($10/10$ diabaikan). |
| **M3-D3** | **Kipas Angin Menyala** di dekat area pantau | Kontinu selama $60\text{ detik}$ | Baling-baling kecil tidak memiliki penampang biometrik diam $\rightarrow$ tetap `[KOSONG]`. | Pastikan kipas tidak diletakkan $< 30\text{ cm}$ tepat di depan muka antena. |
| **M3-D4** | **Manusia Masuk & Duduk** (Kontrol Positif) | Masuk $> 0,8\text{ dtk}$ lalu duduk | Mengunci ke `[BERGERAK]` dalam $\approx 0,8\text{ dtk}$ lalu `[DIAM]`; **`adaOrang = true`**. | Hitung *True Positive Rate*: $\text{TPR} = \frac{\text{Jumlah Terdeteksi}}{10} \times 100\%$ (Target: **$\ge 90\%$**). |

---

## 3. Protokol Pengujian Modul M1: Suhu & Kelembapan SHT30-D + Kipas (FR-01 & FR-02)

### 3.1 Variabel & Hal yang Harus Diperhatikan
- **Inersia Termal:** Sensor suhu butuh waktu $5-15\text{ detik}$ untuk menyeimbangkan suhu keping silikon dengan udara sekitar.
- **Kontaminasi Panas (*Self-Heating / Thermal Bleeding*):** Pastikan SHT30 dipasang di sisi breadboard yang jauh dari modul Radar LD2410 dan ENS160 (minimal berjarak $5-8\text{ cm}$).

### 3.2 Skenario & Jumlah Perulangan
1. **Uji Akurasi Statis (Kondisi Kamar Normal — $N = 10\text{ sampel}$ tiap $30\text{ detik}$):**
   - Bandingkan `suhu` ($^\circ\text{C}$) dan `kelembapan` ($\%\text{RH}$) SHT30 dengan termometer-higrometer digital ruang referensi.
   - Hitung selisih rata-rata (*Mean Absolute Error*): $\text{MAE} = \frac{1}{10}\sum |T_{\text{SHT30}} - T_{\text{ref}}|$ (Target toleransi SHT30: $\pm 0,3^\circ\text{C}$ dan $\pm 3\%\text{RH}$).
2. **Uji Histeresis Kipas Otomatis FR-02 ($N = 5\text{ siklus naik-turun}$):**
   - **Cara memberi stimulus:** Arahkan hembusan udara hangat (*hairdryer* mode rendah atau udara hangat dari jarak $30\text{ cm}$) secara perlahan ke arah SHT30.
   - **Titik pengamatan wajib:**
     - Saat suhu naik melewati **$31,0^\circ\text{C}$** (misal $31,1^\circ\text{C}$) $\rightarrow$ Kipas WAJIB berubah ke `AUTO [NYALA]`.
     - Matikan pemanas, biarkan suhu turun ke **$30,5^\circ\text{C}$** $\rightarrow$ Kipas WAJIB **TETAP MENYALA** (bukti histeresis bekerja!).
     - Saat suhu turun menyentuh **$\le 30,0^\circ\text{C}$** $\rightarrow$ Kipas WAJIB berubah ke `AUTO [MATI]`.
3. **Uji Peringatan Suhu Kritis FR-01 ($> 35,0^\circ\text{C}$ — $N = 3\times$):**
   - Hangatkan udara hingga $> 35,0^\circ\text{C}$, catat waktu respons munculnya peringatan.

---

## 4. Protokol Pengujian Modul M2: Kualitas Udara ENS160 + Buzzer (FR-03 & NFR-06)

### 4.1 Variabel & Hal yang Harus Diperhatikan
- **Kompensasi Suhu & RH Otomatis:** Di kode kita, nilai SHT30 otomatis dikirim ke ENS160 (`_ens160.setTempAndHum`). Pastikan SHT30 terhubung baik agar bacaan `tvoc` dan `eco2` akurat.
- **Jangan Meneteskan Cairan ke Sensor:** Jangan menyemprotkan parfum/alkohol tepat membasahi lubang logam ENS160! Teteskan pada tisu/kapas, lalu dekatkan kapas pada jarak terukur.

### 4.2 Skenario & Jumlah Perulangan
1. **Uji Masa Pemanasan Awal (*Warm-up* 3 Menit — SRS NFR-06, $N = 3\times\text{ cold boot}$):**
   - Nyalakan sistem dari kondisi mati. Pantau selama $180\text{ detik}$ pertama: pastikan sistem tidak membunyikan alarm palsu saat kalibrasi baseline internal berlangsung.
2. **Uji Gradasi Polutan & Jarak Sumber ($N = 5\times$ per skenario):**
   - **Baseline Udara Bersih:** Kamar berventilasi baik $\rightarrow$ Catat `tvoc` ($< 65\text{ ppb}$), `eco2` ($400-500\text{ ppm}$), `aqi = 1`.
   - **Paparan Napas Manusia ($\text{CO}_2$ Biologis):** Hembuskan napas perlahan dari jarak **$15\text{ cm}$** selama $5\text{ detik}$ $\rightarrow$ Amati kenaikan `eco2` ($> 800-1500\text{ ppm}$) dan perubahan AQI ke Level 2–3.
   - **Paparan VOC Sedang (Kapas Alkohol 70% pada jarak $30\text{ cm}$):** Amati kenaikan `tvoc` ke rentang $220-660\text{ ppb}$ $\rightarrow$ `aqi = 4` (`4-Buruk`), Buzzer berbunyi pola peringatan (`alarmAQI_Level4`).
   - **Paparan VOC Tinggi (Kapas Alkohol 70% / Asap Korek pada jarak $10\text{ cm}$):** Amati `tvoc` melonjak $> 2200\text{ ppb}$ $\rightarrow$ `aqi = 5` (`5-Sangat Buruk`), Buzzer membunyikan sirine cepat darurat (`alarmAQI_Level5`).
3. **Uji Waktu Pemulihan Udara (*Recovery Time* dengan Kipas ON vs OFF — $N = 3\times$):**
   - Setelah `aqi` mencapai Level 5, jauhkan sumber alkohol.
   - Ukur pakai *stopwatch* berapa detik waktu yang dibutuhkan hingga `aqi` turun kembali ke Level 1–2 saat **Kipas MATI** dibandingkan saat **Kipas MENYALA**.

---

## 5. Protokol Pengujian Modul M4: Cahaya BH1750 + Relay Lampu (FR-07 & FR-08)

### 5.1 Variabel & Hal yang Harus Diperhatikan
- **Efek Bayangan Tubuh Penguji:** Saat menguji di meja, perhatikan jangan sampai bayangan badan/tanganmu sendiri tidak sengaja menutupi sensor BH1750 saat mencatat kondisi terang.
- **Kompensasi Umpan Balik Optik (*Self-Illumination*):** Karena lampu 5V berada dekat sensor, perhatikan bagaimana sistem mengunci `_luxPatokanSaatNyala` setelah lampu menyala.

### 5.2 Skenario & Jumlah Perulangan
1. **Uji Skala Iluminansi & Halangan Diffuser ($N = 5\times$ per kondisi):**
   - Ukur bacaan `lux` pada 4 kondisi:
     1. Tertutup kotak gelap ($0 - 5\text{ Lux}$ $\rightarrow$ `🌙 Gelap`)
     2. Kamar remang / lampu tidur ($15 - 45\text{ Lux}$ $\rightarrow$ `🌙 Gelap`)
     3. Lampu plafon kamar menyala normal ($80 - 220\text{ Lux}$ $\rightarrow$ `💡 Normal`)
     4. Senter HP dari jarak $20\text{ cm}$ ($> 300\text{ Lux}$ $\rightarrow$ `☀️ Terang`)
   - **Variasi Halangan (Simulasi Tutup Casing):** Bandingkan nilai `lux` saat sensor terbuka vs ditutup plastik mika bening vs ditutup 1 lembar kertas HVS putih (sebagai *diffuser* cahaya).
2. **Uji Matriks 4-Kuadran Kontrol Lampu Adaptif FR-08 ($N = 5\times$ per kuadran):**

| Kuadran | Kondisi Cahaya Awal | Status Radar (`adaOrang`) | Aksi / Perlakuan | Hasil Wajib pada Lampu 5V |
|---|---|---|---|---|
| **Q1** | **Gelap ($< 50\text{ Lux}$)** | **`true` (Ada Orang)** | Masuk ke depan radar saat kamar gelap | Lampu **MENYALA** dalam $\le 1-2\text{ detik}$, dan **TIDAK berkedip mati-nyala** walau sinar lampu mengenai BH1750! |
| **Q2** | **Gelap ($< 50\text{ Lux}$)** | **`false` (Kosong)** | Pergi meninggalkan ruangan | Setelah kosong selama **`10 detik`** (`TIMEOUT_KOSONG_LAMPU_MS`), lampu otomatis **PADAM**. |
| **Q3** | **Terang ($\ge 50\text{ Lux}$)** | **`true` (Ada Orang)** | Nyalakan lampu utama kamar / senter ($+200\text{ Lux}$) | Lampu 5V otomatis **PADAM** (hemat energi karena ruangan sudah terang). |
| **Q4** | **Terang ($\ge 50\text{ Lux}$)** | **`false` (Kosong)** | Ruangan terang & kosong | Lampu 5V tetap **PADAM**. |

---

## 6. Protokol Pengujian Kinerja Sistem & Daya Terpadu (NFR-01 & Stress Test)

1. **Uji Latensi *End-to-End* (SRS NFR-01 — Wajib $N = 10\text{ sampel}$):**
   - Ukur pakai *stopwatch* waktu sejak perubahan fisik diberikan (misal: senter diarahkan ke BH1750, atau tombol `'k'`/`'l'`/`'x'` ditekan di HP) hingga perubahan status tampil di layar HP.
   - **Kriteria Lulus NFR-01:** Minimal **9 dari 10 sampel** memiliki waktu respons **$\le 7,0\text{ detik}$** (pada mode BLE/Serial lokal kita, target aktualnya adalah $1,0 - 2,0\text{ detik}$).
2. **Uji Beban Puncak Serentak (*Full-Load Power Stress Test* — $N = 5\times$):**
   - Aktifkan Kipas (`'k'` $\rightarrow$ `SELALU ON`), aktifkan Lampu (`'l'` $\rightarrow$ `SELALU ON`), dan aktifkan Jam Malam (`'m'` saat ada orang sehingga Buzzer berbunyi) sambil BLE terus mengirim telemetri selama **5 menit penuh**.
   - **Kriteria Lulus:** Tegangan rel 5V (ditopang Elco $1000\mu\text{F}$ + Keramik $100\text{nF}$) tetap stabil; ESP32 **tidak mengalami *brownout reset*** ($0\text{ kali}$ reboot) dan Radar **tidak pernah jatuh ke status `DISCONNECT`**.

---

## 7. Template Tabel Pencatatan Lapangan (Siap Salin ke Excel / Laporan)

```text
+--------+-------------------+-----------------+----------------+----------------+-------------+------------+
| No Uji | Skenario / Jarak  | Halangan / Mode | Bacaan Sensor  | Respon Aktuator| Latensi (s) | Status     |
+--------+-------------------+-----------------+----------------+----------------+-------------+------------+
| 1      | Radar 1.5m Duduk  | Tanpa Halangan  | Diam: 142cm 60%| Lampu: NYALA   | 0.85 s      | LULUS (TP) |
| 2      | Lempar Bantal 1m  | Tanpa Halangan  | VERIF -> KOSONG| Lampu: MATI    | -           | LULUS (TN) |
| 3      | Radar Belakang 0.5| PCB Back-Lobe   | KOSONG (0%)    | Lampu: MATI    | -           | LULUS (TN) |
| 4      | SHT30 Pemanasan   | Hairdryer 30cm  | 31.4 C         | Kipas: NYALA   | 1.80 s      | LULUS      |
| 5      | SHT30 Histeresis  | Dingin Alami    | 30.4 C         | Kipas: NYALA   | -           | LULUS      |
| 6      | SHT30 Histeresis  | Dingin Alami    | 29.9 C         | Kipas: MATI    | 1.95 s      | LULUS      |
| 7      | ENS160 Alkohol    | Jarak 15 cm     | AQI 5, 2450ppb | Sirine Lv.5 ON | 2.10 s      | LULUS      |
| 8      | BH1750 Gelap+Orang| Tanpa Diffuser  | 18.5 Lux       | Lampu: NYALA   | 1.20 s      | LULUS      |
+--------+-------------------+-----------------+----------------+----------------+-------------+------------+
```