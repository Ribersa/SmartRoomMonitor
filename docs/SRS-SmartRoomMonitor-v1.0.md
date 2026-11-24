# SOFTWARE REQUIREMENTS SPECIFICATION
## SmartRoomMonitor
### Aplikasi Android untuk Memantau dan Mengatur Kondisi Ruangan

| Informasi | Keterangan |
|---|---|
| Mata kuliah | Rekayasa Perangkat Lunak (RPL), M4/M5 Analisis dan Dokumentasi Kebutuhan |
| Versi / tanggal | SRS v1.0, 24 November 2026 |
| Menggantikan | SRS v0.9, 21 Oktober 2026 |
| Kontrak data | `firebase/schema-v0.9.json` dan `firebase/database.rules.json` |
| Pemodelan | Dokumen Pemodelan UML SmartRoomMonitor |
| Tim | Deswiryawan Saragih (241111008), Akhmad Zakariya (241111010), Fattih Kuwaka Bekti (241111005), Ahmad Syakir L. (241111026) |

---

### Riwayat Versi

| Versi | Tanggal | Perubahan | Disetujui |
|---|---|---|---|
| v0.1 - v0.6 | 7 - 8 Okt 2026 | Baseline kebutuhan, pembagian FR/NFR, latar belakang, profil pengguna, elisitasi, dan batasan. | Deswiryawan |
| v0.7.1 - v0.7.3 | 14 Okt 2026 | Penyederhanaan lingkup, penambahan bab interaksi pengguna, kebutuhan ditulis dari sudut pandang pengguna. | Digantikan |
| v0.9 | 21 Okt 2026 | Pembaruan kebutuhan fungsional dan non-fungsional, penyesuaian domain modul, penetapan fitur tingkat cahaya pada M4, dan penataan urutan nomor FR/NFR. | Digantikan |
| v1.0 | 24 Nov 2026 | Penyelarasan penuh dengan kontrak Firebase, security rules, firmware ESP32-S3, dan pemodelan UML: peran `Staff`/`Pengunjung`/`Device`, path `room1/current`, `room1/actuator`, `room1/alarm`, `room1/thresholds`, `room1/history`, `room1/events`; `timestamp` dalam milidetik; label udara berbasis `airDelta` terhadap baseline; label cahaya dua tingkat; riwayat 10 menit; tanggung jawab penulisan per path; penambahan Bab 7 kontrak data dan Lampiran B keputusan terbuka. | Menunggu persetujuan |

---

## 1. Latar Belakang

### 1.1 Latar Belakang
Kondisi ruangan seperti suhu, kelembapan, kualitas udara, cahaya, dan keberadaan orang umumnya dipantau secara manual: dicek langsung di tempat, atau baru diketahui setelah ada keluhan. Cara ini tidak berjalan terus-menerus, tidak meninggalkan catatan, dan membuat penanganan terlambat ketika suhu terlalu tinggi, udara memburuk, ada orang di luar jam semestinya, atau lampu dan kipas menyala tanpa perlu.

SmartRoomMonitor adalah aplikasi Android yang membantu pengguna melihat kondisi ruangan dari mana saja, menerima peringatan saat ada yang tidak normal, dan mengatur kipas serta lampu. Data kondisi ruangan dikirim oleh sebuah perangkat sensor di ruangan melalui layanan Firebase. Perangkat sensor menjadi sumber data, pelaksana perintah, sekaligus penulis tunggal data kondisi, status alarm, riwayat, dan catatan kejadian; seluruh interaksi pengguna berjalan di aplikasi.

Aplikasi dipakai oleh dua peran. Pengunjung hanya melihat kondisi ruangan, grafik, dan daftar kejadian. Staff selain melihat juga mengatur kipas dan lampu serta membungkam alarm. Batas peringatan tidak diubah dari aplikasi, melainkan lewat Firebase Console oleh pengelola ruangan.

