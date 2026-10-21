# SOFTWARE REQUIREMENTS SPECIFICATION
## SmartRoomMonitor
### Aplikasi Android untuk Memantau dan Mengatur Kondisi Ruangan

| Informasi | Keterangan |
|---|---|
| **Mata kuliah** | Rekayasa Perangkat Lunak (RPL), M4/M5 Analisis dan Dokumentasi Kebutuhan |
| **Versi / tanggal** | SRS v0.8, 14 Oktober 2026 |
| **Tim** | Deswiryawan Saragih (241111008), Akhmad Zakariya (241111010), Fattih Kuwaka Bekti (241111005), Ahmad Syakir L. (241111026) |

---

### Riwayat Versi

| Versi | Tanggal | Perubahan | Disetujui |
|---|---|---|---|
| **v0.1 - v0.6** | 7 - 8 Okt 2026 | Baseline kebutuhan, pembagian FR/NFR, latar belakang, profil pengguna, elisitasi, dan batasan. | Deswiryawan |
| **v0.7.1 - v0.7.3** | 14 Okt 2026 | Penyederhanaan lingkup, penambahan bab interaksi pengguna, kebutuhan ditulis dari sudut pandang pengguna. | Digantikan |
| **v0.8** | 14 Okt 2026 | Fokus pada enam bagian: latar belakang, profil pengguna, kebutuhan fungsional dan non-fungsional, hasil elisitasi (template kosong), dan batasan sistem. Peran diubah menjadi Pengunjung dan Staff. Kebutuhan dirapikan menjadi 8 FR dan 8 NFR, dua untuk tiap anggota. | Menunggu persetujuan |

---

## 1. Latar Belakang

### 1.1 Latar Belakang
Kondisi ruangan seperti suhu, kelembapan, kualitas udara, cahaya, dan keberadaan orang umumnya dipantau secara manual: dicek langsung di tempat, atau baru diketahui setelah ada keluhan. Cara ini tidak berjalan terus-menerus, tidak meninggalkan catatan, dan membuat penanganan terlambat ketika suhu terlalu tinggi, udara memburuk, ada orang di luar jam semestinya, atau lampu dan kipas menyala tanpa perlu.

SmartRoomMonitor adalah aplikasi Android yang membantu pengguna melihat kondisi ruangan dari mana saja, menerima peringatan saat ada yang tidak normal, dan mengatur kipas serta lampu. Data kondisi ruangan dikirim oleh sebuah perangkat sensor di ruangan melalui layanan Firebase. Perangkat sensor hanya menjadi sumber data dan pelaksana perintah, sementara seluruh interaksi pengguna berjalan di aplikasi.

Aplikasi dipakai oleh dua peran. Pengunjung hanya melihat kondisi ruangan, grafik, dan daftar kejadian. Staff selain melihat juga mengatur kipas dan lampu, membungkam alarm, serta menyesuaikan batas peringatan.

### 1.2 Tujuan
Memberi pengguna satu aplikasi yang mudah dipakai untuk:
1. Melihat kondisi ruangan saat ini,
2. Mendapat peringatan dan membungkamnya dengan cepat,
3. Mengatur kipas dan lampu, serta
4. Melihat tren dan riwayat kejadian.

---

## 2. Profil Pengguna dan Aktor

Aplikasi dipakai oleh dua peran. Pengunjung hanya membaca data, sedangkan Staff dapat mengendalikan perangkat dan mengubah batas.

| Aspek | Pengunjung | Staff |
|---|---|---|
| **Siapa** | Orang yang ingin tahu kondisi ruangan tanpa ikut mengatur, misalnya tamu, mahasiswa, atau penghuni. | Orang yang bertanggung jawab menjaga ruangan, misalnya operator ruang atau pengelola. |
| **Kemampuan teknis** | Rendah; tidak perlu paham IoT atau Firebase. | Menengah; paham arti batas (*threshold*) dan cara mengubahnya. |
| **Perangkat** | Smartphone Android (minSdk 26). | Smartphone Android (minSdk 26) dan Firebase Console. |
| **Tujuan utama** | Tahu kondisi ruangan saat ini dan riwayat kejadian. | Menjaga ruangan tetap nyaman dan aman, serta menyesuaikan batas peringatan. |
| **Yang dapat dilakukan** | Login, melihat dashboard, grafik, dan event log. | Semua milik Pengunjung, ditambah kontrol kipas dan lampu, mute alarm, serta mengubah batas lewat Firebase Console. |
| **Batas akses** | Hanya membaca data; tidak dapat mengubah apa pun. | Dapat mengubah mode kipas dan lampu, membungkam alarm, dan mengubah batas. |

