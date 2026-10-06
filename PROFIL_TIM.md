# Profil Tim - Smart Room Monitor

**Mata Kuliah:** Application Project
**Proyek:** Sistem Monitoring Ruangan Berbasis IoT (Smart Room Monitor)
**Hardware:** ESP32 + DHT11 + PIR + MQ-135
**Aplikasi:** Mobile/Web + Firebase Realtime Database

## 1. Anggota Tim

| No | Nama | NIM | Peran Scrum | Tanggung Jawab |
|----|------|-----|-------------|----------------|
| 1 | Fattih Kuwaka Bekti | 241111005 | Developer (Hardware & Firmware) | Wiring sensor, programming ESP32, integrasi Firebase, testing buzzer/LED |
| 2 | Akhmad Zakariya | 241111010 | Scrum Master | Mengatur sprint, daily standup, mengatasi hambatan tim, memastikan timeline 6 minggu |
| 3 | Deswiryawan Saragih | 241111008 | Product Owner | Menentukan kebutuhan fitur, prioritas backlog, komunikasi dengan dosen, validasi hasil |
| 4 | Ahmad Syakir Lathifuddin | 241111026 | Developer (App & Backend) | Struktur database Firebase, UI dashboard, grafik history, notifikasi alert |

> Catatan: Peran dapat ditukar sesuai kesepakatan tim. Product Owner dan Scrum Master tetap membantu development saat sprint berjalan.

## 2. Visi Proyek

**Visi:**
> "Mewujudkan ruangan kelas dan asrama yang aman, nyaman, dan terpantau secara real-time melalui sistem IoT yang murah, stabil, dan mudah digunakan."

**Masalah yang diselesaikan:**
1. Ruangan panas/pengap tidak terpantau saat tidak ada orang.
2. Kualitas udara buruk / asap tidak terdeteksi dini.
3. Tidak ada catatan kejadian (motion, suhu tinggi) yang bisa dilihat jarak jauh.

**Solusi:**
Sistem Smart Room Monitor membaca suhu, kelembaban, kualitas udara, dan gerakan manusia setiap 5 detik menggunakan ESP32, mengirim ke Firebase, dan menampilkannya di aplikasi mobile/web berupa dashboard live, grafik 24 jam, log kejadian, dan notifikasi otomatis + buzzer lokal.

**Target MVP (4-8 minggu):**
- [ ] Suhu & kelembaban live di aplikasi
- [ ] Status hunian (Occupied/Empty) dari PIR
- [ ] Skor kualitas udara + status Good/Poor/Dangerous
- [ ] Alert otomatis + tombol mute buzzer dari aplikasi
- [ ] Demo stabil menggunakan hotspot HP

**Definisi Selesai (Definition of Done):**
Aplikasi menampilkan 3 data sensor secara live, menyimpan history, dan memicu alert dengan benar saat demo.