### 1.2 Tujuan
Memberi pengguna satu aplikasi yang mudah dipakai untuk: (1) melihat kondisi ruangan saat ini, (2) mendapat peringatan dan membungkamnya dengan cepat, (3) mengatur kipas dan lampu, serta (4) melihat tren dan riwayat kejadian.

---

## 2. Profil Pengguna dan Aktor

Aplikasi dipakai oleh dua peran. Pengunjung hanya membaca data, sedangkan Staff dapat mengendalikan perangkat dan membungkam alarm.

| Aspek | Pengunjung | Staff |
|---|---|---|
| Siapa | Orang yang ingin tahu kondisi ruangan tanpa ikut mengatur, misalnya tamu, mahasiswa, atau penghuni. | Orang yang bertanggung jawab menjaga ruangan, misalnya operator ruang atau pengelola. |
| Kemampuan teknis | Rendah; tidak perlu paham IoT atau Firebase. | Menengah; paham arti batas (threshold) dan cara mengubahnya lewat Firebase Console. |
| Perangkat | Smartphone Android (minSdk 26). | Smartphone Android (minSdk 26) dan Firebase Console. |
| Tujuan utama | Tahu kondisi ruangan saat ini dan riwayat kejadian. | Menjaga ruangan tetap nyaman dan aman, serta menyesuaikan batas peringatan. |
| Yang dapat dilakukan | Login, melihat dashboard, grafik, dan event log. | Semua milik Pengunjung, ditambah kontrol kipas dan lampu serta mute alarm. |
| Batas akses | Hanya membaca; tidak dapat mengubah apa pun. | Dapat mengubah mode kipas dan lampu serta membungkam alarm. Batas peringatan hanya diubah lewat Firebase Console, di luar aplikasi. |

Aktor pendukung:

* **Perangkat sensor (`Device`, ESP32-S3).** Sumber data kondisi ruangan, pelaksana perintah kipas, lampu, dan buzzer, serta penulis tunggal `room1/current`, `room1/alarm/{active, buzzer, status, reason}`, `room1/history`, dan `room1/events`.
* **Firebase.** Firebase Authentication untuk verifikasi identitas dan Firebase Realtime Database beserta security rules untuk penyimpanan data dan pembatasan akses.

---

## 3. Kebutuhan Fungsional

Bab ini memuat 8 kebutuhan fungsional (FR). Setiap anggota memegang dua FR berpasangan secara rapi dan runtut (M1 Syakir, M2 Zakariya, M3 Deswiryawan, M4 Fattih). Kolom Aktor menunjuk Bab 2, kolom PIC menunjuk modul pada pembagian tugas. Kolom Sumber diisi dari Bab 5 setelah observasi selesai, dan sementara ditandai [OBS-xx].