**Aktor pendukung:**
- **Perangkat sensor:** Sumber data kondisi ruangan dan pelaksana perintah kipas, lampu, buzzer.
- **Firebase:** Penyimpanan data (*Realtime Database*) dan autentikasi (*Firebase Auth*).

---

## 3. Kebutuhan Fungsional

Bab ini memuat 8 kebutuhan fungsional (FR). Setiap anggota memegang dua FR. Kolom Aktor menunjuk Bab 2, kolom PIC menunjuk modul pada pembagian tugas (M1 Syakir, M2 Zakariya, M3 Fattih, M4 Deswiryawan). Kolom Sumber diisi dari Bab 5 setelah observasi selesai, dan sementara ditandai `[OBS-xx]`.

| ID | Nama | Aktor | Deskripsi kebutuhan | Sumber | PIC |
|---|---|---|---|---|---|
| **FR-01** | Suhu dan peringatan suhu tinggi | Pengunjung, Staff | Pengguna melihat suhu ruangan. Jika suhu melebihi 35 °C, pengguna mendapat banner/badge, perangkat berbunyi, dan kejadian dicatat. Peringatan tidak berulang dalam 60 detik. | `[OBS-xx]` | M1 Syakir |
| **FR-02** | Kontrol kipas | Staff | Staff memilih mode kipas Otomatis / Selalu ON / Selalu OFF. Pada Otomatis, kipas menyala saat suhu di atas 31 °C dan mati setelah turun 1 °C. Status kipas tampil di dashboard. | `[OBS-xx]` | M1 Syakir |
| **FR-03** | Kelembapan dan label | Pengunjung, Staff | Pengguna melihat kelembapan beserta label Kering, Nyaman, atau Lembap. | `[OBS-xx]` | M2 Zakariya |
| **FR-04** | Grafik 24 jam dan event log | Pengunjung, Staff | Pengguna melihat grafik 24 jam untuk suhu, kelembapan, dan kualitas udara beserta nilai terendah, tertinggi, dan rata-rata, serta daftar kejadian (kehadiran, suhu tinggi, udara buruk) dengan waktu dan warna per tipe, dapat difilter 24 jam dan per tipe. | `[OBS-xx]` | M2 Zakariya |
| **FR-05** | Kualitas udara dan peringatan | Pengunjung, Staff | Pengguna melihat label udara: Memanaskan, Baik, Sedang, atau Buruk (bukan angka mentah). Saat Buruk, pengguna mendapat banner/badge dan bunyi, serta kejadian dicatat. Tidak ada peringatan selama Memanaskan (3 menit pertama). | `[OBS-xx]` | M3 Fattih |
| **FR-06** | Bunyi peringatan dan mute darurat | Staff | Perangkat berbunyi saat ada peringatan dan tetap berbunyi walau koneksi internet terputus. Saat alarm berbunyi, muncul banner dan tombol MUTE besar; staff dapat membungkam alarm maksimal 2 tap, dan bunyi berhenti dalam <= 7 detik. | `[OBS-xx]` | M3 Fattih |
| **FR-07** | Kehadiran dan peringatan jam malam | Pengunjung, Staff | Pengguna melihat status kehadiran (tidak ada / bergerak / diam). Jika ada orang pada pukul 22:00-06:00, pengguna mendapat banner/badge dan bunyi, serta kejadian dicatat. | `[OBS-xx]` | M4 Deswiryawan |
| **FR-08** | Kontrol lampu | Staff | Staff melihat tingkat cahaya dan memilih mode lampu Otomatis / Selalu ON / Selalu OFF. Pada Otomatis, lampu menyala saat gelap dan ada orang, dan mati saat terang atau ruangan kosong beberapa menit. | `[OBS-xx]` | M4 Deswiryawan |

### 3.1 Kebutuhan Bersama
Dua kebutuhan berikut dikerjakan bersama dan tidak dihitung sebagai bagian dari 8 FR di atas.

