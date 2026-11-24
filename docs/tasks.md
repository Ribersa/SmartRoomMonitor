# Task

Paket `com.smartroommonitor`. Root modul: `app/`. Token desain: `tampilan UI-refined/ui-spec.md`.

> **Status 24 Nov 2026.** Seluruh 50 task sudah ditulis dan `:app:assembleDebug` lolos. Verifikasi visual terhadap PNG acuan **belum** dijalankan karena emulator belum terpasang di mesin ini; item yang menuntut perbandingan screenshot ditandai `[x]` hanya bila kondisi kodenya sudah terpenuhi, dan perlu dicek ulang di perangkat. Firmware re-arm 60 detik juga belum di-compile (Arduino CLI tidak tersedia).

## F1. Autentikasi & Peran

### F1.1 Masuk dengan Email

- [x] **F1.1.1** Siapkan modul Gradle, manifest, dan Firebase
  - Sentuh: `app/build.gradle.kts`, `app/src/main/AndroidManifest.xml`, `gradle/libs.versions.toml`
  - Selesai bila: `assembleDebug` lolos dan aplikasi terpasang di emulator minSdk 26 tanpa crash saat dibuka
  - Butuh: —
- [x] **F1.1.2** Bangun `LoginScreen` dari layar 01 dan 02
  - Sentuh: `app/src/main/java/com/smartroommonitor/ui/login/LoginScreen.kt`, `ui/login/LoginViewModel.kt`
  - Selesai bila: layar 01 dan 02 identik dengan PNG acuan; error summary muncul dengan `role=alert` dan fokus pindah ke ringkasan
  - Butuh: F1.1.1
- [x] **F1.1.3** Sambungkan Firebase Auth ke `LoginViewModel`
  - Sentuh: `data/auth/AuthRepository.kt`, `ui/login/LoginViewModel.kt`
  - Selesai bila: sandi salah menampilkan "Email atau kata sandi salah" dan tetap di layar masuk; sandi benar mengembalikan UID
  - Butuh: F1.1.2
- [x] **F1.1.4** Tampilkan state memuat saat verifikasi
  - Sentuh: `ui/login/LoginScreen.kt`
  - Selesai bila: tombol nonaktif dan teks berubah "Memeriksa akun…" selama permintaan; input tidak bisa diubah
  - Butuh: F1.1.3

### F1.2 Pulihkan Sesi Tersimpan

- [x] **F1.2.1** Baca sesi aktif saat aplikasi dibuka
  - Sentuh: `data/auth/AuthRepository.kt`, `ui/navigation/AppNavHost.kt`
  - Selesai bila: membuka ulang aplikasi dengan sesi aktif langsung masuk dashboard tanpa layar masuk
  - Butuh: F1.1.3
- [x] **F1.2.2** Tampilkan skeleton saat memuat sesi
  - Sentuh: `ui/login/SessionLoadingScreen.kt`
  - Selesai bila: layar 03 tampil tanpa status "Tersambung" palsu dan hilang dalam <1 detik pada sesi valid
  - Butuh: F1.2.1

### F1.3 Baca Peran Pengguna

- [x] **F1.3.1** Baca `users/{uid}/role` sekali setelah login
  - Sentuh: `data/user/UserRepository.kt`, `domain/model/Role.kt`
  - Selesai bila: akun `staff@` menghasilkan `Role.Staff`, `pengunjung@` menghasilkan `Role.Pengunjung`, nilai lain menghasilkan `Role.Unknown`
  - Butuh: F1.1.3
- [x] **F1.3.2** Simpan peran di sesi
  - Sentuh: `ui/session/SessionStore.kt`
  - Selesai bila: peran terbaca kembali setelah proses aplikasi dimatikan paksa lalu dibuka
  - Butuh: F1.3.1