| ID | Nama | Aktor | Deskripsi kebutuhan | Sumber | PIC |
|---|---|---|---|---|---|
| FR-01 | Suhu dan peringatan suhu tinggi | Pengunjung, Staff | Pengguna melihat suhu ruangan beserta label kelembapan (Kering/Nyaman/Lembap). Jika suhu melebihi batas `tempAlert` (default 35,0 °C), pengguna mendapat banner/badge, perangkat membunyikan buzzer lokal, dan kejadian `temp_high` dicatat. Peringatan yang sama tidak diulang dalam 60 detik. | [OBS-xx] | M1 Syakir |
| FR-02 | Kontrol kipas | Staff | Staff memilih mode kipas Otomatis / Selalu ON / Selalu OFF. Pada Otomatis, kipas menyala saat suhu di atas `tempFanOn` (default 31,0 °C) dan mati setelah turun sebesar `tempFanHysteresis` (default 1,0 °C, yaitu pada 30,0 °C atau kurang). Status fisik kipas tampil di dashboard. | [OBS-xx] | M1 Syakir |
| FR-03 | Kualitas udara dan peringatan | Pengunjung, Staff | Pengguna melihat label udara: Memanaskan, Baik, Sedang, atau Buruk (bukan angka mentah), beserta nilai `airDelta`. Penilaian memakai kenaikan TVOC terhadap baseline normal, bukan angka absolut. Selama 3 menit pertama (masa pemanasan) label Memanaskan dan tidak ada peringatan. Setelah itu Buruk bila `airDelta` mencapai 160 atau AQI mencapai 4, dan Sedang bila `airDelta` mencapai 80 atau AQI 3. Saat Buruk, pengguna mendapat banner/badge, perangkat berbunyi, dan kejadian `air_bad` dicatat. | [OBS-xx] | M2 Zakariya |
| FR-04 | Grafik 24 jam dan event log | Pengunjung, Staff | Pengguna melihat grafik 24 jam untuk suhu, kelembapan, dan kualitas udara beserta nilai terendah, tertinggi, dan rata-rata, serta daftar kejadian (kehadiran, suhu tinggi, udara buruk) dengan waktu dan warna per tingkat keparahan, dapat difilter 24 jam dan per tipe. | [OBS-xx] | M2 Zakariya |
| FR-05 | Kehadiran dan peringatan jam malam | Pengunjung, Staff | Pengguna melihat status kehadiran (tidak ada / bergerak / diam). Jika ada orang pada pukul 22:00-06:00 WIB, pengguna mendapat banner/badge, perangkat berbunyi, dan kejadian `night_presence` dicatat. | [OBS-xx] | M3 Deswiryawan |
| FR-06 | Bunyi peringatan dan mute darurat | Staff | Perangkat berbunyi saat ada peringatan dan tetap berbunyi walau koneksi internet terputus. Saat alarm aktif, muncul banner dan tombol MUTE besar; Staff dapat membungkam alarm maksimal 2 tap, dan bunyi berhenti dalam 7 detik atau kurang. Selama alarm dibungkam, status ditampilkan sebagai "dibungkam" dan bunyi tidak berulang sampai seluruh kondisi bahaya mereda. | [OBS-xx] | M3 Deswiryawan |
| FR-07 | Tingkat cahaya dan label | Pengunjung, Staff | Pengguna melihat tingkat cahaya ruangan dalam satuan lux secara real-time beserta label Gelap (di bawah 40 lux) atau Terang (40 lux ke atas). | [OBS-xx] | M4 Fattih |
| FR-08 | Kontrol lampu | Staff | Staff melihat tingkat cahaya dan memilih mode lampu Otomatis / Selalu ON / Selalu OFF. Pada Otomatis, lampu menyala saat ruangan gelap dan ada orang, lalu padam saat cahaya luar naik 200 lux di atas cahaya lampu sendiri atau saat ruangan kosong selama 10 detik. Status fisik lampu tampil di dashboard. | [OBS-xx] | M4 Fattih |

### 3.1 Kebutuhan Bersama
Dua kebutuhan berikut dikerjakan bersama dan tidak dihitung sebagai bagian dari 8 FR di atas.

| ID | Nama | Aktor | Deskripsi kebutuhan | Sumber | PIC |
|---|---|---|---|---|---|
| FR-B01 | Autentikasi dan peran | Pengunjung, Staff | Pengguna dapat login dan logout dengan email + password; sesi tetap tersimpan. Akses tanpa login ditolak. Peran Pengunjung dan Staff dibedakan dari `users/{uid}/role`, dan aplikasi menyesuaikan tampilan kontrol berdasarkan peran tersebut. | [OBS-xx] | Bersama |
| FR-B02 | Dashboard dan status offline | Pengunjung, Staff | Pengguna melihat kondisi ruangan pada satu layar, diperbarui tiap 5 detik atau lebih cepat. Jika data berhenti masuk lebih dari 15 detik, muncul banner "Perangkat offline" dan data terakhir tetap tampil. Banner hilang saat data kembali masuk. | [OBS-xx] | Bersama |

---

## 4. Kebutuhan Non-Fungsional

Bab ini memuat 8 kebutuhan non-fungsional (NFR). Setiap anggota memegang dua NFR, berpasangan langsung dengan domain modul miliknya.

