# UI Spec — SmartRoomMonitor (Refined)

Sumber: 19 layar Figma di `tampilan UI-20261124T120108Z-1-001/`, SRS v1.0, `firebase/schema-v0.9.json`, `firebase/database.rules.json`, firmware produksi. Folder asli tidak diubah.

## 1. Prinsip

- Mobile-first. Satu sumber HTML/CSS dirender pada 390×844 @2x; range verifikasi 360/390/412/768.
- Minimalism & Swiss: grid, hirarki tipografi tegas, tanpa shadow kecuali sheet/dialog, tanpa gradien, tanpa emoji sebagai ikon (SVG inline).
- Teks UI seluruhnya bahasa Indonesia.
- Status selalu berupa label, bukan hanya warna atau angka.
- Satu path satu penulis: aplikasi hanya menulis `room1/actuator/fanMode`, `room1/actuator/lampMode`, `room1/alarm/mute`.

## 2. Token

### Light

| Token | Nilai | Pakai |
|---|---|---|
| `--bg` | `#F4F2EE` | latar halaman |
| `--surface` | `#FBF9F6` | kartu, sheet, bottom nav |
| `--surface-2` | `#EDEAE3` | chip, segmented track |
| `--text` | `#1A1815` | judul, nilai |
| `--muted` | `#5C574F` | label, caption |
| `--faint` | `#6B6459` | metadata terkecil (4.5:1+) |
| `--border` | `#D9D4CA` | garis, divider |
| `--border-soft` | `#E7E3DB` | kartu |
| `--danger` | `#96281B` | teks bahaya, tombol MUTE |
| `--danger-bg` | `#F7E7E4` | banner bahaya |
| `--danger-border` | `#E7C4BE` | border banner |
| `--success` | `#3F6F58` | status aman, tersambung |
| `--warning` | `#8A5A16` | status Sedang |
| `--on-accent` | `#FFFFFF` | teks di atas danger/success |

### Dark

| Token | Nilai |
|---|---|
| `--bg` | `#171512` |
| `--surface` | `#201D19` |
| `--surface-2` | `#2B2721` |
| `--text` | `#F5F1EA` |
| `--muted` | `#C9C2B6` |
| `--faint` | `#B4AC9E` |
| `--border` | `#453F36` |
| `--border-soft` | `#332F28` |
| `--danger` | `#F49A8C` |
| `--danger-bg` | `#3A1D18` |
| `--danger-border` | `#6B3029` |
| `--success` | `#7FC3A2` |
| `--warning` | `#E0B368` |
| `--on-accent` | `#171512` |

Kontras di light: `#1A1815` pada `#FBF9F6` = 16,86; `#5C574F` = 6,82; `#6B6459` ≈ 5,3; `#96281B` = 7,64; `#FFFFFF` pada `#1A1815` = 17,72.

### Type

Plus Jakarta Sans (fallback system-ui). Skala: 28/700 judul, 22/700 nilai besar, 17/600 judul kartu, 15/500 body, 13/500 caption, 11/600 micro-label. Line-height 1,35 untuk judul, 1,5 body. Tabular numerals untuk angka sensor.

### Spacing & bentuk

4-based: 4, 8, 12, 16, 20, 24, 32. Padding halaman 20. Radius 20 (kartu), 14 (chip/button), 999 (pill). Border 1px. Tap target minimum 48×48, jarak antar target 8. Bottom nav tinggi 64 + 20 safe-area inset.

### Motion

150–200 ms ease-out untuk state, 300 ms untuk skeleton shimmer. `prefers-reduced-motion: reduce` mematikan transisi dan shimmer, menampilkan state akhir.

## 3. Komponen

| Komponen | Aturan |
|---|---|
| Top bar | Judul 22/700, subjudul 13, tidak collapse saat scroll |
| Status line | Dot + "Tersambung · Diperbarui HH:MM:SS" atau "Perangkat offline" |
| Offline banner | Surface danger-bg, teks "Perangkat offline", keterangan data terakhir tetap tampil, tanpa tombol dismiss |
| Alarm banner | Ikon SVG + judul + alasan + timestamp; tombol MUTE hanya untuk Staff, ≥48dp, penuh lebar |
| Metric card | Label micro, nilai 22, satuan, label status dengan dot, baris batas |
| Segmented control | 3 opsi (Otomatis / Selalu ON / Selalu OFF), tinggi 48, selected surface + border |
| Mode card | Judul, status fisik (Nyala/Mati), segmented, catatan aturan auto |
| Event row | Dot severity + judul + jam + pesan; warna BUKAN satu-satunya penanda (selalu ada label tipe) |
| Chart card | SVG line, grid tipis, axis jam, garis ambang dashed dengan label, ringkasan min/rata-rata/maks. Default hanya suhu; chip Kelembapan dan Udara muncul setelah tombol "Metrik lain" ditekan |
| Empty / Error / Skeleton | Ikon SVG, judul, 1–2 kalimat, tanpa CTA palsu |
| Bottom nav | 4 tab: Dashboard, Grafik, Kejadian, Kontrol, Profil (5 maksimal), ikon + label |
| Snackbar | Konfirmasi aksi Staff, 3 detik, `role="status"` |
| Dialog | Konfirmasi keluar, dua tombol, fokus ke Batal |