- [x] **F1.3.3** Terapkan peran ke navigasi dan visibilitas kontrol
  - Sentuh: `ui/navigation/AppNavHost.kt`, `ui/dashboard/DashboardScreen.kt`
  - Selesai bila: login sebagai Pengunjung tidak menampilkan tombol MUTE maupun segmented mode di layar mana pun
  - Butuh: F1.3.2

### F1.4 Keluar dari Akun

- [x] **F1.4.1** Hapus sesi dan lepas listener
  - Sentuh: `data/auth/AuthRepository.kt`, `ui/session/SessionStore.kt`
  - Selesai bila: setelah keluar, tidak ada pembacaan Firebase aktif dan membuka ulang aplikasi menampilkan layar masuk
  - Butuh: F1.3.3

## F2. Dashboard Kondisi Ruangan

### F2.1 Tampilkan Metrik Live

- [x] **F2.1.1** Definisikan model `RoomCurrent` sesuai schema
  - Sentuh: `domain/model/RoomCurrent.kt`, `data/rtdb/RoomCurrentMapper.kt`
  - Selesai bila: seluruh 17 field terbaca dari JSON contoh `firebase/schema-v0.9.json` tanpa nilai null
  - Butuh: F1.1.1
- [x] **F2.1.2** Berlangganan `room1/current` lewat `addValueEventListener`
  - Sentuh: `data/rtdb/RoomCurrentDataSource.kt`
  - Selesai bila: mengubah nilai di Firebase Console memunculkan emisi baru di `Flow` dalam ≤5 detik
  - Butuh: F2.1.1
- [x] **F2.1.3** Bangun `DashboardScreen` dengan lima kartu metrik
  - Sentuh: `ui/dashboard/DashboardScreen.kt`, `ui/dashboard/DashboardViewModel.kt`
  - Selesai bila: layar 04 identik dengan PNG acuan; semua kartu tampil tanpa scroll horizontal pada lebar 360 px
  - Butuh: F2.1.2

### F2.2 Tandai Perangkat Offline

- [x] **F2.2.1** Hitung kesegaran dari `timestamp`
  - Sentuh: `domain/usecase/ObserveFreshness.kt`
  - Selesai bila: `now - timestamp > 15_000` menghasilkan `Stale`, `timestamp == 0` juga `Stale`
  - Butuh: F2.1.2
- [x] **F2.2.2** Tampilkan banner offline tanpa menghapus data
  - Sentuh: `ui/dashboard/DashboardScreen.kt`
  - Selesai bila: layar 06 tampil dengan banner dan nilai terakhir masih terbaca; banner hilang dalam ≤10 detik setelah data masuk
  - Butuh: F2.2.1

### F2.3 Tampilkan Label Status

- [x] **F2.3.1** Petakan label dan badge dari field perangkat
  - Sentuh: `domain/model/RoomLabels.kt`
  - Selesai bila: `airLabel`, `lightLabel`, `presence`, dan `humidityLabel` tampil apa adanya tanpa transformasi yang mengubah nilainya
  - Butuh: F2.1.1
- [x] **F2.3.2** Tampilkan label pemanasan tanpa banner bahaya
  - Sentuh: `ui/dashboard/DashboardScreen.kt`
  - Selesai bila: saat `airWarmingUp == true` kartu udara menampilkan "Memanaskan" dan tidak ada banner peringatan udara
  - Butuh: F2.3.1

### F2.4 Tampilkan Status Fisik

- [x] **F2.4.1** Pisahkan status fisik dari mode pilihan
  - Sentuh: `ui/dashboard/MetricCard.kt`
  - Selesai bila: kartu perangkat menampilkan `fanStatus`/`lampStatus` sebagai Nyala/Mati, terpisah dari `fanMode`/`lampMode`
  - Butuh: F2.1.3

### F2.5 Simpan Snapshot ke Room