| ID | Kategori | Deskripsi kebutuhan | Verifikasi | PIC |
|---|---|---|---|---|
| NFR-01 | Kinerja | Perubahan kondisi ruangan tampil di aplikasi dalam 7 detik atau kurang; cold start kurang dari 3 detik. | 10 sampel via hotspot; lulus bila 9/10 dalam 7 detik atau kurang. | M1 Syakir |
| NFR-02 | Keandalan | Sistem tersambung kembali otomatis setelah koneksi putus; banner offline muncul saat data berhenti, dan data terakhir tetap tampil. | 15 menit tanpa putus; banner muncul dalam 20 detik dan hilang dalam 10 detik setelah data masuk. | M1 Syakir |
| NFR-03 | Keamanan akses dan peran | Data hanya dapat dibaca pengguna yang login. Pengunjung tidak dapat mengubah data apa pun. Hanya Staff yang dapat mengubah mode kipas/lampu dan membungkam alarm. Aplikasi tidak pernah menulis data kondisi, status alarm, riwayat, maupun catatan kejadian. Batas peringatan hanya diubah lewat Firebase Console. | File rules + screenshot akses ditolak. | M2 Zakariya |
| NFR-04 | Keamanan data dan kredensial | Kunci rahasia tidak masuk repositori; akses data diatur lewat aturan (rules) Firebase. | Pemeriksaan repositori tanpa kunci rahasia + file rules. | M2 Zakariya |
| NFR-05 | Keterpeliharaan | Struktur modul dipisah per fitur; nama field data disepakati sebelum coding agar tidak bentrok, dan setiap path hanya memiliki satu penulis. | Tinjauan struktur modul, daftar nama field, dan tabel penulis pada Bab 7. | M3 Deswiryawan |
| NFR-06 | Akurasi data | Penilaian udara berbasis kenaikan dari kondisi normal (baseline), bukan angka absolut; tidak ada peringatan dari pembacaan sensor yang belum stabil selama masa pemanasan 3 menit. | Uji masa pemanasan 3 menit tanpa peringatan. | M3 Deswiryawan |
| NFR-07 | Usabilitas | Satu layar utama; mute maksimal 2 tap; bahasa Indonesia; status berupa label yang mudah dipahami; tombol minimal 48 dp. | Checklist 5 item + uji 2 pengguna non-teknis; screenshot. | M4 Fattih |
| NFR-08 | Kompatibilitas | Android minSdk 26+, Kotlin + Compose, Firebase. | assembleDebug lolos di 2 HP. | M4 Fattih |

---

## 5. Hasil Elisitasi

Elisitasi belum dilakukan secara langsung. Tabel di bawah disediakan sebagai template yang diisi setelah observasi dan pengujian selesai. Kolom Sumber pada Bab 3 dan 4 baru dapat diisi setelah bagian ini lengkap.

### 5.1 Rencana Observasi
| Item | Isi |
|---|---|
| Teknik | (isi: observasi langsung / wawancara / uji coba skenario) |
| Objek amati | (isi) |
| Tanggal | (isi) |
| Lokasi | (isi) |
| Pengamat | (isi) |
| Alat bantu | (isi: catatan, foto, rekaman) |

### 5.2 Temuan
| ID | Aspek | Temuan | Implikasi kebutuhan |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

### 5.3 Diskusi Kebutuhan
Usulan dari diskusi tim, diposisikan sebagai client. Statusnya usulan dan belum divalidasi ke pengguna nyata.

| ID | Aspek | Kebutuhan yang diusulkan | Implikasi kebutuhan |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

Catatan: isi kolom Sumber pada Bab 3 dan 4 dengan ID temuan dari 5.2 dan 5.3 setelah observasi selesai. Sebelum terisi, kolom Sumber ditandai [OBS-xx].

---

## 6. Batasan Sistem

Batasan berikut membatasi ruang lingkup software dan kebutuhan pada Bab 3 dan 4. Perangkat keras hanya disebut sebagai sumber data dan pelaksana perintah; detailnya dibahas di dokumen terpisah.

