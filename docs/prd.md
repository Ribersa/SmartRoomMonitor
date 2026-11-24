# SmartRoomMonitor

Aplikasi Android untuk pengunjung dan staff ruangan memantau suhu, kelembapan, kualitas udara, cahaya, dan kehadiran secara real-time, menerima peringatan, serta mengatur kipas dan lampu. Data berasal dari ESP32-S3 lewat Firebase Realtime Database. Pengguna tidak perlu datang ke ruangan untuk tahu kondisinya.

## Masalah

- **Siapa**: Pengunjung ruangan (ingin tahu kondisi) dan Staff pengelola (bertanggung jawab menjaga ruangan).
- **Sekarang**: Kondisi dicek manual di tempat, atau baru diketahui setelah ada keluhan.
- **Yang bikin repot**: Tidak ada catatan, penanganan terlambat saat suhu tinggi atau udara buruk, dan kipas/lampu menyala tanpa perlu.

## Alur inti fase 1

Staff membuka aplikasi → login → dashboard menampilkan lima metrik live dalam 5 detik → saat suhu melewati 35 °C muncul banner dan tombol MUTE → Staff menekan MUTE satu kali dan buzzer berhenti dalam 7 detik.

## Tech stack

Kotlin + Jetpack Compose Material 3, MVVM + Clean Architecture, Coroutines/StateFlow, Firebase Auth + Realtime Database SDK, Room untuk cache, minSdk 26, manual DI lewat `AppContainer`. Alasannya: mengikuti kontrak `AGENTS.md` dan 28 layar di `tampilan UI-refined/` sudah punya token siap pakai.

`ponytail:` manual DI, bukan Hilt — pindah ke Hilt kalau layar melewati 12 atau butuh scoping lintas fitur. Room dipakai hanya untuk snapshot terakhir dan 144 titik riwayat, bukan cache penuh.

## Fitur

### F1. Autentikasi & Peran — fase 1 · high

Login email/password dan penentuan peran dari `users/{uid}/role` sebelum dashboard dibuka.

| Sub-fitur | Isi |
|---|---|
| Masuk dengan Email | Form email + sandi, error summary dan inline error, tombol nonaktif saat memuat |
| Pulihkan Sesi Tersimpan | Buka aplikasi dengan sesi aktif langsung ke dashboard tanpa login ulang |
| Baca Peran Pengguna | Ambil `role` sekali setelah login, simpan di sesi, tentukan tampilan Staff atau Pengunjung |
| Keluar dari Akun | Hapus sesi, lepas listener, kembali ke layar masuk |

### F2. Dashboard Kondisi Ruangan — fase 1 · high

Satu layar berisi lima metrik live dari `room1/current` beserta status kesegaran data.

| Sub-fitur | Isi |
|---|---|
| Tampilkan Metrik Live | Suhu, kelembapan, `airDelta`, lux, dan kehadiran lewat `addValueEventListener` |
| Tandai Perangkat Offline | Banner "Perangkat offline" bila `now - timestamp > 15.000 ms`, data terakhir tetap tampil |
| Tampilkan Label Status | Label Memanaskan/Baik/Sedang/Buruk, Gelap/Terang, tidak ada/bergerak/diam, plus badge batas |
| Tampilkan Status Fisik | `fanStatus` dan `lampStatus` dibedakan dari mode pilihan Staff |
| Simpan Snapshot ke Room | Data terakhir tetap tampil setelah aplikasi ditutup, termasuk saat offline |

### F3. Peringatan & Mute Darurat — fase 1 · high

Banner peringatan dari `room1/alarm` dan tombol MUTE untuk Staff.

| Sub-fitur | Isi |
|---|---|
| Tampilkan Banner Peringatan | Banner merah dengan alasan dan waktu saat `alarm.active` benar |
| Kirim Perintah Mute | Tulis `room1/alarm/mute = true`, tombol ≥48 dp, satu ketukan |
| Tampilkan Status Dibungkam | Banner "Alarm dibungkam" dengan keterangan aktif lagi dalam 60 detik |
| Sembunyikan Kontrol Pengunjung | Pengunjung melihat banner tanpa tombol MUTE |
| Aktifkan Ulang Alarm 60 Detik | Firmware membunyikan lagi bila kondisi bahaya belum mereda setelah 60 detik dibungkam |

### F4. Kontrol Kipas & Lampu — fase 2 · high

Staff memilih mode kipas dan lampu, status fisik dikembalikan perangkat.

| Sub-fitur | Isi |
|---|---|
| Pilih Mode Kipas | Segmented Otomatis/Selalu ON/Selalu OFF ke `room1/actuator/fanMode` |
| Pilih Mode Lampu | Segmented yang sama ke `room1/actuator/lampMode` |
| Tampilkan Hasil Perintah | Snackbar sukses, pesan tertunda saat offline, penolakan saat Pengunjung |
| Tampilkan Aturan Otomatis | Catatan ambang aktif dari `room1/thresholds` di bawah tiap kontrol |

### F5. Grafik 24 Jam & Event Log — fase 2 · medium

Tren dari `room1/history` dan daftar kejadian dari `room1/events`.

| Sub-fitur | Isi |
|---|---|
| Muat Grafik Suhu 24 Jam | Default hanya suhu, `limitToLast(144)`, garis ambang putus-putus, sumbu jam |
| Buka Metrik Lain | Tombol expand memunculkan chip Kelembapan dan `airDelta`, pilihan terakhir disimpan |
| Tampilkan Ringkasan Statistik | Terendah, rata-rata, tertinggi untuk metrik yang sedang aktif |
| Filter Kejadian per Tipe | Chip Semua/Suhu/Udara/Jam malam atas `temp_high`, `air_bad`, `night_presence` |
| Cache Riwayat ke Room | 144 titik terakhir tersedia offline setelah aplikasi ditutup |

### F6. Profil & Tema — fase 3 · low

Identitas akun, ringkasan hak akses, dan pilihan tema.

| Sub-fitur | Isi |
|---|---|
| Tampilkan Profil | Nama, email, peran, dan ringkasan hak akses |
| Ganti Tema Tampilan | Terang / Gelap / Ikut sistem, tersimpan lokal |
| Konfirmasi Keluar | Dialog dua tombol sebelum sesi dihapus |

## Di luar scope

- Multi-room, web dashboard, OTA, export PDF, kamera, dan FCM — di luar MVP sesuai SRS v1.0 Bab 6.
- Mengubah `room1/thresholds` dari aplikasi — hanya lewat Firebase Console, dan rules menolak.
- Menulis `room1/current`, `room1/alarm/{active,buzzer,status,reason}`, `room1/history`, `room1/events` — milik ESP32 (`Device`).
- Push notification: peringatan hanya muncul saat aplikasi terbuka.
- BLE dan firmware uji `firmware/tests/` — tidak disentuh.

## Asumsi

- `Asumsi:` paket `com.smartroommonitor`, label launcher "SmartRoomMonitor".
- `Asumsi:` satu proyek Firebase dengan firmware; akun `staff@` dan `pengunjung@` sudah dibuat lewat Console dan `role` sudah diisi.
- `Asumsi:` `google-services.json` disediakan pengguna secara lokal dan tidak di-commit (NFR-04).
- `Asumsi:` re-arm 60 detik disetujui dan dikerjakan di firmware, bukan dihitung aplikasi.
- `Asumsi:` dua keputusan terbuka lain di SRS Lampiran B (kehadiran sebagai syarat kipas, status offline per sensor) tidak diubah di fase mana pun.