## 4. Matriks layar

| # | Layar | Sumber | Perubahan utama |
|---|---|---|---|
| 01 | Masuk | asli | Plus Jakarta Sans, label field, show/hide sandi |
| 02 | Masuk · Kredensial salah | asli | Error summary + inline error + role=alert, tanpa syarat panjang sandi |
| 03 | Masuk · Memuat sesi | asli | Skeleton, tanpa status palsu |
| 04 | Dashboard · Staff · Normal | asli | Tambah satuan ppb, label status, batas tiap kartu |
| 05 | Dashboard · Pengunjung | asli | Footnote konsisten, tanpa kontrol |
| 06 | Dashboard · Offline | asli | Banner + data terakhir, status jujur |
| 07 | Dashboard · Alarm aktif | asli | Banner + MUTE 1 tap |
| 08 | Dashboard · Alarm dibungkam | asli | Hapus klaim re-arm 60 detik |
| 09 | Dashboard · Memuat | asli | Header skeleton, tanpa "Tersambung" palsu |
| 10 | Grafik · Ada data | asli | Default hanya suhu; tombol "Metrik lain" untuk expand |
| 11 | Grafik · Belum ada data | asli | Copy: history tiap 10 menit, 144 titik/24 jam |
| 12 | Kejadian · Daftar | asli | Hanya 3 tipe event, severity sebagai teks |
| 13 | Kejadian · Filter per tipe | asli | Chip filter + hasil |
| 14 | Kejadian · Kosong | asli | Copy diselaraskan |
| 15 | Kontrol · Staff · Normal | asli | Kosong 10 detik, status fisik jelas |
| 16 | Kontrol · Pengunjung · Ditolak | asli | Alasan akses + tombol kembali Dashboard |
| 17 | Kontrol · Offline | asli | Kontrol disabled + alasan |
| 18 | Profil · Staff | asli | Typo dibetulkan |
| 19 | Profil · Pengunjung | asli | Konsisten "Tampilan" |
| 20 | Dashboard · Alarm · Pengunjung | baru | Banner tanpa MUTE |
| 21 | Dashboard · Alarm · Offline | baru | Alarm aktif + offline, MUTE tetap tampil untuk Staff |
| 22 | Kontrol · Berhasil | baru | Snackbar "Mode kipas diperbarui" |
| 23 | Konfirmasi keluar | baru | Dialog dua tombol |
| 24 | Grafik · Data parsial | baru | 6 titik, axis tetap 24 jam |
| 25 | Dashboard · Gelap | baru | Token dark |
| 26 | Kontrol · Gelap | baru | Token dark |
| 27 | Grafik · Gelap | baru | Token dark |
| 28 | Profil · Gelap | baru | Token dark |

## 5. Copy final (pengganti)

| Lokasi | Lama | Baru |
|---|---|---|
| 02 | "Kata sandi minimal 6 karakter." | "Email atau kata sandi salah. Periksa kembali lalu coba lagi." |
| 08 | "Alarm tetap dibungkam sampai seluruh kondisi bahaya mereda." | "Alarm aktif lagi dalam 60 detik bila kondisi bahaya belum mereda." |
| 11 | "Perangkat mengirim tiap 5 detik, grafik terisi dalam satu menit." | "Perangkat menyimpan satu titik tiap 10 menit. Grafik menampilkan hingga 144 titik untuk 24 jam." |
| 15/16/17 | "kosong selama beberapa menit" | "kosong selama 10 detik" |
| 12 | "Ruangan kosong selama 6 menit, lampu dimatikan otomatis" | dihapus; ganti event `temp_high` / `air_bad` / `night_presence` |
| 18 | "halu mati", "bertaku", "menyuaikan", "sekali gus" | "lalu mati", "berlaku", "menyesuaikan", "sekaligus" |
| 19 | "Tampilkan" | "Tampilan" |
| 04 | "Delta 18" | "Delta 18 ppb" |
| 09 | "Tersambung · Diperbarui 12:04:31" | skeleton tanpa status |

## 6. Verifikasi

- [x] Kontras teks ≥4.5:1 pada light & dark (light 5,23–16,86; dark 7,19–14,91)
- [x] Semua target ≥48×48, jarak ≥8 (token `.btn`/`.seg`/`.nav-item`)
- [x] Tidak ada horizontal overflow: `width: min(390px, 100vw)`, `min-width: 0`, `flex-wrap`
- [x] Dark mode 4 layar utama (25–28, terverifikasi piksel `#171512`)
- [x] `prefers-reduced-motion` mematikan transisi & shimmer
- [x] 28 PNG @780×1688, tidak ada berkas kosong
- [x] Tidak ada emoji sebagai ikon struktural (SVG inline)
- [ ] Fokus error login pindah ke ringkasan: butuh perilaku JS saat implementasi Compose, prototipe statis baru menyediakan `role="alert" tabindex="-1"`
- [ ] Uji sentuh nyata di 2 perangkat fisik (NFR-08)
