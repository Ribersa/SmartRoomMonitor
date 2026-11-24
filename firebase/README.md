# Konfigurasi Backend Firebase — SmartRoomMonitor (SRS v0.9)

Direktori ini berisi kontrak skema JSON resmi (`schema-v0.9.json`) dan aturan keamanan Firebase Realtime Database (`database.rules.json`) untuk memenuhi **FR-B01, FR-B02, FR-01 s.d. FR-08, NFR-03, NFR-04, dan NFR-05**.

---

## Panduan Refinement Proyek Firebase dari Tahap Uji `06` ke Produksi

Ikuti langkah berikut pada proyek Firebase Console yang sebelumnya digunakan saat pengujian `06_uji_telemetri_firebase_rtdb`:

### 1. Aktifkan Firebase Authentication (Email/Password)
1. Buka **Firebase Console** → pilih proyek SmartRoomMonitor Anda.
2. Masuk ke menu **Build → Authentication → Get Started** (jika belum aktif).
3. Pada tab **Sign-in method**, aktifkan provider **Email/Password** (biarkan *Email link* nonaktif).
4. Buka tab **Users** dan tambahkan minimal **3 akun**:
   - **Akun Perangkat ESP32:** mis. `device@smartroom.local` (gunakan password kuat, simpan untuk `secrets.h` lokal).
   - **Akun Staff:** mis. `staff@smartroom.local` (untuk aplikasi Android peran *Staff*).
   - **Akun Pengunjung:** mis. `visitor@smartroom.local` (untuk aplikasi Android peran *Pengunjung*).
5. Catat **User UID** dari ketiga akun tersebut.

### 2. Impor & Bersihkan Struktur RTDB ke Skema v0.9
1. Masuk ke **Build → Realtime Database → Data**.
2. Hapus node uji coba lama dari tahap `06` (jika ada field lawas yang tidak sesuai kontrak v0.9).
3. Klik ikon titik tiga (`⋮`) di kanan atas pohon data → **Import JSON** → pilih file [`schema-v0.9.json`](schema-v0.9.json).
4. Buka node `/users`, lalu ganti ketiga kunci placeholder (`UID_STAFF_GANTI_DARI_AUTH`, `UID_VISITOR_GANTI_DARI_AUTH`, `UID_DEVICE_GANTI_DARI_AUTH`) dengan **User UID** asli dari Langkah 1:
   - UID akun perangkat → `"role": "device"`
   - UID akun staff → `"role": "staff"`
   - UID akun pengunjung → `"role": "visitor"`

### 3. Terapkan Security Rules Produksi (NFR-03)
1. Pindah ke tab **Realtime Database → Rules**.
2. Ganti seluruh isi aturan *test mode* lama dengan isi file [`database.rules.json`](database.rules.json).
3. Klik **Publish**.

### 4. Ambil Web API Key & URL Database untuk ESP32 (`secrets.h`)
1. Masuk ke **Project Settings** (ikon gerigi kiri atas) → tab **General** → salin **Web API Key** (jika belum muncul, tambahkan satu *Web App* dummy atau ambil dari *Android App* `google-services.json` bagian `api_key.current_key`).
2. Salin URL Realtime Database (tanpa garis miring di akhir), contoh:
   `https://nama-proyek-default-rtdb.asia-southeast1.firebasedatabase.app`
3. Di folder firmware `firmware/smartroom_esp32s3_firebase/`, salin `secrets.h.example` menjadi `secrets.h` dan isi kredensial Anda secara lokal. File `secrets.h` otomatis diabaikan oleh `.gitignore` (**NFR-04**).

---

## Ringkasan Matriks Hak Akses (`database.rules.json`)

| Path RTDB | Tanpa Login | Pengunjung (`visitor`) | Staff (`staff`) | Perangkat (`device`) | Firebase Console (Admin) |
|---|---|---|---|---|---|
| `/users/$uid` | Ditolak | Baca milik sendiri | Baca milik sendiri | Baca milik sendiri | Baca & Tulis |
| `/room1/telemetry` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
| `/room1/controls/fan_mode`, `lamp_mode` | Ditolak | Baca | Baca & Tulis | Baca | Baca & Tulis |
| `/room1/controls/alarm_muted` | Ditolak | Baca | Baca & Tulis | Baca & Tulis | Baca & Tulis |
| `/room1/controls/fan_status`, `lamp_status` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
| `/room1/thresholds` | Ditolak | Baca | Baca | Baca | Baca & Tulis |
| `/room1/history` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
| `/room1/events` | Ditolak | Baca | Baca | Baca & Tulis | Baca & Tulis |