| Kategori | Batasan |
|---|---|
| Lingkup | Satu ruangan (room1). Multi-room, web dashboard, OTA, export PDF, kamera, humidifier/dehumidifier, dan push notification di luar MVP. Kamera tidak dipakai karena beban pemrosesan dan pertimbangan privasi. |
| Hak akses | Pengunjung hanya membaca. Staff dapat mengubah mode kipas/lampu dan membungkam alarm. Ubah batas dilakukan lewat Firebase Console, di luar aplikasi. |
| Perangkat sensor | Satu perangkat ESP32-S3 sebagai sumber data (suhu, kelembapan, udara, cahaya, kehadiran), pelaksana perintah kipas, lampu, dan buzzer, serta penulis tunggal data kondisi, status alarm, riwayat, dan catatan kejadian. Detail rangkaian di dokumen terpisah. Nilai udara bersifat perkiraan, bukan alat ukur resmi. |
| Beban kipas dan lampu | Diasumsikan berskala kecil (mis. LED atau kipas DC lewat MOSFET/relay), tidak langsung ke listrik AC. |
| Privasi | Sistem hanya menyimpan data numerik dan status; tidak merekam gambar maupun suara. |
| Jaringan | Bergantung pada hotspot HP dan internet. Tanpa koneksi, data tidak masuk ke aplikasi; perangkat tetap menjalankan otomatisasi dan bunyi peringatan secara lokal. |
| Pembaruan data | Perangkat mengirim data kondisi tiap 3 detik; aplikasi menampilkan paling lambat 5 detik setelah perubahan; perintah dari aplikasi dapat tertunda hingga 7 detik. |
| Riwayat | Satu titik per 10 menit, maksimal 144 titik per 24 jam; titik lebih lama dari 24 jam dihapus perangkat. |
| Platform | Hanya Android minSdk 26+; antarmuka berbahasa Indonesia; login hanya email + password. |
| Peringatan | Muncul sebagai banner/badge saat aplikasi terbuka, plus bunyi di perangkat. Semua batas masih tentatif dan dapat diubah lewat Firebase Console. Sistem bukan alarm kebakaran/gas resmi. |
| Backend | Menggunakan Firebase (Realtime Database dan Auth) sesuai batas paket yang dipakai. |
| Proyek | Tugas mata kuliah RPL M4/M5, dikerjakan tim 4 orang dengan tenggat dari mata kuliah. |

---

## 7. Kontrak Data dan Tanggung Jawab Penulisan

Bab ini menjadi rujukan tunggal untuk nama field, tipe, satuan, dan penulis data. Bila terjadi perbedaan dengan dokumen lain, Bab 7 dan berkas `firebase/schema-v0.9.json` yang berlaku.

### 7.1 Path dan Field