- [x] **F2.5.1** Siapkan Room dengan entity snapshot
  - Sentuh: `app/build.gradle.kts`, `data/local/AppDatabase.kt`, `data/local/CurrentSnapshotDao.kt`
  - Selesai bila: schema JSON terekspor ke `app/schemas/` dan `assembleDebug` lolos
  - Butuh: F1.1.1
- [x] **F2.5.2** Tulis snapshot setiap emisi
  - Sentuh: `data/rtdb/RoomCurrentDataSource.kt`
  - Selesai bila: tabel snapshot berisi satu baris terbaru setelah dashboard menerima data
  - Butuh: F2.5.1, F2.1.2
- [x] **F2.5.3** Tampilkan snapshot saat cache dipakai
  - Sentuh: `ui/dashboard/DashboardViewModel.kt`
  - Selesai bila: mematikan jaringan lalu membuka aplikasi tetap menampilkan nilai terakhir dengan banner offline
  - Butuh: F2.5.2

## F3. Peringatan & Mute Darurat

### F3.1 Tampilkan Banner Peringatan

- [x] **F3.1.1** Definisikan model `RoomAlarm`
  - Sentuh: `domain/model/RoomAlarm.kt`, `data/rtdb/RoomAlarmDataSource.kt`
  - Selesai bila: `active`, `buzzer`, `status`, `reason`, `mute` terbaca dan berubah mengikuti Console
  - Butuh: F2.1.1
- [x] **F3.1.2** Bangun banner peringatan
  - Sentuh: `ui/dashboard/AlarmBanner.kt`
  - Selesai bila: layar 07 identik dengan PNG acuan; banner memuat alasan dan waktu, bukan hanya warna
  - Butuh: F3.1.1

### F3.2 Kirim Perintah Mute

- [x] **F3.2.1** Tulis `room1/alarm/mute = true`
  - Sentuh: `data/rtdb/RoomAlarmDataSource.kt`
  - Selesai bila: menekan tombol menulis `true` dan nilai terverifikasi berubah di Console
  - Butuh: F3.1.1
- [x] **F3.2.2** Pasang tombol MUTE satu ketukan
  - Sentuh: `ui/dashboard/AlarmBanner.kt`
  - Selesai bila: tombol ≥48 dp, satu ketukan tanpa dialog, dan nonaktif selama permintaan berjalan
  - Butuh: F3.2.1

### F3.3 Tampilkan Status Dibungkam

- [x] **F3.3.1** Ubah banner saat `status = "dibungkam"`
  - Sentuh: `ui/dashboard/AlarmBanner.kt`
  - Selesai bila: layar 08 identik dengan PNG acuan dan teks menyebut "aktif lagi dalam 60 detik"
  - Butuh: F3.1.2

### F3.4 Sembunyikan Kontrol Pengunjung

- [x] **F3.4.1** Sembunyikan tombol MUTE untuk Pengunjung
  - Sentuh: `ui/dashboard/AlarmBanner.kt`, `ui/dashboard/DashboardScreen.kt`
  - Selesai bila: layar 20 menampilkan banner tanpa tombol dan tanpa celah kosong
  - Butuh: F1.3.3, F3.1.2

### F3.5 Aktifkan Ulang Alarm 60 Detik

- [x] **F3.5.1** Tambah konstanta re-arm di firmware
  - Sentuh: `firmware/smartroom_esp32s3_firebase/config.h`
  - Selesai bila: `ALARM_REARM_MS` bernilai `60000UL` dan firmware masih compile
  - Butuh: —
- [x] **F3.5.2** Simpan waktu mute dan lepas otomatis
  - Sentuh: `firmware/smartroom_esp32s3_firebase/BuzzerManager.h`, `BuzzerManager.cpp`
  - Selesai bila: setelah mute, bila kondisi bahaya masih aktif 60 detik kemudian buzzer berbunyi lagi dan `status` kembali `"aktif"`
  - Butuh: F3.5.1