| ID | Nama | Aktor | Deskripsi kebutuhan | Sumber | PIC |
|---|---|---|---|---|---|
| **FR-B01** | Autentikasi dan peran | Pengunjung, Staff | Pengguna dapat login dan logout dengan email + password; sesi tetap tersimpan. Akses tanpa login ditolak. Peran Pengunjung dan Staff dibedakan. | `[OBS-xx]` | Bersama |
| **FR-B02** | Dashboard dan status offline | Pengunjung, Staff | Pengguna melihat kondisi ruangan pada satu layar, diperbarui tiap 5 detik. Jika data berhenti masuk lebih dari 15 detik, muncul banner "Perangkat offline" dan data terakhir tetap tampil. | `[OBS-xx]` | Bersama |

---

## 4. Kebutuhan Non-Fungsional

Bab ini memuat 8 kebutuhan non-fungsional (NFR). Setiap anggota memegang dua NFR, berpasangan dengan dua FR miliknya.

| ID | Kategori | Deskripsi kebutuhan | Verifikasi | PIC |
|---|---|---|---|---|
| **NFR-01** | Kinerja | Perubahan kondisi ruangan tampil di aplikasi dalam <= 7 detik; cold start < 3 detik. | 10 sampel via hotspot; lulus bila 9/10 <= 7 detik. | M1 Syakir |
| **NFR-02** | Keandalan | Sistem tersambung kembali otomatis setelah koneksi putus; banner offline muncul saat data berhenti, dan data terakhir tetap tampil. | 15 menit tanpa putus; banner muncul <= 20 detik dan hilang <= 10 detik setelah data masuk. | M1 Syakir |
| **NFR-03** | Keamanan akses dan peran | Data hanya dapat dibaca pengguna yang login; Pengunjung tidak dapat mengubah data maupun batas; hanya Staff yang dapat mengubah mode kipas/lampu, mute, dan batas. | File rules + screenshot akses ditolak. | M2 Zakariya |
| **NFR-04** | Keamanan data dan kredensial | Kunci rahasia tidak masuk repositori; akses data diatur lewat aturan (*rules*) Firebase. | Pemeriksaan repositori tanpa kunci rahasia + file rules. | M2 Zakariya |
| **NFR-05** | Usabilitas | Satu layar utama; mute <= 2 tap; bahasa Indonesia; status berupa label yang mudah dipahami; tombol >= 48 dp. | Checklist 5 item + uji 2 pengguna non-teknis; screenshot. | M3 Fattih |
| **NFR-06** | Kompatibilitas | Android minSdk 26+, Kotlin + Compose, Firebase. | assembleDebug lolos di 2 HP. | M3 Fattih |
| **NFR-07** | Keterpeliharaan | Struktur modul dipisah per fitur; nama field data disepakati sebelum coding agar tidak bentrok. | Tinjauan struktur modul dan daftar nama field. | M4 Deswiryawan |
| **NFR-08** | Akurasi data | Penilaian udara berbasis kenaikan dari kondisi normal, bukan angka absolut; tidak ada peringatan dari pembacaan sensor yang belum stabil (masa pemanasan). | Uji masa pemanasan 3 menit tanpa peringatan. | M4 Deswiryawan |

---

## 5. Hasil Elisitasi

Elisitasi belum dilakukan secara langsung. Tabel di bawah disediakan sebagai template yang diisi setelah observasi dan pengujian selesai. Kolom Sumber pada Bab 3 dan 4 baru dapat diisi setelah bagian ini lengkap.

### 5.1 Rencana Observasi
| Item | Isi |
|---|---|
| **Teknik** | *(isi: observasi langsung / wawancara / uji coba skenario)* |
| **Objek amati** | *(isi)* |
| **Tanggal** | *(isi)* |
| **Lokasi** | *(isi)* |
| **Pengamat** | *(isi)* |
| **Alat bantu** | *(isi: catatan, foto, rekaman)* |

### 5.2 Temuan
| ID | Aspek | Temuan | Implikasi kebutuhan |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

### 5.3 Diskusi Kebutuhan
*Usulan dari diskusi tim, diposisikan sebagai client. Statusnya usulan dan belum divalidasi ke pengguna nyata.*
| ID | Aspek | Kebutuhan yang diusulkan | Implikasi kebutuhan |
|---|---|---|---|
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |
|  |  |  |  |

*Catatan: isi kolom Sumber pada Bab 3 dan 4 dengan ID temuan dari 5.2 dan 5.3 setelah observasi selesai. Sebelum terisi, kolom Sumber ditandai `[OBS-xx]`.*

---

## 6. Batasan Sistem

Batasan berikut membatasi ruang lingkup software dan kebutuhan pada Bab 3 dan 4. Perangkat keras hanya disebut sebagai sumber data dan pelaksana perintah; detailnya dibahas di dokumen terpisah.