| Path | Field | Tipe | Satuan / nilai | Penulis |
|---|---|---|---|---|
| `users/{uid}` | `role`, `name` | String | `Staff` / `Pengunjung` / `Device` | Firebase Console |
| `room1/current` | `temperature` | Float | °C | Device |
| `room1/current` | `humidity` | Float | %RH | Device |
| `room1/current` | `humidityLabel` | String | `Kering` / `Nyaman` / `Lembap` | Device |
| `room1/current` | `airDelta` | Integer | selisih TVOC terhadap baseline (ppb) | Device |
| `room1/current` | `airLabel` | String | `Memanaskan` / `Baik` / `Sedang` / `Buruk` | Device |
| `room1/current` | `airAqi`, `airTvoc`, `airEco2` | Integer | AQI 1-5, ppb, ppm | Device |
| `room1/current` | `airWarmingUp` | Boolean | `true` selama 3 menit pertama | Device |
| `room1/current` | `lightLux` | Float | lux | Device |
| `room1/current` | `lightLabel` | String | `Gelap` / `Terang` | Device |
| `room1/current` | `presence` | String | `tidak ada` / `bergerak` / `diam` | Device |
| `room1/current` | `presenceDetected` | Boolean | `true` / `false` | Device |
| `room1/current` | `fanStatus`, `lampStatus` | Boolean | status fisik aktuator | Device |
| `room1/current` | `nightModeActive` | Boolean | `true` pada 22:00-06:00 WIB | Device |
| `room1/current` | `timestamp` | Long | Unix epoch milidetik (13 digit) | Device |
| `room1/actuator` | `fanMode`, `lampMode` | String | `auto` / `on` / `off` | Staff |
| `room1/alarm` | `active`, `buzzer` | Boolean | status alarm dan bunyi | Device |
| `room1/alarm` | `status` | String | `normal` / `aktif` / `dibungkam` | Device |
| `room1/alarm` | `reason` | String | `none` / `temp_high` / `air_bad` / `night_presence` | Device |
| `room1/alarm` | `mute` | Boolean | perintah mute | Staff, direset Device |
| `room1/thresholds` | `tempFanOn`, `tempFanHysteresis`, `tempAlert`, `lightDarkLux`, `lightExternalHysteresisLux` | Float | °C dan lux | Firebase Console |
| `room1/thresholds` | `airDeltaModerate`, `airDeltaBad` | Integer | ppb | Firebase Console |
| `room1/thresholds` | `lampEmptyTimeoutSec` | Integer | detik | Firebase Console |
| `room1/thresholds` | `nightStartHour`, `nightEndHour` | Integer | jam WIB | Firebase Console |
| `room1/history/{epochSec}` | `temperature`, `humidity`, `airDelta`, `airAqi`, `lightLux`, `timestamp` | Campuran | nilai sensor, `timestamp` milidetik | Device |
| `room1/events/{id}` | `timestamp`, `type`, `severity`, `message` | Campuran | `type`: `temp_high` / `air_bad` / `night_presence`; `severity`: `danger` / `warning` / `info` | Device |

### 7.2 Aturan Penulisan

| Path | Pengunjung | Staff | Device |
|---|---|---|---|
| `users/{uid}` | Baca milik sendiri | Baca milik sendiri | Baca milik sendiri |
| `room1/current` | Baca | Baca | Baca dan tulis |
| `room1/actuator/fanMode`, `lampMode` | Baca | Baca dan tulis | Baca |
| `room1/alarm/mute` | Baca | Baca dan tulis | Baca dan tulis |
| `room1/alarm/active`, `buzzer`, `status`, `reason` | Baca | Baca | Baca dan tulis |
| `room1/thresholds` | Baca | Baca | Baca |
| `room1/history` | Baca | Baca | Baca dan tulis |
| `room1/events` | Baca | Baca | Baca dan tulis |

Tanpa login, seluruh akses ditolak. Aplikasi Android tidak menulis `room1/current`, `room1/alarm/{active, buzzer, status, reason}`, `room1/history`, maupun `room1/events`.

### 7.3 Kesegaran Data

1. Perangkat memperbarui `room1/current` tiap 3 detik dengan `timestamp` epoch milidetik.
2. Aplikasi menghitung `System.currentTimeMillis() - timestamp`. Jika selisihnya lebih dari 15.000 ms, atau `timestamp` bernilai 0, aplikasi menampilkan banner "Perangkat offline" dan mempertahankan data terakhir.
3. Banner disembunyikan setelah data baru masuk.

### 7.4 Ambang Default

| Konstanta | Nilai default | Dipakai oleh |
|---|---|---|
| `tempAlert` | 35,0 °C | FR-01 |
| `tempFanOn` | 31,0 °C | FR-02 |
| `tempFanHysteresis` | 1,0 °C | FR-02 |
| `airDeltaModerate` | 80 ppb | FR-03 |
| `airDeltaBad` | 160 ppb | FR-03 |
| `lightDarkLux` | 40,0 lux | FR-07, FR-08 |
| `lightExternalHysteresisLux` | 200,0 lux | FR-08 |
| `lampEmptyTimeoutSec` | 10 detik | FR-08 |
| `nightStartHour`, `nightEndHour` | 22, 6 | FR-05 |