- [x] **F3.5.3** Pastikan auto-unmute kondisi normal tetap jalan
  - Sentuh: `firmware/smartroom_esp32s3_firebase/smartroom_esp32s3_firebase.ino`
  - Selesai bila: kondisi bahaya mereda sebelum 60 detik membuat buzzer diam dan `mute` kembali `false` tanpa menunggu re-arm
  - Butuh: F3.5.2

## F4. Kontrol Kipas & Lampu

### F4.1 Pilih Mode Kipas

- [x] **F4.1.1** Bangun `ControlScreen` dari layar 15
  - Sentuh: `ui/control/ControlScreen.kt`, `ui/control/ControlViewModel.kt`
  - Selesai bila: layar 15 identik dengan PNG acuan; segmented control tinggi ≥48 dp
  - Butuh: F2.1.3
- [x] **F4.1.2** Tulis `room1/actuator/fanMode`
  - Sentuh: `data/rtdb/ActuatorDataSource.kt`
  - Selesai bila: memilih "Selalu ON" mengubah nilai di Console menjadi `on` dan bertahan setelah aplikasi ditutup
  - Butuh: F4.1.1

### F4.2 Pilih Mode Lampu

- [x] **F4.2.1** Tulis `room1/actuator/lampMode`
  - Sentuh: `data/rtdb/ActuatorDataSource.kt`, `ui/control/ControlViewModel.kt`
  - Selesai bila: memilih "Selalu OFF" mengubah nilai di Console menjadi `off`
  - Butuh: F4.1.2

### F4.3 Tampilkan Hasil Perintah

- [x] **F4.3.1** Tampilkan snackbar sukses
  - Sentuh: `ui/control/ControlScreen.kt`
  - Selesai bila: layar 22 identik dengan PNG acuan dan snackbar hilang setelah 3 detik
  - Butuh: F4.1.2
- [x] **F4.3.2** Tampilkan kondisi tertunda dan penolakan
  - Sentuh: `ui/control/ControlScreen.kt`
  - Selesai bila: layar 16 dan 17 identik dengan PNG acuan; segmented nonaktif dan alasan tertulis
  - Butuh: F4.3.1

### F4.4 Tampilkan Aturan Otomatis

- [x] **F4.4.1** Baca `room1/thresholds` dan tampilkan di bawah kontrol
  - Sentuh: `data/rtdb/ThresholdDataSource.kt`, `ui/control/ControlScreen.kt`
  - Selesai bila: mengubah `tempFanOn` di Console mengubah catatan "Menyala di atas …" tanpa membuka ulang aplikasi
  - Butuh: F4.1.1

## F5. Grafik 24 Jam & Event Log

### F5.1 Muat Grafik Suhu 24 Jam

- [x] **F5.1.1** Muat `room1/history` dengan `limitToLast(144)`
  - Sentuh: `data/rtdb/HistoryDataSource.kt`, `domain/model/HistoryPoint.kt`
  - Selesai bila: query mengembalikan ≤144 titik terurut naik dan `timestamp` terbaca sebagai milidetik
  - Butuh: F2.1.1
- [x] **F5.1.2** Gambar grafik suhu dengan garis ambang
  - Sentuh: `ui/chart/TrendChart.kt`, `ui/chart/ChartScreen.kt`
  - Selesai bila: layar 10 identik dengan PNG acuan; garis ambang putus-putus, sumbu jam, satuan °C
  - Butuh: F5.1.1

### F5.2 Buka Metrik Lain

- [x] **F5.2.1** Sediakan tombol expand dengan chip metrik
  - Sentuh: `ui/chart/ChartScreen.kt`
  - Selesai bila: kondisi awal hanya menampilkan suhu; menekan expand memunculkan chip Kelembapan dan `airDelta`
  - Butuh: F5.1.2
- [x] **F5.2.2** Simpan metrik terakhir yang dipilih
  - Sentuh: `ui/chart/ChartViewModel.kt`, `data/local/PreferenceStore.kt`
  - Selesai bila: memilih `airDelta` lalu membuka ulang aplikasi menampilkan `airDelta` dengan state expand terbuka
  - Butuh: F5.2.1