| Kategori | Batasan |
|---|---|
| **Lingkup** | Satu ruangan (`room1`). Multi-room, web dashboard, OTA, export PDF, kamera, humidifier/dehumidifier, dan push notification di luar MVP. Kamera tidak dipakai karena beban pemrosesan dan pertimbangan privasi. |
| **Hak akses** | Pengunjung hanya membaca; Staff dapat mengubah mode kipas/lampu, mute, dan batas. Ubah batas dilakukan lewat Firebase Console, di luar aplikasi. |
| **Perangkat sensor** | Satu perangkat sensor sebagai sumber data (suhu, kelembapan, udara, cahaya, kehadiran) dan pelaksana perintah kipas, lampu, dan buzzer. Detail rangkaian di dokumen terpisah. Nilai udara bersifat perkiraan, bukan alat ukur resmi. |
| **Beban kipas dan lampu** | Diasumsikan berskala kecil (mis. LED atau kipas DC lewat relay), tidak langsung ke listrik AC. |
| **Privasi** | Sistem hanya menyimpan data numerik dan status; tidak merekam gambar maupun suara. |
| **Jaringan** | Bergantung pada hotspot HP dan internet. Tanpa koneksi, data tidak masuk ke aplikasi; perangkat tetap menjalankan otomatisasi dan bunyi peringatan secara lokal. |
| **Pembaruan data** | Data diperbarui tiap 5 detik; perintah dari aplikasi dapat tertunda hingga 7 detik. |
| **Platform** | Hanya Android minSdk 26+; antarmuka berbahasa Indonesia; login hanya email + password. |
| **Peringatan** | Muncul sebagai banner/badge saat aplikasi terbuka, plus bunyi di perangkat. Semua batas masih tentatif. Sistem bukan alarm kebakaran/gas resmi. |
| **Backend** | Menggunakan Firebase (*Realtime Database* dan *Auth*) sesuai batas paket yang dipakai. |
| **Proyek** | Tugas mata kuliah RPL M4/M5, dikerjakan tim 4 orang dengan tenggat dari mata kuliah. |

---

## Lampiran A. Matriks Keterlacakan

Kolom Sumber menunjuk temuan pada Bab 5 dan sementara ditandai `[OBS-xx]` sampai observasi selesai.

| ID | Nama | Aktor | Sumber | PIC |
|---|---|---|---|---|
| **FR-01** | Suhu dan peringatan suhu tinggi | Pengunjung, Staff | `[OBS-xx]` | M1 Syakir |
| **FR-02** | Kontrol kipas | Staff | `[OBS-xx]` | M1 Syakir |
| **FR-03** | Kelembapan dan label | Pengunjung, Staff | `[OBS-xx]` | M2 Zakariya |
| **FR-04** | Grafik 24 jam dan event log | Pengunjung, Staff | `[OBS-xx]` | M2 Zakariya |
| **FR-05** | Kualitas udara dan peringatan | Pengunjung, Staff | `[OBS-xx]` | M3 Fattih |
| **FR-06** | Bunyi peringatan dan mute darurat | Staff | `[OBS-xx]` | M3 Fattih |
| **FR-07** | Kehadiran dan peringatan jam malam | Pengunjung, Staff | `[OBS-xx]` | M4 Deswiryawan |
| **FR-08** | Kontrol lampu | Staff | `[OBS-xx]` | M4 Deswiryawan |
| **FR-B01** | Autentikasi dan peran | Pengunjung, Staff | `[OBS-xx]` | Bersama |
| **FR-B02** | Dashboard dan status offline | Pengunjung, Staff | `[OBS-xx]` | Bersama |
| **NFR-01** | Kinerja | - | `[OBS-xx]` | M1 Syakir |
| **NFR-02** | Keandalan | - | `[OBS-xx]` | M1 Syakir |
| **NFR-03** | Keamanan akses dan peran | - | `[OBS-xx]` | M2 Zakariya |
| **NFR-04** | Keamanan data dan kredensial | - | `[OBS-xx]` | M2 Zakariya |
| **NFR-05** | Usabilitas | - | `[OBS-xx]` | M3 Fattih |
| **NFR-06** | Kompatibilitas | - | `[OBS-xx]` | M3 Fattih |
| **NFR-07** | Keterpeliharaan | - | `[OBS-xx]` | M4 Deswiryawan |
| **NFR-08** | Akurasi data | - | `[OBS-xx]` | M4 Deswiryawan |