Seluruh nilai di atas dapat diubah lewat Firebase Console tanpa mengubah aplikasi maupun firmware.

---

## Lampiran A. Matriks Keterlacakan

Kolom Sumber menunjuk temuan pada Bab 5 dan sementara ditandai [OBS-xx] sampai observasi selesai.

| ID | Nama | Aktor | Path data utama | Sumber | PIC |
|---|---|---|---|---|---|
| FR-01 | Suhu dan peringatan suhu tinggi | Pengunjung, Staff | `room1/current/temperature`, `room1/alarm`, `room1/events` | [OBS-xx] | M1 Syakir |
| FR-02 | Kontrol kipas | Staff | `room1/actuator/fanMode`, `room1/current/fanStatus` | [OBS-xx] | M1 Syakir |
| FR-03 | Kualitas udara dan peringatan | Pengunjung, Staff | `room1/current/airDelta`, `airLabel`, `airWarmingUp` | [OBS-xx] | M2 Zakariya |
| FR-04 | Grafik 24 jam dan event log | Pengunjung, Staff | `room1/history`, `room1/events` | [OBS-xx] | M2 Zakariya |
| FR-05 | Kehadiran dan peringatan jam malam | Pengunjung, Staff | `room1/current/presence`, `nightModeActive` | [OBS-xx] | M3 Deswiryawan |
| FR-06 | Bunyi peringatan dan mute darurat | Staff | `room1/alarm` | [OBS-xx] | M3 Deswiryawan |
| FR-07 | Tingkat cahaya dan label | Pengunjung, Staff | `room1/current/lightLux`, `lightLabel` | [OBS-xx] | M4 Fattih |
| FR-08 | Kontrol lampu | Staff | `room1/actuator/lampMode`, `room1/current/lampStatus` | [OBS-xx] | M4 Fattih |
| FR-B01 | Autentikasi dan peran | Pengunjung, Staff | `users/{uid}/role` | [OBS-xx] | Bersama |
| FR-B02 | Dashboard dan status offline | Pengunjung, Staff | `room1/current/timestamp` | [OBS-xx] | Bersama |
| NFR-01 | Kinerja | - | - | [OBS-xx] | M1 Syakir |
| NFR-02 | Keandalan | - | - | [OBS-xx] | M1 Syakir |
| NFR-03 | Keamanan akses dan peran | - | `firebase/database.rules.json` | [OBS-xx] | M2 Zakariya |
| NFR-04 | Keamanan data dan kredensial | - | - | [OBS-xx] | M2 Zakariya |
| NFR-05 | Keterpeliharaan | - | Bab 7 | [OBS-xx] | M3 Deswiryawan |
| NFR-06 | Akurasi data | - | `room1/current/airWarmingUp` | [OBS-xx] | M3 Deswiryawan |
| NFR-07 | Usabilitas | - | - | [OBS-xx] | M4 Fattih |
| NFR-08 | Kompatibilitas | - | - | [OBS-xx] | M4 Fattih |

---

## Lampiran B. Keputusan Terbuka

1. **Mute tanpa batas waktu.** Saat ini alarm yang dibungkam tetap senyap selama kondisi bahaya belum mereda; tidak ada pengaktifan ulang otomatis setelah 60 detik seperti pada pemodelan UML. Perlu diputuskan apakah aturan 60 detik diberlakukan.
2. **Kehadiran sebagai syarat kipas Otomatis.** FR-02 hanya memakai suhu, sehingga kipas tetap menyala di ruangan kosong yang panas. Perlu diputuskan apakah kehadiran ikut menjadi syarat.
3. **Status offline per sensor.** FR-B02 memakai satu `timestamp` untuk seluruh data, bukan status terpisah per sensor. Perlu diputuskan apakah pemantauan kesegaran per sensor diperlukan.
