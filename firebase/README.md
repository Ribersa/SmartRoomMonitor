# Konfigurasi Backend Firebase — SmartRoomMonitor (SRS v1.0 & Sinkron UML)

Direktori ini berisi kontrak skema JSON resmi (`schema-v0.9.json`) dan aturan keamanan Firebase Realtime Database (`database.rules.json`) yang diselaraskan penuh dengan **dokumen Pemodelan UML SmartRoomMonitor** sekaligus mematuhi **SRS v1.0** (**FR-B01, FR-B02, FR-01 s.d. FR-08, NFR-03, NFR-04, NFR-05, NFR-06**).

---

## Catatan Penyelarasan UML vs SRS v1.0

- **Path & Nama Field Mengikuti UML:** `users/{uid}/role` (`"Staff"` / `"Pengunjung"` / `"Device"`), `room1/current`, `room1/actuator/{fanMode, lampMode}`, `room1/alarm/{active, buzzer, status, reason, mute}`, `room1/history/{epochSec}` (interval 10 menit = 144 titik per 24 jam untuk `limitToLast(144)`), dan `room1/events`.
- **Tanggung Jawab Penulisan Aman (Kepatuhan FR-06 & NFR-03):** Penulisan `room1/events` dan pengaktifan `room1/alarm/{active, buzzer, status, reason}` dikerjakan secara otonom oleh **ESP32-S3 (`Device`)**, bukan oleh Aplikasi Android. Ini menjamin:
  1. Buzzer tetap berbunyi lokal walau koneksi internet putus (**FR-06**).
  2. Peran `Pengunjung` 100% *read-only* tanpa error `permission_denied` (**NFR-03**).
  3. Tidak terjadi duplikasi log kejadian saat banyak klien Android membuka dashboard bersamaan.
- **Satuan Timestamp:** Field `timestamp` menggunakan **Unix Epoch dalam milidetik (13 digit)** agar langsung kompatibel dengan `System.currentTimeMillis() - snapshot.timestamp > 15_000` di Android (UC-12 / FR-B02).

---

## Panduan Refinement Proyek Firebase dari Tahap Uji `06` ke Produksi

### 1. Aktifkan Firebase Authentication (Email/Password)
1. Buka **Firebase Console** → pilih proyek SmartRoomMonitor Anda.
2. Masuk ke menu **Build → Authentication → Get Started** (jika belum aktif).
3. Pada tab **Sign-in method**, aktifkan provider **Email/Password**.
4. Pada tab **Users**, buat minimal **3 akun**:
   - **Akun Perangkat ESP32 (`Device`):** mis. `device@smartroom.local`
   - **Akun Staff (`Staff`):** mis. `staff@smartroom.local`
   - **Akun Pengunjung (`Pengunjung`):** mis. `pengunjung@smartroom.local`
5. Catat **User UID** dari ketiga akun tersebut.

### 2. Impor Struktur RTDB v0.9
1. Masuk ke **Build → Realtime Database → Data**.
2. Hapus node uji lama dari tahap `06` agar struktur bersih.
3. Klik ikon titik tiga (`⋮`) → **Import JSON** → pilih [`schema-v0.9.json`](schema-v0.9.json).
4. Buka node `/users`, lalu ganti ketiga kunci placeholder dengan **User UID** asli dari Langkah 1 (perhatikan huruf kapital pada `role`):
   - UID akun perangkat → `"role": "Device"`
   - UID akun staff → `"role": "Staff"`
   - UID akun pengunjung → `"role": "Pengunjung"`

### 3. Terapkan Security Rules Produksi (NFR-03)
1. Pindah ke tab **Realtime Database → Rules**.
2. Ganti seluruh isi aturan *test mode* lama dengan isi file [`database.rules.json`](database.rules.json).
3. Klik **Publish**.

### 4. Kredensial Lokal ESP32 (`secrets.h`)
1. Ambil **Web API Key** di **Project Settings → General** dan URL RTDB (tanpa garis miring akhir).
2. Di folder firmware `firmware/smartroom_esp32s3_firebase/`, salin `secrets.h.example` menjadi `secrets.h` dan isi kredensial Anda secara lokal (`secrets.h` otomatis diabaikan oleh `.gitignore` sesuai **NFR-04**).

---

## Ringkasan Matriks Hak Akses (`database.rules.json`)

| Path RTDB | Tanpa Login | Pengunjung (`Pengunjung`) | Staff (`Staff`) | Perangkat (`Device`) | Firebase Console (Admin) |
|---|---|---|---|---|---|
| `/users/$uid` | Ditolak | Baca milik sendiri | Baca milik sendiri | Baca milik sendiri | Baca & Tulis |
| `/room1/current` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
| `/room1/actuator/fanMode`, `lampMode` | Ditolak | Baca | Baca & Tulis | Baca | Baca & Tulis |
| `/room1/alarm/mute` | Ditolak | Baca | Baca & Tulis | Baca & Tulis | Baca & Tulis |
| `/room1/alarm/active`, `buzzer`, `status`, `reason` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
| `/room1/thresholds` | Ditolak | Baca | Baca | Baca | Baca & Tulis |
| `/room1/history` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
| `/room1/events` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