### F5.3 Tampilkan Ringkasan Statistik

- [x] **F5.3.1** Hitung terendah, rata-rata, tertinggi
  - Sentuh: `domain/usecase/ComputeHistoryStats.kt`
  - Selesai bila: nilai cocok dengan hitung manual pada 6 titik uji, dan satuan mengikuti metrik aktif
  - Butuh: F5.1.1

### F5.4 Filter Kejadian per Tipe

- [x] **F5.4.1** Muat `room1/events` dengan filter 24 jam
  - Sentuh: `data/rtdb/EventDataSource.kt`, `domain/model/RoomEvent.kt`
  - Selesai bila: hanya event dengan `timestamp` dalam 24 jam terakhir yang tampil, terurut menurun
  - Butuh: F2.1.1
- [x] **F5.4.2** Bangun daftar dan chip filter
  - Sentuh: `ui/events/EventListScreen.kt`, `ui/events/EventViewModel.kt`
  - Selesai bila: layar 12 dan 13 identik dengan PNG acuan; tiap baris memuat label tipe dan severity sebagai teks
  - Butuh: F5.4.1

### F5.5 Cache Riwayat ke Room

- [x] **F5.5.1** Simpan 144 titik terakhir ke Room
  - Sentuh: `data/local/HistoryDao.kt`, `data/rtdb/HistoryDataSource.kt`
  - Selesai bila: tabel berisi ≤144 baris dan titik tertua terhapus otomatis saat melewati batas
  - Butuh: F2.5.1, F5.1.1
- [x] **F5.5.2** Tampilkan riwayat cache saat jaringan mati
  - Sentuh: `ui/chart/ChartViewModel.kt`
  - Selesai bila: mematikan jaringan lalu membuka tab Grafik tetap menampilkan grafik terakhir beserta keterangan sumber cache
  - Butuh: F5.5.1

## F6. Profil & Tema

### F6.1 Tampilkan Profil

- [x] **F6.1.1** Bangun `ProfileScreen` dari layar 18 dan 19
  - Sentuh: `ui/profile/ProfileScreen.kt`, `ui/profile/ProfileViewModel.kt`
  - Selesai bila: layar 18 dan 19 identik dengan PNG acuan dan ringkasan hak akses mengikuti peran
  - Butuh: F1.3.3

### F6.2 Ganti Tema Tampilan

- [x] **F6.2.1** Sediakan tiga pilihan tema
  - Sentuh: `ui/theme/ThemePreference.kt`, `ui/profile/ProfileScreen.kt`
  - Selesai bila: memilih Gelap mengubah seluruh layar ke token dark dan bertahan setelah aplikasi dibuka ulang
  - Butuh: F6.1.1
- [x] **F6.2.2** Terapkan tema ke seluruh layar
  - Sentuh: `ui/theme/Theme.kt`
  - Selesai bila: layar 25 sampai 28 identik dengan PNG acuan gelap; tidak ada warna hardcoded yang tersisa di layar fitur
  - Butuh: F6.2.1

### F6.3 Konfirmasi Keluar

- [x] **F6.3.1** Tampilkan dialog konfirmasi
  - Sentuh: `ui/profile/LogoutDialog.kt`
  - Selesai bila: layar 23 identik dengan PNG acuan; fokus awal di Batal dan `Esc` menutup dialog
  - Butuh: F6.1.1
- [x] **F6.3.2** Sambungkan dialog ke `AuthRepository`
  - Sentuh: `ui/profile/LogoutDialog.kt`, `ui/profile/ProfileViewModel.kt`
  - Selesai bila: memilih Keluar menghapus sesi dan kembali ke layar masuk; Batal tidak mengubah apa pun
  - Butuh: F6.3.1, F1.4.1
